#include "ioactionform.h"
#include "ui_ioactionform.h"
#include <QMessageBox>
#include <QDebug>
#include <QSerialPortInfo>
#include <QSettings>
#include <QTextCodec>
#include <QDateTime>
#include <QLibrary>
#include <QMetaObject>
#include <QThread>
#include <QtConcurrent/QtConcurrentRun>
#include "MZ_VC3000H.h"
#include <Common/SysEnv.h>
#ifdef Q_OS_WIN
#include <qt_windows.h>
#endif

namespace {
bool isRunningAsAdministrator()
{
#ifdef Q_OS_WIN
    QLibrary shell32(QStringLiteral("shell32"));
    using IsUserAnAdminFunction = BOOL (WINAPI*)();
    const auto isUserAnAdmin = reinterpret_cast<IsUserAnAdminFunction>(
        shell32.resolve("IsUserAnAdmin"));
    return isUserAnAdmin && isUserAnAdmin() != FALSE;
#else
    return true;
#endif
}
}

ioActionForm::ioActionForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ioActionForm)
{
    ui->setupUi(this);
    m_plcTaskPool.setMaxThreadCount(3);
    this->setWindowFlags(Qt::Window);
    this->setAttribute(Qt::WA_DeleteOnClose, false);
    this->setWindowFlags(Qt::Window | Qt::CustomizeWindowHint | Qt::WindowCloseButtonHint);
    this->setWindowTitle(tr("IO Channel (PLC)"));

    // 初始化DI状态
    for (int i = 0; i < 8; i++) {
        m_lastDIStates[i] = true;
    }

    // 读取配置
    readPLCConfig();

    // 初始化定时器
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &ioActionForm::onTimeout);

    m_diPollTimer = new QTimer(this);
    connect(m_diPollTimer, &QTimer::timeout, this, &ioActionForm::pollDIInputs);

    // 连接信号（先不连接COM端口变化）
    connect(ui->cBox_3, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ioActionForm::onDOIndexChanged);

    connect(ui->Bt_open, &QPushButton::clicked, this, &ioActionForm::on_Bt_open_clicked);
    connect(ui->Bt_refresh, &QPushButton::clicked, this, &ioActionForm::on_Bt_refresh_clicked);
    connect(ui->Bt_reset_sync, &QPushButton::clicked, this, &ioActionForm::on_Bt_reset_sync_clicked);
    connect(ui->checkBox_auto_poll, &QCheckBox::stateChanged,
            this, &ioActionForm::on_checkBox_auto_poll_stateChanged);

    // 初始化显示
    if (!m_do.isEmpty()) {
        ui->lineEdit->setText(QString::number(m_do[0]));
    }

    initDIDisplay();
    updateCameraStatusDisplay();
    updateSyncStatusDisplay();

    // 刷新COM端口（临时阻止信号）
    bool oldState = ui->cBox_1->blockSignals(true);
    populateCOMPorts();
    ui->cBox_1->blockSignals(oldState);

    // 现在连接COM端口变化信号
    connect(ui->cBox_1, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ioActionForm::onComPortChanged);

    // 关键：恢复自动初始化（和旧代码一样）
    if(initPLC()){
        ui->textEdit->append("PLC initialization successful!");
        m_isOpen = true;
        SysEnv::instance().setModuleOk(SysEnv::PLC);
        ui->Bt_open->setText("关闭");

        // 启动DI轮询
        //暂时关闭默认轮询
        m_diPollTimer->start(10);
    } else {
        SysEnv::instance().setModuleBad(SysEnv::PLC);
        ui->textEdit->append("PLC未连接，主程序已进入离线模式");
    }
}

ioActionForm::~ioActionForm()
{
    closePLC();
    delete ui;
}

void ioActionForm::readPLCConfig()
{
    QString Path = "configs/configs.ini";
    QSettings iniRead(Path, QSettings::IniFormat);
    iniRead.setIniCodec(QTextCodec::codecForName("UTF_8"));

    QString comPort = iniRead.value("PLC/comPort").toString();
    QString outputType = iniRead.value("PLC/outputType").toString();

    for(int i = 0; i < 8; i++){
        QString strDO = QString("DO%1").arg(i + 1);
        ui->cBox_3->addItem(strDO);
        QString outputDO = QString("PLC/DO%1").arg(i + 1);
        m_do.append(iniRead.value(outputDO, 1000).toInt());
    }

    ui->cBox_1->addItem(comPort);
    ui->cBox_2->addItem(outputType);

    m_savedComPort = comPort;
    m_outputType = outputType;
}

void ioActionForm::savePLCConfig()
{
    if (ui->cBox_3->currentIndex() < 0)
    {
        QMessageBox::warning(this, tr("Warning"), tr("Please select a DO index!"));
        ui->cBox_3->setFocus();
        return;
    }

    QString msText = ui->lineEdit->text().trimmed();
    if (msText.isEmpty())
    {
        QMessageBox::warning(this, tr("Warning"), tr("Delay time cannot be empty!"));
        ui->lineEdit->setFocus();
        ui->lineEdit->selectAll();
        return;
    }

    bool ok = false;
    int nMs = msText.toInt(&ok);
    if (!ok || nMs <= 0)
    {
        QMessageBox::warning(this, tr("Warning"), tr("Please enter a positive integer!"));
        ui->lineEdit->setFocus();
        ui->lineEdit->selectAll();
        return;
    }

    QString Path = "configs/configs.ini";
    QSettings iniWrite(Path, QSettings::IniFormat);
    iniWrite.setIniCodec(QTextCodec::codecForName("UTF-8"));

    int nDoIndex = ui->cBox_3->currentIndex();
    QString key = QString("PLC/DO%1").arg(nDoIndex + 1);
    iniWrite.setValue(key, nMs);
    m_do[nDoIndex] = nMs;
    iniWrite.sync();

    ui->textEdit->append(QString("DO%1脉冲宽度配置已保存: %2ms")
                             .arg(nDoIndex + 1).arg(nMs));
    QMessageBox::information(this, tr("Information"), tr("Saved successfully!"));
}

void ioActionForm::populateCOMPorts()
{
    // 保存当前选择
    QString currentPort = ui->cBox_1->currentText();

    // 临时阻止信号，防止触发onComPortChanged
    bool signalsBlocked = ui->cBox_1->blockSignals(true);

    // 清空并重新填充
    ui->cBox_1->clear();

    // 获取可用串口
    QList<QSerialPortInfo> ports = QSerialPortInfo::availablePorts();
    for (const QSerialPortInfo &port : ports) {
        ui->cBox_1->addItem(port.portName());
    }

    // 恢复信号状态
    ui->cBox_1->blockSignals(signalsBlocked);

    // 尝试恢复选择
    if (!currentPort.isEmpty()) {
        int index = ui->cBox_1->findText(currentPort);
        if (index >= 0) {
            ui->cBox_1->setCurrentIndex(index);
        } else if (ui->cBox_1->count() > 0) {
            ui->cBox_1->setCurrentIndex(0);
        }
    } else if (ui->cBox_1->count() > 0) {
        ui->cBox_1->setCurrentIndex(0);
    }
}

bool ioActionForm::initPLC()
{
    if (!isRunningAsAdministrator()) {
        qWarning() << "PLC initialization skipped: WinIO requires administrator privileges.";
        ui->textEdit->append("PLC未初始化：WinIO需要管理员权限；如需使用PLC，请以管理员身份运行程序");
        return false;
    }

    QMutexLocker locker(&m_plcMutex);

    // 初始化VC3000H系统
    if (PCI::pci().InitialSystem() != 0) {
        qDebug() << "Error: Failed to initialize VC3000H system!";
        ui->textEdit->append("Error: Failed to initialize VC3000H system!");
        return false;
    }

    // 打开串口
    QString portName = ui->cBox_1->currentText();
    if (PCI::pci().openseral(portName.toStdString().c_str()) != 0) {
        qDebug() << "Error: Failed to open serial port!";
        ui->textEdit->append("Error: Failed to open serial port!");
        ui->Bt_clear->setEnabled(false);
        ui->Bt_get->setEnabled(false);
        ui->Bt_start->setEnabled(false);
        return false;
    }

    // 设置PNP模式
    QString mode = ui->cBox_2->currentText();
    if (PCI::pci().setPNP(0, mode.toStdString().c_str()) != 0) {
        QString info = QString("Error: Failed to set %1 mode!").arg(mode);
        qDebug() << info;
        ui->textEdit->append(info);
        return false;
    }

    // DO1 与 DO2/3/4 走同一处理路径：初始化时不强制设定输出模式和电平，
    // 保留 PLC/demo 中已配置的空闲电平设置，避免覆盖用户在 demo 中调整的电平。

    for (int i = 0; i < 8; ++i) {
        m_diFilters[i].reset();
        m_lastDIStates[i] = true;
    }
    return true;
}

void ioActionForm::closePLC()
{
    m_isOpen = false;

    // 停止DI轮询定时器
    if (m_diPollTimer && m_diPollTimer->isActive()) {
        m_diPollTimer->stop();
    }

    // 停止输出定时器
    if (m_timer && m_timer->isActive()) {
        m_timer->stop();
    }

    m_plcTaskPool.waitForDone();
    m_diReadPending.store(false);

    QMutexLocker locker(&m_plcMutex);
    PCI::pci().Destroy();
}

void ioActionForm::initDIDisplay()
{
    // 设置DI标签初始颜色（默认高电平，绿色）
    QList<QLabel*> diLabels = {
        ui->label_44, ui->label_45, ui->label_46, ui->label_47,
        ui->label_48, ui->label_49, ui->label_50, ui->label_51
    };

    for (auto label : diLabels) {
        label->setStyleSheet("color: green;");
    }
}

void ioActionForm::updateDIDisplay(int port, bool state)
{
    // 更新UI显示输入状态
    QLabel* label = nullptr;
    switch (port) {
    case 1: label = ui->label_44; break;
    case 2: label = ui->label_45; break;
    case 3: label = ui->label_46; break;
    case 4: label = ui->label_47; break;
    case 5: label = ui->label_48; break;
    case 6: label = ui->label_49; break;
    case 7: label = ui->label_50; break;
    case 8: label = ui->label_51; break;
    default: return;
    }

    if (label) {
        label->setStyleSheet(state ? "color: green;" : "color: red;");
        label->setToolTip(state ? "高电平" : "低电平");
    }
}

// 推荐使用简化的pollDIInputs
void ioActionForm::pollDIInputs()
{
    if (!m_isOpen || !ui->checkBox_auto_poll->isChecked()) return;

    bool expected = false;
    if (!m_diReadPending.compare_exchange_strong(expected, true)) {
        return;
    }

    QtConcurrent::run(&m_plcTaskPool, [this]() {
        std::array<int, 8> rawStates{};
        {
            QMutexLocker locker(&m_plcMutex);
            for (int i = 0; i < 8; ++i) {
                rawStates[i] = PCI::pci().getinput(i + 1);
            }
        }

        QMetaObject::invokeMethod(this, [this, rawStates]() {
            m_diReadPending.store(false);
            if (!m_isOpen || !ui->checkBox_auto_poll->isChecked()) {
                return;
            }
            processDIInputs(rawStates);
        }, Qt::QueuedConnection);
    });
}

void ioActionForm::processDIInputs(const std::array<int, 8>& rawStates)
{
        for (int i = 0; i < 8; ++i) {
            if (rawStates[i] < 0) {
                qWarning() << QString("DI%1 read failed: %2").arg(i + 1).arg(rawStates[i]);
            }
        }

    for (int i = 1; i <= 8; i++) {
        const int rawState = rawStates[i - 1];
        if (rawState < 0) {
            qWarning() << QString("DI%1 read failed: %2").arg(i).arg(rawState);
            continue;
        }
        const bool rawHigh = rawState > 0;
        if (!m_diFilters[i - 1].filter(rawHigh)) {
            updateDIDisplay(i, m_diFilters[i - 1].currentState());
            continue;
        }

        const bool currentState = m_diFilters[i - 1].currentState();
        updateDIDisplay(i, currentState);

        if (m_lastDIStates[i-1] != currentState) {
            // 调试信息
            static int callCount = 0;
            callCount++;
            qDebug() << QString("[%1] DI%2 状态变化: %3 -> %4")
                            .arg(callCount)
                            .arg(i)
                            .arg(m_lastDIStates[i-1] ? "高" : "低")
                            .arg(currentState ? "高" : "低");

            // 处理变化
            handleDITrigger(i, currentState);

            // 更新状态
            m_lastDIStates[i-1] = currentState;
        }
    }
}

void ioActionForm::handleDITrigger(int diPort, bool state)
{
    // state为true=高电平，false=低电平
    // 根据需求，需要检测低电平（下降沿）

    qDebug() << QString("DI%1: %2").arg(diPort).arg(state ? "HIGH" : "LOW");

    // 检测下降沿：高->低
    bool fallingEdge = !state;  // 低电平直接表示下降沿触发

    switch (diPort) {
    case 1:  // DI1 - 相机1完成
        if (fallingEdge) {
            qDebug() << "Camera 1采集完成信号（低电平触发）";
            ui->textEdit->append("Camera 1采集完成信号");
            m_camera1Ready = true;
            updateCameraStatusDisplay();
            updateSyncStatusDisplay();
            emit camera1Ready();
        }
        break;

    case 2:  // DI2 - 相机2完成
        if (fallingEdge) {
            qDebug() << "Camera 2采集完成信号（低电平触发）";
            ui->textEdit->append("Camera 2采集完成信号");
            m_camera2Ready = true;
            emit camera2Ready();
            updateCameraStatusDisplay();
            updateSyncStatusDisplay();
        }
        break;

    case 3:  // DI3 - 清除数据
        if (fallingEdge) {
            qDebug() << "收到清除数据命令（低电平触发）";
            ui->textEdit->append("收到清除数据命令");
            clearDetectionOutputs();
            emit clearDataRequested();
            // 重置相机就绪标志
            m_camera1Ready = false;
            m_camera2Ready = false;
            updateCameraStatusDisplay();
            updateSyncStatusDisplay();
        }
        break;

    case 5:  // DI5-7 - 产品模式切换，按 SysEnv::productModes() 列表顺序对应 Z19/U07/E16
    case 6:
    case 7:
        if (fallingEdge) {
            const QString mode = SysEnv::productModes().value(diPort - 5);
            if (mode.isEmpty()) break;
            qDebug() << QString("收到产品模式切换命令（低电平触发）: %1").arg(mode);
            ui->textEdit->append(QString("DI%1触发：切换产品模式 -> %2").arg(diPort).arg(mode));
            emit productModeChangeRequested(mode);
        }
        break;

    default:
        // 其他DI端口可以在这里添加处理逻辑
        break;
    }
}

void ioActionForm::clearDetectionOutputs()
{
    const int resultOutputs[] = {1, 2, 3, 4, 8};
    bool success = true;
    QMutexLocker locker(&m_plcMutex);
    for (int point : resultOutputs) {
        const int result = PCI::pci().setoutput(point, false);
        if (result != 0) {
            success = false;
            qWarning() << QString("Clear DO%1 failed: %2").arg(point).arg(result);
        }
    }
    if (!success) {
        SysEnv::instance().setModuleBad(SysEnv::PLC);
    }
}

void ioActionForm::onComPortChanged(int index)
{
    // 关闭当前PLC连接
    closePLC();

    // 重新初始化PLC
    if(initPLC()){
        m_isOpen = true;
        SysEnv::instance().setModuleOk(SysEnv::PLC);
        ui->textEdit->append("PLC reinitialization successful!");
        if (ui->checkBox_auto_poll->isChecked()) {
            m_diPollTimer->start(50);
        }
    } else {
        m_isOpen = false;
        SysEnv::instance().setModuleBad(SysEnv::PLC);
    }
}

void ioActionForm::slotFrameInfo(const CFrameResult &data)
{
    if(!SysEnv::instance().isModuleOk(SysEnv::PLC) || !m_isOpen){
        return;
    }
    qDebug() << "[DO] cam=" << data.camera
             << "res=" << data.result
             << "plcOk=" << SysEnv::instance().isModuleOk(SysEnv::PLC)
             << "open=" << m_isOpen
             << "ts=" << QDateTime::currentMSecsSinceEpoch();

    const int camera = data.camera;
    const bool isNg = data.result;
    int resultPoint = 0;
    const bool resultLevel = true;
    QString resultText;

    if (camera == 1) {
        resultPoint = isNg ? 1 : 3;
        resultText = isNg ? QStringLiteral("N端 NG -> DO1 高电平")
                          : QStringLiteral("N端 OK -> DO3 脉冲");
    } else if (camera == 2) {
        resultPoint = isNg ? 2 : 4;
        resultText = isNg ? QStringLiteral("P端 NG -> DO2 脉冲")
                          : QStringLiteral("P端 OK -> DO4 脉冲");
    } else {
        qWarning() << "Ignoring result for invalid camera:" << data.camera;
        return;
    }

    QtConcurrent::run(&m_plcTaskPool, [this, camera, resultPoint, resultLevel, resultText]() {
        int resultRc = 0;
        int runRc = 0;
        {
            QMutexLocker locker(&m_plcMutex);
            resultRc = PCI::pci().setoutput(resultPoint, resultLevel);
            if (resultRc == 0) {
                runRc = PCI::pci().setoutput(8, true);
            }
        }

        QMetaObject::invokeMethod(this, [this, camera, resultPoint, resultText, resultRc, runRc]() {
            qDebug() << "[DO-OUT]" << "cam=" << camera << "chn=" << resultPoint
                     << "rc=" << resultRc << "ts=" << QDateTime::currentMSecsSinceEpoch();
            if (resultRc != 0) {
                SysEnv::instance().setModuleBad(SysEnv::PLC);
                return;
            }

            ui->textEdit->append(resultText);
            qDebug() << "[DO-OUT]" << "cam=" << camera << "chn=8" << "rc=" << runRc
                     << "ts=" << QDateTime::currentMSecsSinceEpoch();
            if (runRc == 0) {
                SysEnv::instance().setModuleOk(SysEnv::PLC);
                ui->textEdit->append(QStringLiteral("CCD运行信号 -> DO8 脉冲"));
            } else {
                SysEnv::instance().setModuleBad(SysEnv::PLC);
            }
        }, Qt::QueuedConnection);
    });
    return;

#if 0
    int result = 0;
    const QList<int> doChn = {1, 2, 3, 4, 5, 6, 7, 8};
    QMutexLocker locker(&m_plcMutex);

    switch (data.camera) {
    case 1:
        if(data.result){ // CCD1 NG
            result = PCI::pci().setoutput(doChn[0], false);  // DO1 active low
            qDebug() << "[DO-OUT]" << "cam=1" << "chn=1" << "rc=" << result
                     << "ts=" << QDateTime::currentMSecsSinceEpoch();
            ui->textEdit->append("N端 NG -> DO1 脉冲");
        } else {         // CCD1 OK
            result = PCI::pci().setoutput(doChn[2], true);  // DO3
            qDebug() << "[DO-OUT]" << "cam=1" << "chn=3" << "rc=" << result
                     << "ts=" << QDateTime::currentMSecsSinceEpoch();
            ui->textEdit->append("N端 OK -> DO3 脉冲");
        }
        break;

    case 2:
        if(data.result){ // CCD2 NG
            result = PCI::pci().setoutput(doChn[1], true);  // DO2
            qDebug() << "[DO-OUT]" << "cam=2" << "chn=2" << "rc=" << result
                     << "ts=" << QDateTime::currentMSecsSinceEpoch();
            ui->textEdit->append("P端 NG -> DO2 脉冲");
        } else {         // CCD2 OK
            result = PCI::pci().setoutput(doChn[3], true);  // DO4
            qDebug() << "[DO-OUT]" << "cam=2" << "chn=4" << "rc=" << result
                     << "ts=" << QDateTime::currentMSecsSinceEpoch();
            ui->textEdit->append("P端 OK -> DO4 脉冲");
        }
        break;

    default:
        qWarning() << "Ignoring result for invalid camera:" << data.camera;
        return;
    }
    if (result != 0) {
        SysEnv::instance().setModuleBad(SysEnv::PLC);
        return;
    }
    // 发送运行信号 - DO8
    result = PCI::pci().setoutput(doChn[7], true);
    qDebug() << "[DO-OUT]" << "cam=" << data.camera << "chn=8" << "rc=" << result
             << "ts=" << QDateTime::currentMSecsSinceEpoch();
    if (result == 0) {
        SysEnv::instance().setModuleOk(SysEnv::PLC);
        ui->textEdit->append("CCD运行信号 -> DO8 脉冲");
    } else {
        SysEnv::instance().setModuleBad(SysEnv::PLC);
    }
#endif
}

void ioActionForm::slotFixtureCodeReceived()
{
    const int point = 5;
    const int durationMs = m_do.value(4, 1000);
    queueScannerAckPulse(point, durationMs, QStringLiteral("fixture code ack DO5"));
    return;

#if 0
    int r1 = 0;
    {
        QMutexLocker locker(&m_plcMutex);
        r1 = PCI::pci().setoutput(point, true);
    }
    if (r1 != 0) {
        qDebug() << QString("[ERROR] fixture code ack DO5 ON failed:%1").arg(r1);
        return;
    }
    qDebug() << "[INFO] fixture code ack DO5 ON ok, point=" << point << ", will release in" << durationMs << "ms";

    QTimer::singleShot(durationMs, this, [this, point]() {
        int r2 = 0;
        {
            QMutexLocker locker(&m_plcMutex);
            r2 = PCI::pci().setoutput(point, false);
        }
        if (r2 != 0) {
            qDebug() << QString("[ERROR] fixture code ack DO5 OFF failed:%1").arg(r2);
        } else {
            qDebug() << "[INFO] fixture code ack DO5 OFF ok, point=" << point;
        }
    });
#endif
}

void ioActionForm::slotShellCodeReceived()
{
    const int point = 6;
    const int durationMs = m_do.value(5, 1000);
    queueScannerAckPulse(point, durationMs, QStringLiteral("shell code ack DO6"));
    return;

#if 0
    int r1 = 0;
    {
        QMutexLocker locker(&m_plcMutex);
        r1 = PCI::pci().setoutput(point, true);
    }
    if (r1 != 0) {
        qDebug() << QString("[ERROR] shell code ack DO6 ON failed:%1").arg(r1);
        return;
    }
    qDebug() << "[INFO] shell code ack DO6 ON ok, point=" << point << ", will release in" << durationMs << "ms";

    QTimer::singleShot(durationMs, this, [this, point]() {
        int r2 = 0;
        {
            QMutexLocker locker(&m_plcMutex);
            r2 = PCI::pci().setoutput(point, false);
        }
        if (r2 != 0) {
            qDebug() << QString("[ERROR] shell code ack DO6 OFF failed:%1").arg(r2);
        } else {
            qDebug() << "[INFO] shell code ack DO6 OFF ok, point=" << point;
        }
    });
#endif
}

void ioActionForm::queueScannerAckPulse(int point, int durationMs, const QString& codeName)
{
    if (!m_isOpen) {
        return;
    }

    QtConcurrent::run(&m_plcTaskPool, [this, point, durationMs, codeName]() {
        int r1 = 0;
        {
            QMutexLocker locker(&m_plcMutex);
            r1 = PCI::pci().setoutput(point, true);
        }

        QMetaObject::invokeMethod(this, [this, point, durationMs, codeName, r1]() {
            if (r1 != 0) {
                qDebug() << QString("[ERROR] %1 ON failed:%2").arg(codeName).arg(r1);
                return;
            }

            qDebug() << "[INFO]" << codeName << "ON ok, point=" << point
                     << ", will release in" << durationMs << "ms";

            QTimer::singleShot(durationMs, this, [this, point, codeName]() {
                QtConcurrent::run(&m_plcTaskPool, [this, point, codeName]() {
                    int r2 = 0;
                    {
                        QMutexLocker locker(&m_plcMutex);
                        r2 = PCI::pci().setoutput(point, false);
                    }

                    QMetaObject::invokeMethod(this, [point, codeName, r2]() {
                        if (r2 != 0) {
                            qDebug() << QString("[ERROR] %1 OFF failed:%2").arg(codeName).arg(r2);
                        } else {
                            qDebug() << "[INFO]" << codeName << "OFF ok, point=" << point;
                        }
                    }, Qt::QueuedConnection);
                });
            });
        }, Qt::QueuedConnection);
    });
}

void ioActionForm::onDOIndexChanged(int index)
{
    if (index >= 0 && index < m_do.size()) {
        ui->lineEdit->setText(QString::number(m_do[index]));
    }
}

void ioActionForm::onTimeout()
{
    if (!m_isOpen) return;

    QMutexLocker locker(&m_plcMutex);

    // 只发送勾选的输出
    const QList<int> doChn = {1, 2, 3, 4, 5, 6, 7, 8};
    for (int i = 0; i < 8; i++) {
        QCheckBox *checkBox = findChild<QCheckBox*>(QString("checkBox_%1").arg(i + 9));
        if (checkBox && checkBox->isChecked()) {
            int result = PCI::pci().setoutput(doChn[i], true);
            QString status = (result == 0) ? "成功" : QString("失败: %1").arg(result);
            ui->textEdit->append(QString("手动触发 DO%1: %2")
                                     .arg(i+1)
                                     .arg(status));
        }
    }
}

void ioActionForm::on_Bt_get_clicked()
{
    if (!m_isOpen) {
        ui->textEdit->append("PLC未连接!");
        return;
    }

    ui->textEdit->append(tr("获取IO电平..."));

    QMutexLocker locker(&m_plcMutex);

    for (int i = 1; i <= 8; i++) {
        int state = PCI::pci().getinput(i);
        if (state < 0) {
            ui->textEdit->append(QString(tr("DI%1 read failed: %2")).arg(i).arg(state));
            continue;
        }
        bool isHigh = state > 0;
        updateDIDisplay(i, isHigh);
        ui->textEdit->append(QString(tr("DI%1: %2")).arg(i).arg(isHigh ? tr("高电平") : tr("低电平")));
    }
}

void ioActionForm::on_Bt_start_clicked()
{
    if (!m_isOpen) {
        ui->textEdit->append("PLC未连接，无法启动!");
        return;
    }

    if (!m_isRunning) {
        // Start loop
        m_timer->start(1000); // Trigger every second
        m_isRunning = true;
        ui->Bt_start->setText(tr("停止执行"));
        ui->textEdit->append(tr("开始循环执行..."));
    } else {
        // Stop loop
        m_timer->stop();
        m_isRunning = false;
        ui->Bt_start->setText(tr("开始执行"));
        ui->textEdit->append(tr("停止执行"));
    }
}

void ioActionForm::on_Bt_clear_clicked()
{
    ui->textEdit->clear();
    ui->textEdit->append(tr("文本已清空"));
}

void ioActionForm::on_Bt_save_clicked()
{
    savePLCConfig();
    int nDoIndex = ui->cBox_3->currentIndex();
    if (nDoIndex >= 0 && nDoIndex < m_do.size()) {
        ui->lineEdit->setText(QString::number(m_do[nDoIndex]));
    }
}

void ioActionForm::on_Bt_reset_sync_clicked()
{
    m_camera1Ready = false;
    m_camera2Ready = false;
    m_syncReady = false;

    // 更新UI显示
    ui->label_camera1_status->setText("相机1: 等待");
    ui->frame_camera1->setStyleSheet("background-color: gray;");
    ui->label_camera2_status->setText("相机2: 等待");
    ui->frame_camera2->setStyleSheet("background-color: gray;");
    ui->label_sync_status->setText("同步状态: 等待");
    ui->frame_sync->setStyleSheet("background-color: gray;");

    ui->textEdit->append("已重置同步状态");
}


void ioActionForm::on_Bt_open_clicked()
{
    // 空实现！需要添加打开/关闭PLC的功能
    if (!m_isOpen) {
        if (initPLC()) {
            m_isOpen = true;
            SysEnv::instance().setModuleOk(SysEnv::PLC);
            ui->textEdit->append("PLC已打开");
            ui->Bt_open->setText("关闭");
            if (ui->checkBox_auto_poll->isChecked()) {
                m_diPollTimer->start(50);
            }
        } else {
            SysEnv::instance().setModuleBad(SysEnv::PLC);
        }
    } else {
        closePLC();
        m_isOpen = false;
        SysEnv::instance().setModuleBad(SysEnv::PLC);
        ui->textEdit->append("PLC已关闭");
        ui->Bt_open->setText("打开");
    }
}


void ioActionForm::on_Bt_refresh_clicked()
{
    populateCOMPorts();
    ui->textEdit->append("已刷新可用COM端口");
}


void ioActionForm::on_checkBox_auto_poll_stateChanged(int state)
{
    if (state == Qt::Checked) {
        if (m_isOpen) {
            m_diPollTimer->start(50);
            ui->textEdit->append("开启自动轮询");
        }
    } else {
        m_diPollTimer->stop();
        ui->textEdit->append("关闭自动轮询");
    }
}

void ioActionForm::updateCameraStatusDisplay()
{
    // 更新相机1状态
    if (m_camera1Ready) {
        ui->label_camera1_status->setText("相机1: 就绪");
        ui->frame_camera1->setStyleSheet("background-color: green;");
    } else {
        ui->label_camera1_status->setText("相机1: 等待");
        ui->frame_camera1->setStyleSheet("background-color: gray;");
    }

    // 更新相机2状态
    if (m_camera2Ready) {
        ui->label_camera2_status->setText("相机2: 就绪");
        ui->frame_camera2->setStyleSheet("background-color: green;");
    } else {
        ui->label_camera2_status->setText("相机2: 等待");
        ui->frame_camera2->setStyleSheet("background-color: gray;");
    }
}

void ioActionForm::updateSyncStatusDisplay()
{
    if (m_camera1Ready && m_camera2Ready) {
        ui->label_sync_status->setText("同步状态: 完成");
        ui->frame_sync->setStyleSheet("background-color: blue;");
        m_syncReady = true;
    } else {
        ui->label_sync_status->setText("同步状态: 等待");
        ui->frame_sync->setStyleSheet("background-color: gray;");
        m_syncReady = false;
    }
}
