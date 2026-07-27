#include "initwidget.h"
#include "ui_initwidget.h"
#include <CapSenseTri/capsensetriform.h>
#include <Form/algconfigform.h>
#include <Form/dataform.h>
#include <Form/ioactionform.h>
#include <Form/saveform.h>
#include <algorithmForm/algorithmtester.h>
#include <QMessageBox>
#include <QDateTime>
#include <QMovie>
#include <QTextStream>
#include <QTimer>
#include <QTranslator>
#include <QPushButton>
#include <QDebug>
#include <QRegularExpression>
#include <QMetaObject>
#include <QThread>
#include <Thread/imagenamer.h>
#include <Alg/weldingalgorithmmanager.h>
#include <Common/SysEnv.h>
#include <Common/common.h>
#include <Common/ScannerHook.h>

namespace {

bool isFixtureCodeFormatValid(const QString& code)
{
    static const QRegularExpression re(QStringLiteral("^[A-Z0-9-]{1,20}$"));
    return re.match(code).hasMatch();
}

bool isShellCodeFormatValid(const QString& code)
{
    static const QRegularExpression re(QStringLiteral("^[A-Z0-9@-]{21,}$"));
    return re.match(code).hasMatch();
}

}

InitWidget::InitWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::InitWidget)
{
    ui->setupUi(this);
    setWindowIcon(QIcon(":icon/app.ico"));
    setWindowFlags(Qt::FramelessWindowHint | Qt::Window | Qt::WindowMinimizeButtonHint);
    ui->btnMin->setFocusPolicy(Qt::NoFocus);
    ui->btnMax->setFocusPolicy(Qt::NoFocus);
    ui->btnClose->setFocusPolicy(Qt::NoFocus);
    connect(ui->btnMin,&QPushButton::clicked,this,&InitWidget::windowMin);
    connect(ui->btnMax,&QPushButton::clicked,this,&InitWidget::windowMax);
    connect(ui->btnClose,&QPushButton::clicked,this,&InitWidget::windowClose);

    connect(&SysEnv::instance(), &SysEnv::initDone, this,
            [this](quint8 m){
                updateModuleStatus(m);
            });
    qRegisterMetaType<WeldingResult>("WeldingResult");

    initWindow();
    initMenuBar();

    loadQssFile();

    ui->btnMax->hide();

    m_timer = new QTimer(this);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(updateTime()));
    m_timer->start(1000);

    // 添加清理定时器
    m_cleanupTimer = new QTimer(this);
    connect(m_cleanupTimer, &QTimer::timeout, this, &InitWidget::cleanupExpiredImages);

    windowMax();
    Alg::WeldingAlgorithmManager::instance().initialize();

    // 程序启动时执行一次清理，并安排每日清理
    cleanupExpiredImages();
    scheduleDailyCleanup();

    startScannerListening();
}

InitWidget::~InitWidget()
{
    Alg::WeldingAlgorithmManager::instance().setResultCallback({});
    Alg::WeldingAlgorithmManager::instance().stopWorkerThreads();

    // 停止所有定时器
    if (m_timer) {
        m_timer->stop();
    }
    if (m_cleanupTimer) {
        m_cleanupTimer->stop();
    }
    if (m_scannerHook) {
        disconnect(m_scannerHook, nullptr, this, nullptr);
        if (m_scannerThread && m_scannerThread->isRunning()) {
            QMetaObject::invokeMethod(m_scannerHook, "stopContinuous",
                                      Qt::BlockingQueuedConnection);
        }
    }
    if (m_scannerThread) {
        m_scannerThread->quit();
        m_scannerThread->wait();
        m_scannerHook = nullptr;
    }

    delete ui;
}

void InitWidget::initWindow()
{
    isMaxWin=true;
    ui->btnMax->setIcon(QIcon(":/img/max.png"));
    ui->btnMin->setIcon(QIcon(":/img/minimize.png"));
    ui->btnClose->setIcon(QIcon(":/img/close.png"));
    ui->textLogo->setPixmap(QPixmap(":/img/logo.png"));
    ui->textLogo->setScaledContents(true);
    qRegisterMetaType<QList<WeldingResult>>("QList<WeldingResult>");
    qRegisterMetaType<CFrameResult>("CFrameResult");

    m_triForm = new CapSenseTriForm(this);
    this->addWidget(m_triForm);

    m_ioForm = new ioActionForm();
    m_algorithmTester = new AlgorithmTester();
    m_saveForm = new SaveForm();
    // 添加清除数据连接
    connect(m_ioForm, &ioActionForm::clearDataRequested, m_triForm, &CapSenseTriForm::onClearData);
    // 添加相机采集完成信号连接
    connect(m_ioForm, &ioActionForm::camera1Ready, m_triForm, &CapSenseTriForm::onCamera1Ready);
    connect(m_ioForm, &ioActionForm::camera2Ready, m_triForm, &CapSenseTriForm::onCamera2Ready);
    // DI5-7 产品模式切换
    connect(m_ioForm, &ioActionForm::productModeChangeRequested,
            this, &InitWidget::onProductModeDIRequested);
    connect(m_triForm, &CapSenseTriForm::sigFramResult,
            m_ioForm, &ioActionForm::slotFrameInfo);
    connect(this, &InitWidget::fixtureCodeReceived,
            m_ioForm, &ioActionForm::slotFixtureCodeReceived);
    connect(this, &InitWidget::shellCodeReceived,
            m_ioForm, &ioActionForm::slotShellCodeReceived);

    connect(m_triForm, &CapSenseTriForm::sigCamResult, m_algorithmTester, &AlgorithmTester::onAlgorithmFinished);
    connect(m_saveForm, &SaveForm::sigSaveStateChanged, this, &InitWidget::upSaveImg);
    connect(m_saveForm, &SaveForm::sigSavePathChanged, this, &InitWidget::upSavePath);
    connect(&SysEnv::instance(),&SysEnv::runningChanged,m_triForm, &CapSenseTriForm::onSysRuning);
    connect(&SysEnv::instance(), &SysEnv::scannerCodesCleared,
            this, &InitWidget::onScannerCodesCleared);
    connect(&SysEnv::instance(), &SysEnv::productModeChanged,
            this, &InitWidget::updateProductModeLabel);

    m_algConfigForm = new AlgConfigForm();
    upSaveImg();
}

void InitWidget::refreshModuleStatusText()
{
    const QString mode = SysEnv::instance().getProductMode();
    if (!m_hasModuleStatus) {
        ui->textModuleStatus->setText(tr("当前模式：%1 | Initializing...").arg(mode));
        return;
    }

    QString camA = (m_lastModuleStatusBits & SysEnv::CAM_A) ? tr("CamA: Failed") : tr("CamA: OK");
    QString camB = (m_lastModuleStatusBits & SysEnv::CAM_B) ? tr("CamB: Failed") : tr("CamB: OK");
    QString algo = (m_lastModuleStatusBits & SysEnv::ALGO)  ? tr("Algorithm: Failed") : tr("Algorithm: OK");
    QString plc  = (m_lastModuleStatusBits & SysEnv::PLC)   ? tr("PLC: Failed") : tr("PLC: OK");

    ui->textModuleStatus->setText(tr("当前模式：%1 | %2 | %3 | %4 | %5")
                                      .arg(mode, camA, camB, algo, plc));
}

void InitWidget::updateModuleStatus(quint8 statusBits)
{
    m_lastModuleStatusBits = statusBits;
    m_hasModuleStatus = true;
    refreshModuleStatusText();


    // m_ioAction->setEnabled(!(statusBits & SysEnv::PLC));   // PLC 失败就禁用 IO
    // m_localTestAction->setEnabled(!(statusBits & SysEnv::ALGO)); // Algo 失败就禁用本地测试

    QString style = "QLabel { padding: 2px 5px; border-radius: 3px; ";
    if (statusBits != 240) {
        style += "background-color: #ffcccc; color: #990000;";
        SysEnv::instance().setRunning(false);
    } else {
        style += "background-color: #ccffcc; color: #006600;";//全部成功
        SysEnv::instance().setRunning(true);
    }
    style += " }";
    ui->textModuleStatus->setStyleSheet(style);
}

void InitWidget::initMenuBar()
{
    m_menuBar = ui->menuBar;

    /* 1. Run Mode ------------------------------------------------------ */
    QMenu *runMenu = m_menuBar->addMenu(tr("Run(&R)"));
    m_ioAction = runMenu->addAction(tr("IO Channel (PLC)"));
    m_localTestAction = runMenu->addAction(tr("Local Test (Algo Debug)"));
    connect(m_ioAction,    &QAction::triggered, this, &InitWidget::onIoChannel);
    connect(m_localTestAction, &QAction::triggered, this, &InitWidget::onLocalTest);

    /* 2. System Config ------------------------------------------------- */
    QMenu *setMenu = m_menuBar->addMenu(tr("Settings(&S)"));
    QAction *algoAction = setMenu->addAction(tr("Algorithm Parameters"));
    QAction *saveAction = setMenu->addAction(tr("Save Image Settings"));
    QMenu *modeMenu = setMenu->addMenu(tr("模式切换"));
    m_productModeGroup = new QActionGroup(this);
    m_productModeGroup->setExclusive(true);
    const QString currentMode = SysEnv::instance().getProductMode();
    for (const QString& mode : SysEnv::productModes()) {
        QAction* modeAction = modeMenu->addAction(mode);
        modeAction->setCheckable(true);
        modeAction->setData(mode);
        modeAction->setChecked(mode == currentMode);
        m_productModeGroup->addAction(modeAction);
    }
    connect(algoAction, &QAction::triggered, this, &InitWidget::onAlgoParam);
    connect(saveAction, &QAction::triggered, this, &InitWidget::onSaveImageCfg);
    connect(m_productModeGroup, &QActionGroup::triggered,
            this, &InitWidget::onProductModeSelected);

    /* 3. Data Management ----------------------------------------------- */
    QMenu *dataMenu = m_menuBar->addMenu(tr("Data(&D)"));  // 添加数据管理菜单
    m_historyDataAction = dataMenu->addAction(tr("History Data Query"));
    connect(m_historyDataAction, &QAction::triggered, this, &InitWidget::onHistoryData);

    /* 4. About --------------------------------------------------------- */
    QMenu *helpMenu = m_menuBar->addMenu(tr("Help(&H)"));
    QAction *verAction   = helpMenu->addAction(tr("Version Info"));
    QAction *vendorAction = helpMenu->addAction(tr("Vendor Info"));
    connect(verAction,   &QAction::triggered, this, &InitWidget::onVersionInfo);
    connect(vendorAction,&QAction::triggered, this, &InitWidget::onVendorInfo);

    // m_ioAction->setEnabled(false);
    // m_localTestAction->setEnabled(false);

    updateProductModeLabel(currentMode);
}

void InitWidget::loadQssFile()
{
    QFile file(":/qss/initForm.qss");
    if (file.open(QFile::ReadOnly))
    {
        QTextStream filetext(&file);
        QString stylesheet = filetext.readAll();
        this->setStyleSheet(stylesheet);
        file.close();
    }
}

void InitWidget::addWidget(QWidget *widget)
{
    ui->horizontalLayout->insertWidget(1,widget);
}

void InitWidget::mousePressEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton)
    {
        isPressed=true;
        curPos = event->pos();
    }
}

void InitWidget::mouseMoveEvent(QMouseEvent *event)
{
    if(isPressed)
    {
        //当窗口最大化或最小化时也不进行触发
        if(InitWidget::isMaximized() || InitWidget::isMinimized())
        {
            return;
        }
        else
        {
            //当在按钮之类需要鼠标操作的地方不进行触发(防误触)
            if (ui->btnClose->underMouse()||ui->btnMax->underMouse()||ui->btnMin->underMouse())
            {
                return;
            }
            if(ui->titleWidget->underMouse())
            {
                this->move(event->pos()-curPos+this->pos());    //窗口移动
            }
        }
    }
    event->accept();
}

void InitWidget::mouseReleaseEvent(QMouseEvent *event)
{
    isPressed = false;
}

void InitWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
    Q_UNUSED(event)

    if(ui->titleWidget->underMouse())
    {
        isMaxWin=!isMaxWin;
        if(isMaxWin)
        {

            this->showNormal();
            ui->btnMax->setIcon(QIcon(":/img/max.png"));
        }
        else
        {
            this->showMaximized();
            ui->btnMax->setIcon(QIcon(":/img/mini.png"));
        }
    }
}

void InitWidget::windowMin()
{
    this->showMinimized();
}

void InitWidget::windowMax()
{
    isMaxWin=!isMaxWin;
    if(isMaxWin)
    {
        this->showNormal();
        ui->btnMax->setIcon(QIcon(":/img/max.png"));
    }
    else
    {
        this->showMaximized();
        ui->btnMax->setIcon(QIcon(":/img/mini.png"));
    }
}

void InitWidget::windowClose()
{
    if(!QMessageBox::information(this, tr("info"), tr("Exit the App!"), "Yes", "No")){
        emit appQuit();
        qApp->exit();
    }
}

void InitWidget::show()
{
    this->setVisible(true);
}

void InitWidget::updateTime()
{
    QDateTime time=QDateTime::currentDateTime();
    QString timeText = time.toString("yyyy-MM-dd hh:mm:ss dddd");
    ui->textTime->setText(timeText);
}

void InitWidget::upSaveImg()
{
    m_triForm->setSaveForm(m_saveForm);
}

void InitWidget::upSavePath(const QString &newPath)
{
    m_triForm->upSaveImgPath(newPath);
}

void InitWidget::onIoChannel()
{
    m_ioForm->show();
    m_ioForm->raise();
    m_ioForm->activateWindow();
}

void InitWidget::onLocalTest()
{
    m_algorithmTester->show();
    m_algorithmTester->raise();
    m_algorithmTester->activateWindow();
}

void InitWidget::onAlgoParam()
{
    m_algConfigForm->show();
    m_algConfigForm->raise();
    m_algConfigForm->activateWindow();
}

void InitWidget::onSaveImageCfg()
{
    m_saveForm->show();
    m_saveForm->raise();
    m_saveForm->activateWindow();
}

void InitWidget::onHistoryData()
{
    DataForm *dataForm = new DataForm();
    dataForm->setObjectName("dataForm");
    dataForm->show();
}

void InitWidget::onProductModeSelected(QAction* action)
{
    if (!action) {
        return;
    }

    const QString mode = action->data().toString();
    if (mode == SysEnv::instance().getProductMode()) {
        updateProductModeLabel(mode);
        return;
    }

    SysEnv::instance().setProductMode(mode);
    updateProductModeLabel(SysEnv::instance().getProductMode());
    if (m_triForm) {
        m_triForm->onClearData();
    }
}

void InitWidget::onProductModeDIRequested(const QString &mode)
{
    // DI5-7 低电平触发的模式切换，行为与界面手动切换一致
    if (mode.isEmpty() || mode == SysEnv::instance().getProductMode()) {
        return;
    }

    SysEnv::instance().setProductMode(mode);
    updateProductModeLabel(SysEnv::instance().getProductMode());
    if (m_triForm) {
        m_triForm->onClearData();
    }
}

void InitWidget::updateProductModeLabel(const QString& mode)
{
    if (m_productModeGroup) {
        for (QAction* action : m_productModeGroup->actions()) {
            action->setChecked(action->data().toString() == mode);
        }
    }
    refreshModuleStatusText();
}

void InitWidget::startScannerListening()
{
    if (m_scannerHook || m_scannerThread) {
        return;
    }

    m_scannerThread = new QThread(this);
    m_scannerHook = new ScannerHook();
    m_scannerHook->moveToThread(m_scannerThread);

    connect(m_scannerHook, &ScannerHook::scanFinished,
            this, &InitWidget::handleScannerCode, Qt::QueuedConnection);
    connect(m_scannerThread, &QThread::started, m_scannerHook,
            [scanner = m_scannerHook]() {
                if (!scanner->startContinuous()) {
                    qWarning() << "Failed to start scanner listener";
                }
            });
    connect(m_scannerThread, &QThread::finished,
            m_scannerHook, &QObject::deleteLater);

    m_scannerThread->start();
}

void InitWidget::handleScannerCode(const QString& code)
{
    QString scanCode = code.trimmed();
    if (scanCode.isEmpty()) {
        qWarning() << "Empty scanner code ignored";
        return;
    }

    if (scanCode.length() <= 20) {
        if (!isFixtureCodeFormatValid(scanCode)) {
            qWarning() << "Fixture code rejected: invalid chars" << scanCode;
            showScannerError(tr("治具码无效：只能包含大写字母、数字和-，请重新扫码"));
            return;
        }
        m_pendingFixtureCode = scanCode;
        m_pendingShellCode.clear();
        SysEnv::instance().setScannerCodes(m_pendingFixtureCode, m_pendingShellCode);
        emit fixtureCodeReceived();
        return;
    }

    if (!isShellCodeFormatValid(scanCode)) {
        qWarning() << "Shell code rejected: invalid chars" << scanCode;
        showScannerError(tr("外壳码无效：只能包含大写字母、数字、@和-，请重新扫码"));
        return;
    }

    if (m_pendingFixtureCode.isEmpty()) {
        qWarning() << "Shell code rejected: fixture code is missing";
        showScannerError(tr("请先扫描治具码，再扫描外壳码"));
        return;
    }

    if (!m_pendingShellCode.isEmpty()) {
        qWarning() << "Shell code rejected: current scan cycle is complete";
        showScannerError(tr("当前扫码已完成，请先扫描新的治具码"));
        return;
    }

    m_pendingShellCode = scanCode;
    SysEnv::instance().setScannerCodes(m_pendingFixtureCode, m_pendingShellCode);
    emit shellCodeReceived();
}
void InitWidget::showScannerError(const QString& message)
{
    // 若已有弹窗未关闭，仅刷新其文本，避免同一错误重复弹出叠窗
    if (m_scannerErrorBox && m_scannerErrorBox->isVisible()) {
        m_scannerErrorBox->setText(message);
        m_scannerErrorBox->raise();
        m_scannerErrorBox->activateWindow();
        return;
    }

    if (!m_scannerErrorBox) {
        m_scannerErrorBox = new QMessageBox(QMessageBox::Warning,
                                            tr("扫码错误"),
                                            message,
                                            QMessageBox::Ok,
                                            this);
        m_scannerErrorBox->setAttribute(Qt::WA_DeleteOnClose, false);
        m_scannerErrorBox->setModal(false);
    } else {
        m_scannerErrorBox->setText(message);
    }

    m_scannerErrorBox->show();
    m_scannerErrorBox->raise();
    m_scannerErrorBox->activateWindow();
}

void InitWidget::onScannerCodesCleared()
{
    m_pendingFixtureCode.clear();
    m_pendingShellCode.clear();
}

void InitWidget::onCameraInfo()
{
}

void InitWidget::onLiveImage()
{
}

void InitWidget::onVersionInfo()
{
}

void InitWidget::onVendorInfo()
{
}

void InitWidget::switchLanguage(const QString &lang)
{
    static QTranslator *current = nullptr;
    if (current) { qApp->removeTranslator(current); delete current; }

    current = new QTranslator(this);
    current->load(lang == "zh" ? ":/i18n/MzWeldSight_cn.qm"
                               : ":/i18n/MzWeldSight_en.qm");
    qApp->installTranslator(current);
}

void InitWidget::retranslateMenuBar()
{
}

// 添加清理函数实现
void InitWidget::cleanupExpiredImages()
{
    QString imageSavePath = SysEnv::instance().getImageSavePath();
    int saveDays = SysEnv::instance().getImageSaveDays();

    if (imageSavePath.isEmpty() || saveDays <= 0) {
        qDebug() << "Invalid save path or save days, skip cleanup";
        scheduleDailyCleanup();
        return;
    }

    QDir saveDir(imageSavePath);
    if (!saveDir.exists()) {
        qDebug() << "Save directory does not exist, skip cleanup:" << imageSavePath;
        scheduleDailyCleanup();
        return;
    }

    QDateTime cutoffTime = QDateTime::currentDateTime().addDays(-saveDays);
    qDebug() << "Starting image cleanup. Cutoff time:" << cutoffTime.toString("yyyyMMdd")
             << "Save days:" << saveDays;

    int deletedCount = 0;
    int errorCount = 0;

    // 递归清理所有子目录中的过期文件
    cleanupDirectoryRecursive(imageSavePath, cutoffTime, deletedCount, errorCount);

    qDebug() << "Image cleanup completed. Deleted:" << deletedCount
             << "files, Errors:" << errorCount;

    // 清理完成后重新安排下一次清理
    scheduleDailyCleanup();
}

void InitWidget::cleanupDirectoryRecursive(const QString &directoryPath, const QDateTime &cutoffTime,
                                           int &deletedCount, int &errorCount)
{
    QDir dir(directoryPath);
    if (!dir.exists()) {
        return;
    }

    // 获取目录中的所有条目
    QFileInfoList entries = dir.entryInfoList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);

    foreach (const QFileInfo &entry, entries) {
        if (entry.isDir()) {
            // 递归处理子目录
            cleanupDirectoryRecursive(entry.absoluteFilePath(), cutoffTime, deletedCount, errorCount);

            // 检查目录是否为空，如果为空则删除
            // QDir subDir(entry.absoluteFilePath());
            // if (subDir.isEmpty()) {
            //     if (subDir.rmdir(entry.absoluteFilePath())) {
            //         qDebug() << "Removed empty directory:" << entry.absoluteFilePath();
            //     }
            // }
        } else if (entry.isFile()) {
            // 从文件名中提取时间信息来判断是否过期
            if (isFileExpired(entry.absoluteFilePath(), cutoffTime)) {
                if (QFile::remove(entry.absoluteFilePath())) {
                    deletedCount++;
                    qDebug() << "Deleted expired file:" << entry.fileName();
                } else {
                    errorCount++;
                    qWarning() << "Failed to delete file:" << entry.absoluteFilePath();
                }
            }
        }
    }
}

bool InitWidget::isFileExpired(const QString &filePath, const QDateTime &cutoffTime)
{
    const QFileInfo fileInfo(filePath);
    const QString fileName = fileInfo.fileName();
    // 根据 CameraSaverWorker 中的命名规则解析文件名
    // 文件名格式: yyyyMMdd_HHmmss_zzz_Txxx_Cxx_type.jpg
    // 例如: 20240115_143045_123_T001_C01_composite.jpg

    QRegularExpression timePattern(R"(^(\d{4})(\d{2})(\d{2})_(\d{2})(\d{2})(\d{2})_\d+_)");
    QRegularExpressionMatch match = timePattern.match(fileName);

    if (match.hasMatch()) {
        // 从文件名中提取日期时间
        int year = match.captured(1).toInt();
        int month = match.captured(2).toInt();
        int day = match.captured(3).toInt();
        int hour = match.captured(4).toInt();
        int minute = match.captured(5).toInt();
        int second = match.captured(6).toInt();

        QDateTime fileTime = QDateTime(QDate(year, month, day), QTime(hour, minute, second));

        if (fileTime.isValid()) {
            return fileTime < cutoffTime;
        }
    }

    // 如果无法从文件名解析时间，使用文件的最后修改时间作为备选方案
    if (fileInfo.exists()) {
        return fileInfo.lastModified() < cutoffTime;
    }

    return false;
}

void InitWidget::scheduleDailyCleanup()
{
    // 计算距离下一次清理的时间（毫秒）
    QDateTime now = QDateTime::currentDateTime();
    QDateTime nextCleanup(now.date(), QTime(2, 0, 0));
    if (nextCleanup <= now) {
        nextCleanup = nextCleanup.addDays(1);
    }

    qint64 interval = now.msecsTo(nextCleanup);

    // 设置单次定时器，触发后重新安排
    m_cleanupTimer->setSingleShot(true);
    m_cleanupTimer->start(interval);

    qDebug() << "Next image cleanup scheduled at:" << nextCleanup.toString("yyyy-MM-dd hh:mm:ss")
             << "in" << (interval / 1000 / 3600.0) << "hours";
}
