// capsensetriform.cpp

#include "capsensetriform.h"

#include "CapSenseTri/capsenseinterface.h"
#include "Thread/imagesavemanager.h"
#include "qdebug.h"
#include "qobjectdefs.h"
#include "ui_capsensetriform.h"

#include <QFile>
#include <QFormLayout>
#include <QGraphicsDropShadowEffect>
#include <QLabel>
#include <QTextStream>
#include <QGridLayout>
#include <QDateTime>
#include <QtConcurrent>
#include <QSettings>
#include <QTextCodec>
#include <QDir>
#include <QMessageBox>
#include <QMouseEvent>
#include <QPushButton>
#include <opencv2/imgproc.hpp>
#include <Alg/ImageConvert.h>
#include <Alg/weldingalgorithmmanager.h>
#include <Form/algorithmparammanager.h>
#include <Common/SysEnv.h>
#include <Common/cvqt_text.hpp>

#include <algorithm>

namespace {

QString resultSuffix(DetectionRsult result)
{
    switch (result) {
    case OK: return QStringLiteral("ok");
    case HXAREA_Err: return QStringLiteral("area");
    case HXSIZE_W_Err: return QStringLiteral("width");
    case HXSIZE_H_Err: return QStringLiteral("height");
    case YJ_Err: return QStringLiteral("pin");
    case HXPY_Err: return QStringLiteral("offset");
    case TBDIS_Err: return QStringLiteral("distance");
    case LT_Err: return QStringLiteral("copper");
    case HD_Err: return QStringLiteral("black_dot");
    case HXGAP_Err: return QStringLiteral("gap");
    case NG: return QStringLiteral("ng");
    default: return QStringLiteral("unknown");
    }
}

QString makeResultImageName(int triggerId, int cameraId, DetectionRsult result)
{
    return QString("%1_T%2_C%3_%4.jpg")
        .arg(QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss_zzz"))
        .arg(triggerId, 3, 10, QLatin1Char('0'))
        .arg(cameraId, 2, 10, QLatin1Char('0'))
        .arg(resultSuffix(result));
}

QVector<int> initialDisplayWeldPointIndexes()
{
    return {1, SysEnv::instance().productPStart()};
}

int displayRowsPerSide()
{
    return SysEnv::instance().productNEnd();
}

float resultTextScale(const cv::Mat& image)
{
    if (image.empty()) return 1.0f;

    const float widthScale = static_cast<float>(image.cols) / 640.0f;
    const float heightScale = static_cast<float>(image.rows) / 480.0f;
    const float scale = (std::max)(widthScale, heightScale);
    return (std::max)(1.0f, scale);
}

std::vector<LineStyle> scaleLineStyles(const std::vector<LineStyle>& styles,
                                       float scale)
{
    std::vector<LineStyle> scaled = styles;
    for (LineStyle& style : scaled) {
        style.fontSize = (std::max)(1, static_cast<int>(style.fontSize * scale + 0.5f));
    }
    return scaled;
}

}

DbInsertWorker::DbInsertWorker(const QString &connectionName, QObject *parent)
    : QObject(parent)
    , m_connectionName(connectionName)
{
}

void DbInsertWorker::initialize()
{
    if (m_initialized) {
        return;
    }
    m_sql.init(m_connectionName);
    m_initialized = true;
}

void DbInsertWorker::insertResult(const DB_WeldingResult &result)
{
    if (!m_initialized) {
        initialize();
    }
    if (!m_sql.insertWeldingResult(result)) {
        qWarning() << "Failed to insert welding result in worker for port" << result.hxPort;
    } else {
        qDebug() << "Inserted welding result in worker for port" << result.hxPort;
    }
}


CapSenseTriForm::CapSenseTriForm(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::CapSenseTriForm)
{
    ui->setupUi(this);
    ui->Bt_start->setEnabled(false);
    ui->Bt_start->setFocusPolicy(Qt::NoFocus);
    ui->Bt_clear->setFocusPolicy(Qt::NoFocus);
    m_hxIndex = initialDisplayWeldPointIndexes();
    // 初始化运行时间计时器
    m_runtimeTimer = new QTimer(this);
    m_runtimeSeconds = 0;
    m_runtimeString = "00000:00:00";
    connect(m_runtimeTimer, &QTimer::timeout, this, [this]() {
        m_runtimeSeconds++;
        updateRuntimeDisplay();
    });

    m_imageSaveManager = new ImageSaveManager(this);
    m_imageSaveManager->initialize();

    readConfig();
    initWindow();
    initCameras();

    qRegisterMetaType<cv::Mat>("cv::Mat");
    qRegisterMetaType<DB_WeldingResult>("DB_WeldingResult");
    // 设置结果回调
    Alg::WeldingAlgorithmManager::instance().setResultCallback(
        [this](const WeldingResult& results) {
            // 使用Qt的元对象系统确保在UI线程执行
            QMetaObject::invokeMethod(this, "onAlgorithmResultReady",
                                    //测试使用
                                      //Qt::DirectConnection,
                                      //正常运行使用
                                      Qt::QueuedConnection,
                                      Q_ARG(WeldingResult,results));
        });
    // 连接参数变更信号
    connect(AlgorithmParamManager::instance(), &AlgorithmParamManager::algParamChanged,
            this, &CapSenseTriForm::onAlgParamChanged);
    connect(&SysEnv::instance(), &SysEnv::weldPointResetMessageChanged,
            this, &CapSenseTriForm::onWeldPointResetMessage);
    connect(&SysEnv::instance(), &SysEnv::weldPointCountChanged,
            this, &CapSenseTriForm::onWeldPointCountChanged);
    connect(&SysEnv::instance(), &SysEnv::scannerCodesChanged,
            this, &CapSenseTriForm::onScannerCodesChanged);

    // 初始化本地参数
    m_currentAlgParam = AlgorithmParamManager::instance()->getAlgParam();
    Obj<WeldingDetector>::GetInstance()->SetThresholdParams(m_currentAlgParam);

    // 从 SysEnv 加载自启动设置
    ui->cb_autoStart->setChecked(SysEnv::instance().getAutoStart());

    m_sql.init("MainForm_db");
    m_dbThread = new QThread(this);
    m_dbWorker = new DbInsertWorker("MainForm_db_worker");
    m_dbWorker->moveToThread(m_dbThread);
    connect(m_dbThread, &QThread::started, m_dbWorker, &DbInsertWorker::initialize);
    connect(m_dbThread, &QThread::finished, m_dbWorker, &QObject::deleteLater);
    connect(this, &CapSenseTriForm::sigDbInsert, m_dbWorker, &DbInsertWorker::insertResult,
            Qt::QueuedConnection);
    m_dbThread->start();
    // 重置图像计数
    resetCameraImageCount(1);
    resetCameraImageCount(2);
}

CapSenseTriForm::~CapSenseTriForm()
{
    releaseAllCameras();
    if (m_dbThread) {
        m_dbThread->quit();
        m_dbThread->wait();
    }
    delete ui;
}

void CapSenseTriForm::handleFrame(CFrameInfo frameInfo)
{
    int currentId = getCameraID(frameInfo.cameraKey);
    if (currentId <= 0) {
        qWarning() << "Invalid camera ID:" << frameInfo.cameraKey;
        return;
    }

    auto& targetList = (currentId == 1) ? m_vListC1 : m_vListC2;
    int& imageCount = (currentId == 1) ? m_camera1ImageCount : m_camera2ImageCount;

    // 转换为Mat
    cv::Mat frameMat;
    try {
        switch(frameInfo.m_ePixelType) {
        case gvspPixelRGB8:
            frameMat = cv::Mat(frameInfo.m_nHeight,
                               frameInfo.m_nWidth,
                               CV_8UC3,
                               frameInfo.m_pImageBuf.get()).clone();
            cv::cvtColor(frameMat, frameMat, cv::COLOR_RGB2BGR);
            break;
        case gvspPixelMono8:
        {
            cv::Mat gray = cv::Mat(
                               frameInfo.m_nHeight,
                               frameInfo.m_nWidth,
                               CV_8UC1,
                               frameInfo.m_pImageBuf.get()
                               ).clone();
            cv::cvtColor(gray, frameMat, cv::COLOR_GRAY2BGR);
            break;
        }
        default:
            qWarning() << "Unsupported format:" << frameInfo.m_ePixelType;
            return;
        }
    } catch (const cv::Exception& e) {
        qWarning() << "OpenCV exception:" << e.what();
        return;
    }

    m_imageSaveManager->saveRawImage(currentId-1, frameMat, m_vTriggerIndexs[currentId-1], m_vImgIndexs[currentId-1]);

    // --- 保护措施 1：时间间隔自愈 (针对丢图场景) ---
    // 如果距离上一帧已超过 800ms，判定为新周期开始，强制清理之前不完整的残留帧
    if (imageCount > 0 && m_lastFrameTimer[currentId-1].isValid() && m_lastFrameTimer[currentId-1].elapsed() > 800) {
        qWarning() << QString("Camera %1: 检测到跨周期残余图像 (间隔 %2ms)，已强制清理重置")
                        .arg(currentId).arg(m_lastFrameTimer[currentId-1].elapsed());
        
        // 可视化提示：由于图片不齐导致丢弃
        generateNGResult(currentId, QStringLiteral("图像不足 (%1/4)").arg(imageCount));
        
        resetCameraImageCount(currentId);
        imageCount = 0; // 重置本地局部引用，确保当前帧作为第 1 张存入
    }
    m_lastFrameTimer[currentId-1].start(); // 更新帧到达时间

    // --- 保护措施 2：数量饱和自愈 (针对丢信号场景) ---
    // 如果内存已满 4 张但迟迟没等到信号，现在又来新图，说明物理周期已切换，之前的 4 张已过期
    if (imageCount >= 4) {
        bool waitingForTrigger = (currentId == 1) ? m_waitingForTrigger1 : m_waitingForTrigger2;
        if (!waitingForTrigger) {
            qWarning() << QString("Camera %1: 检测到信号缺失(丢号)，已强制清理 4 张过期图像").arg(currentId);
            
            // 可视化提示：信号缺失警告
            generateNGResult(currentId, QStringLiteral("信号缺失"));
            
            resetCameraImageCount(currentId);
            imageCount = 0;
        } else {
            // 信号已就绪且图像已齐，应该立即触发处理（而不是等待）
            // 但此处已经是第5张及以后的图像，属于杂讯帧，直接过滤
            qDebug() << QString("Camera %1: 屏蔽多余杂讯帧 (当前已有 %2 张)").arg(currentId).arg(imageCount);
            return;
        }
    }

    // 针对多余杂讯帧进行200ms处理
    if (m_lastTriggerTime[currentId-1].isValid() && m_lastTriggerTime[currentId-1].elapsed() < 200) {
        return;
    }

    // 添加到列表
    targetList.push_back(std::move(frameMat));
    imageCount++;
    qDebug() << QString("Camera %1: 收到第 %2 张图像，总计 %3/4 张")
                    .arg(currentId)
                    .arg(m_vImgIndexs[currentId-1])
                    .arg(imageCount);
    // 增加原始图像索引
    m_vImgIndexs[currentId-1]++;
    // 注意：这里不再自动处理4张图像，改为等待PLC触发信号
    // 如果“挂号信号”已经先到了，且此时图片齐了，立即触发
    if (imageCount >= 4) {
        checkAndProcessCameraImages(currentId);
    }
}

void CapSenseTriForm::onAlgorithmResultReady(const WeldingResult& result)
{
    if (result.id < 0 || result.id >= 2) {
        qWarning() << "Ignoring algorithm result with invalid camera id:" << result.id;
        return;
    }
    if (m_isRun && result.inspectionGeneration != m_inspectionGeneration) {
        qWarning() << "Ignoring stale algorithm result, camera=" << result.id + 1
                   << "result generation=" << result.inspectionGeneration
                   << "current generation=" << m_inspectionGeneration;
        return;
    }
    if (!m_isRun && result.inspectionGeneration != 0) {
        qWarning() << "Ignoring production result after detection stopped, camera="
                   << result.id + 1;
        return;
    }
    if (result.dst.empty())
    {
        qWarning() << "CapSenseTriForm: received empty cv::Mat";
        return;
    }
    qDebug() << "[CB] cam=" << (result.id + 1) << "res=" << result.NO_HX_NGReults
             << "ts=" << QDateTime::currentMSecsSinceEpoch();
    qDebug() << "[T3] Callback start";
    int cameraId = result.id+1;

    CapSenseInterface face;
    if(m_isRun){
        CFrameResult framData;
        framData.sig = 1;
        framData.camera = cameraId;
        framData.result = result.NO_HX_NGReults;
        emit sigFramResult(framData);

        QStringList lines;
        std::vector<LineStyle> styles;
        QString strNGReults;

        // 根据检测结果设置颜色
        QColor rowColor = Qt::green; // 默认颜色
        QVector<int> errorColumns; // 记录所有错误列的索引
        bool hasError = false;

        // 检查 HX_NGReult 位掩码，确定所有错误类型
        QStringList errorTypes;
        QVector<int> errorColIndices;
        // 检查每个错误位
        if (result.HX_NGReult & 0b10000000) { // HXAREA_Err
            errorTypes << tr("%1(%2)").arg(tr("HXAREA_Err")).arg(result.HX_Area, 0, 'f', 2);
            errorColIndices << 3; // Area列
            hasError = true;
        }
        if (result.HX_NGReult & 0b01000000) { // HXSIZE_W_Err
            if (!result.hxResult.empty()) {
                errorTypes << tr("%1(%2)").arg(tr("HXSIZE_W_Err")).arg(result.hxResult[0].hx_weight, 0, 'f', 2);
            } else {
                errorTypes << tr("HXSIZE_W_Err");
            }
            errorColIndices << 1; // Weight列
            hasError = true;
        }
        if (result.HX_NGReult & 0b00100000) { // HXSIZE_H_Err
            if (!result.hxResult.empty()) {
                errorTypes << tr("%1(%2)").arg(tr("HXSIZE_H_Err")).arg(result.hxResult[0].hx_height, 0, 'f', 2);
            } else {
                errorTypes << tr("HXSIZE_H_Err");
            }
            errorColIndices << 2; // Height列
            hasError = true;
        }
        if (result.HX_NGReult & 0b00010000) { // YJ_Err
            errorTypes << tr("YJ_Err");
            errorColIndices << 10; // 结果列
            hasError = true;
        }
        if (result.HX_NGReult & 0b00001000) { // HXPY_Err
            if (!result.hxResult.empty()) {
                errorTypes << tr("%1(%2)").arg(tr("HXPY_Err")).arg(result.hxResult[0].center_pianyi, 0, 'f', 2);
            } else {
                errorTypes << tr("HXPY_Err");
            }
            errorColIndices << 5; // Offset列
            hasError = true;
        }
        if (result.HX_NGReult & 0b00000100) { // TBDIS_Err
            if (!result.hxResult.empty()) {
                errorTypes << tr("%1(%2)").arg(tr("TBDIS_Err")).arg(result.hxResult[0].TBtoHX_distance, 0, 'f', 2);
            } else {
                errorTypes << tr("TBDIS_Err");
            }
            errorColIndices << 4; // TBtoHX列
            hasError = true;
        }
        if (result.HX_NGReult & 0b00000010) { // LT_Err
            errorTypes << tr("%1(%2)").arg(tr("LT_Err")).arg(QString("%1/%2").arg(result.LT_SumArea, 0, 'f', 2).arg(result.LT_MaxArea, 0, 'f', 2));
            errorColIndices << 6; // LTsum列
            hasError = true;
        }
        if (result.HX_NGReult & 0b00000001) { // HD_Err
            errorTypes << tr("%1(%2)").arg(tr("HD_Err")).arg(QString("%1/%2").arg(result.HD_SumArea, 0, 'f', 2).arg(result.HD_MaxArea, 0, 'f', 2));
            errorColIndices << 8; // HDsum列
            hasError = true;
        }
        if (result.HX_NGReult & 0x0100) { // HXGAP_Err
            errorTypes << tr("%1(%2)").arg(tr("HXGAP_Err")).arg(result.HX_DiffArea, 0, 'f', 2);
            errorColIndices << 10;
            hasError = true;
        }
        if (result.NO_HX_NGReults == NG) {
            errorTypes << tr("NG");
            errorColIndices << 10; // 结果列
            hasError = true;
        }

        // 构建显示文本
        if (result.HX_NGReult == 0x00) {
            // OK情况
            strNGReults = tr("OK");
            lines << strNGReults;
            styles.push_back(LineStyle{40, Qt::green, "Consolas", QFont::Bold});
            rowColor = Qt::green;
        } else if (hasError) {
            // 有错误的情况
            // 第一行显示主要错误类型
            if (!errorTypes.isEmpty()) {
                lines << errorTypes.first();
                styles.push_back(LineStyle{30, Qt::red, "Consolas"});
            }

            // 如果有多个错误类型，显示在后续行
            for (int i = 1; i < errorTypes.size(); i++) {
                lines << errorTypes[i];
                styles.push_back(LineStyle{30, Qt::red, "Consolas"});
            }

            // 如果包含面积错误，显示详细信息
            if (result.HX_NGReult & 0b10000000) {
                lines << tr("%1(%2)").arg(tr("LT_SUM")).arg(QString("%1/%2").arg(result.LT_SumArea, 0, 'f', 2).arg(result.LT_MaxArea, 0, 'f', 2));
                styles.push_back(LineStyle{30, Qt::red, "Consolas"});
                lines << tr("%1(%2)").arg(tr("HD_SUM")).arg(QString("%1/%2").arg(result.HD_SumArea, 0, 'f', 2).arg(result.HD_MaxArea, 0, 'f', 2));
                styles.push_back(LineStyle{30, Qt::red, "Consolas"});
            }

            rowColor = Qt::red;
            errorColumns = errorColIndices;
        }

        // 存档图：在原分辨率上按比例放大字号，保证存档文字清晰
        const float textScale = resultTextScale(result.dst);
        const std::vector<LineStyle> scaledStyles = scaleLineStyles(styles, textScale);
        const cv::Point textOrigin(
            static_cast<int>(30 * textScale + 0.5f),
            static_cast<int>(100 * textScale + 0.5f));
        const int lineSpacing = static_cast<int>(10 * textScale + 0.5f);
        QImage archivePix = cvqt::drawImgText(result.dst, lines, scaledStyles,
                                              textOrigin, lineSpacing);

        // 界面显示：先把结果图缩放到长边 640 的固定画布，再用固定字号画字，
        // 这样无论原图分辨率多大，界面上的文字大小都保持一致
        cv::Mat displaySrc = result.dst;
        // 特殊旋转焊点的结果是横版图，显示前拉回竖版尺寸，
        // 保证界面上与未旋转焊点的显示大小一致（仅影响显示，不影响算法和存档）
        if (displaySrc.cols > displaySrc.rows) {
            cv::resize(displaySrc, displaySrc,
                       cv::Size(displaySrc.rows, displaySrc.cols),
                       0, 0, cv::INTER_LINEAR);
        }
        const double displayScale =
            640.0 / (std::max)(displaySrc.cols, displaySrc.rows);
        cv::Mat displayMat;
        cv::resize(displaySrc, displayMat, cv::Size(),
                   displayScale, displayScale, cv::INTER_AREA);
        QImage pix = cvqt::drawImgText(displayMat, lines, styles,
                                       cv::Point(30, 100), 10);
        face.setDisplayImage(result.id, 1, pix);

        QStringList headers = {};
        // 记录当前行索引
        int currentRowIndex = m_resultData[result.id].size();
        int displayIdx = m_hxIndex[result.id];
        // 构建结果描述字符串（包含所有错误类型）
        QString fullErrorDesc;
        if (result.HX_NGReult == 0x00) {
            fullErrorDesc = tr("OK");
        } else if (result.NO_HX_NGReults == NG) {
            fullErrorDesc = tr("NG");
        } else {
            fullErrorDesc = errorTypes.join("; ");
        }

        for (const auto& hx : result.hxResult)
        {
            m_resultData[result.id] << QVariant(QVariantList{
                QString::number(displayIdx),
                QString::number(hx.hx_weight, 'f', 2),
                QString::number(hx.hx_height, 'f', 2),
                QString::number(result.HX_Area,   'f', 2),
                QString::number(hx.TBtoHX_distance, 'f', 2),
                QString::number(hx.center_pianyi,   'f', 2),
                QString::number(result.LT_SumArea,   'f', 2),
                QString::number(result.LT_MaxArea,   'f', 2),
                QString::number(result.HD_SumArea,   'f', 2),
                QString::number(result.HD_MaxArea,   'f', 2),
                fullErrorDesc  // 使用完整的错误描述
            });
            break;
        }

        if (m_resultData[result.id].size() == 1) {
            face.clearAllRowTextColors(result.id);
            face.clearAllCellTextColors(result.id);
        }
        face.setTableData(result.id, headers, m_resultData[result.id]);
        // 根据检测结果设置颜色
        if (hasError) {
            // 设置整行颜色为红色
            face.setRowTextColor(result.id, currentRowIndex, Qt::green);

            // 设置所有错误列的颜色为红色
            QColor errorCellColor = Qt::red;
            for (int errorColumn : errorColumns) {
                face.setCellTextColor(result.id, currentRowIndex, errorColumn, errorCellColor);
            }
            // 始终高亮显示第一列（ID）和最后一列（结果）
            face.setCellTextColor(result.id, currentRowIndex, 0, errorCellColor);
            face.setCellTextColor(result.id, currentRowIndex, 10, errorCellColor);
        } else {
            // OK结果设置为绿色
            face.setRowTextColor(result.id, currentRowIndex, Qt::green);
        }

        if (result.id == 0) {
            m_hxIndex[result.id]++;
        } else {
            m_hxIndex[result.id]--;
        }

        if (m_resultData[result.id].size() >= displayRowsPerSide()) {
            m_resultData[result.id].clear();
            m_hxIndex[result.id] = (result.id == 0)
                ? 1
                : SysEnv::instance().productPStart();
        }

        // 保存合成图像
        qDebug() << "[T4] Save start";
        m_imageSaveManager->saveCompositeImage(cameraId, result.FusionMap, m_vTriggerIndexs[result.id]);

        WeldingResult weData;

        weData.NO_HX_NGReults = result.NO_HX_NGReults;
        weData.HX_NGReult = result.HX_NGReult; // 保存位掩码
        weData.dst = cvqt::qImageToCvMat(archivePix);
        const QString resultImageName = makeResultImageName(
            m_vTriggerIndexs[result.id], cameraId, result.NO_HX_NGReults);
        // 保存检测结果图像
        m_imageSaveManager->saveResultImage(
            cameraId, weData, m_vTriggerIndexs[result.id], resultImageName);
        qDebug() << "[T5] Save end";

        // 重置原始图像索引，为下一次触发做准备

        m_vImgIndexs[result.id] = 1;
        m_vTriggerIndexs[result.id]++;
        if(cameraId ==1){
            m_ccd1_num++;
            if(result.NO_HX_NGReults != OK){
                m_ccd1_ng++;
            }
            if (m_ccd1_num > 0) {
                m_ccd1_rate = (float)(m_ccd1_num - m_ccd1_ng) / m_ccd1_num * 100.0f;
            } else {
                m_ccd1_rate = 0.0f; // 避免除以0
            }

        }

        if(cameraId ==2){
            m_ccd2_num++;
            if(result.NO_HX_NGReults != OK){
                m_ccd2_ng++;
            }
            if (m_ccd2_num > 0) {
                m_ccd2_rate = (float)(m_ccd2_num - m_ccd2_ng) / m_ccd2_num * 100.0f;
            } else {
                m_ccd2_rate = 0.0f; // 避免除以0
            }
        }
        QList<QPair<QString, QVariant>> okRateData = {
            {tr("CCD1:"), QString("%1%").arg(m_ccd1_rate, 0, 'f', 2)},
            {tr("CCD2:"), QString("%1%").arg(m_ccd2_rate, 0, 'f', 2)}
        };

        QList<QPair<QString, QVariant>> ngCountData = {
            {tr("CCD1:"), QString::number(m_ccd1_ng)},
            {tr("CCD2:"), QString::number(m_ccd2_ng)}
        };

        QList<QPair<QString, QVariant>> totalData = {
            {tr("CCD1:"), QString::number(m_ccd1_num)},
            {tr("CCD2:"), QString::number(m_ccd2_num)}
        };
        updateDynamicGroupBox(ui->gBox_ok, okRateData);
        updateDynamicGroupBox(ui->gBox_ng, ngCountData);
        updateDynamicGroupBox(ui->gBox_total, totalData);

        // ========== 调用数据库存储函数 ==========
        if (!result.hxResult.empty()) {
            const auto& hx = result.hxResult[0]; // 取第一个焊锡结果
            qDebug() << "[T6] DB start";

            DB_WeldingResult dbResult;
            dbResult.hxPort = (cameraId == 1)? "N端":"P端";                    // 相机端口
            dbResult.hxId = displayIdx;                       // 焊锡编号
            dbResult.hxBatch = 1;                          // 批次，没有的话记为1
            dbResult.hxWeight = hx.hx_weight;              // 焊锡长
            dbResult.hxHeight = hx.hx_height;              // 焊锡宽
            dbResult.hxArea = result.HX_Area;              // 焊锡面积
            dbResult.hxDistance = hx.TBtoHX_distance;      // 裸露铜引脚长度
            dbResult.hxPianyi = hx.center_pianyi;          // 焊锡偏移
            dbResult.hxLTSumArea = result.LT_SumArea;      // 漏铜总面积
            dbResult.hxLTMaxArea = result.LT_MaxArea;      // 漏铜最大面积
            dbResult.hxHDSumArea = result.HD_SumArea;      // 黑点总面积
            dbResult.hxHDMaxArea = result.HD_MaxArea;      // 黑点最大面积
            dbResult.hxReult = (result.NO_HX_NGReults == OK); // 检测结果
            dbResult.hxReultsDescribe = fullErrorDesc; // 结果描述
            dbResult.classifyStr = SysEnv::instance().getClassifyStr(); // 二维码
            dbResult.fixtureCode = result.fixtureCode;
            dbResult.shellCode = result.shellCode;
            dbResult.imageCode = resultImageName;

            if (m_enableDbInsert) {
                emit sigDbInsert(dbResult);
            }
            qDebug() << "[T7] DB end";
        }
        // ========== 数据库存储结束 ==========
    }else{

        qDebug() << "CapSenseTriForm::onAlgorithmResultReady() sigCamResult";
        //本地测试
        emit sigCamResult(result);
    }
    qDebug() << "[T8] Callback end";
}


void CapSenseTriForm::onSysRuning(bool on)
{
    ui->Bt_start->setEnabled(on);
    if (!on && m_isRun) {
        on_Bt_start_clicked();
        return;
    }
    // 如果系统开始运行且启用了自启动，并且当前不在运行状态，则自动启动
    if (on && SysEnv::instance().getAutoStart() && !m_isRun) {
        qDebug() << "Auto starting detection...";
        // 延迟一小段时间确保UI完全就绪
        QTimer::singleShot(500, this, [this]() {
            if (SysEnv::instance().isRunning() &&
                SysEnv::instance().getAutoStart() && !m_isRun) {
                on_Bt_start_clicked();
            }
        });
    }
}

void CapSenseTriForm::updateDataGroup(DataGroup group, const QList<QPair<QString, QVariant>> &data)
{
    switch(group) {
    case ConfigData:
        updateDynamicGroupBox(ui->gBox_config, data);
        break;
    case OkRateData:
        updateDynamicGroupBox(ui->gBox_ok, data);
        break;
    case NgCountData:
        updateDynamicGroupBox(ui->gBox_ng, data);
        break;
    case TotalData:
        updateDynamicGroupBox(ui->gBox_total, data);
        break;
    }
}

CapSenseForm *CapSenseTriForm::getDisplay(int index) const
{
    if(index >= 0 && index < displays.size()) {
        return displays[index];
    }
    return nullptr;
}

int CapSenseTriForm::getCameraID(const QString &cameraKey)
{
    static const QHash<QString,int> tbl = {
        {m_cameraKey1, 1},
        {m_cameraKey2, 2}
    };
    return tbl.value(cameraKey, 0);   // 找不到返回 0
}

inline QGraphicsDropShadowEffect *CapSenseTriForm::createShadow()
{
    QGraphicsDropShadowEffect* shadow = new QGraphicsDropShadowEffect(this);
    shadow->setOffset(0, 0);
    shadow->setColor(QColor(150, 150, 150));
    shadow->setBlurRadius(6);
    return shadow;
}

void CapSenseTriForm::updateDynamicGroupBox(QGroupBox *groupBox, const QList<QPair<QString, QVariant>> &data)
{
    QLayout* oldLayout = groupBox->layout();
    if (oldLayout) {
        // 先检查布局是否有子项
        while (oldLayout->count() > 0) {
            QLayoutItem* item = oldLayout->takeAt(0);
            if (item) {
                if (item->widget()) {
                    delete item->widget();
                }
                delete item;
            }
        }
        delete oldLayout;
    }

    QFormLayout* layout = new QFormLayout(groupBox);
    layout->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
    layout->setContentsMargins(12, 15, 12, 12);
    layout->setSpacing(8);

    for (const auto &pair : data) {
        QLabel* valueLabel = new QLabel(pair.second.toString());
        valueLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        valueLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

        QLabel* keyLabel = new QLabel(pair.first);
        keyLabel->setStyleSheet("font-weight: bold;");

        layout->addRow(keyLabel, valueLabel);
    }

    groupBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
}

void CapSenseTriForm::initCameras()
{
    //注册数据类型 不然信号槽会数据丢失
    qRegisterMetaType<CFrameInfo>("CFrameInfo");

    int ret = IMV_OK;
    // 连接相机之前不显示状态栏 | Don't show status bar before connecting camera

    ret = IMV_EnumDevices(&m_deviceInfoList, interfaceTypeAll);
    if (IMV_OK != ret)
    {
        printf("Enumeration devices failed! ErrorCode[%d]\n", ret);
        return;
    }
    if (m_deviceInfoList.nDevNum < 1)
    {
        ui->Bt_start->setEnabled(false);
    }
    else
    {
        createCameraThread(m_deviceInfoList);
    }
}

void CapSenseTriForm::createCameraThread(IMV_DeviceList m_deviceInfoList)
{
    for (unsigned int i = 0; i < m_deviceInfoList.nDevNum; i++)
    {
        QString cameraKey = m_deviceInfoList.pDevInfo[i].cameraKey;
        qDebug()<<"createCameraThread:" << cameraKey;
        QString cameraName = QString::fromLocal8Bit(m_deviceInfoList.pDevInfo[i].cameraName);
        QString ccdInfo;
        CameraThread* thread = new CameraThread(cameraKey);
        connect(thread, &CameraThread::sendAreaCamImage, this, &CapSenseTriForm::handleFrame);
        // connect(thread, &CameraThread::sendScanCamImage, this, &CapSenseTriForm::woodFrame);
        connect(thread, &CameraThread::errorOccurred, this, &CapSenseTriForm::handleError);

        cameraThreads.insert(cameraKey, thread);
        m_cameraThreadList.append(thread); // 添加到列表

        ccdInfo = QString("%1[%2]").arg(cameraKey).arg(cameraName);
        if (cameraKey == m_cameraKey1) {
            thread->m_worker->m_isAreaScan = true;
            thread->m_worker->m_nId = 1;
            SysEnv::instance().setModuleOk(SysEnv::CAM_A);
        } else if (cameraKey == m_cameraKey2) {
            thread->m_worker->m_isAreaScan = true;
            thread->m_worker->m_nId = 2;
            SysEnv::instance().setModuleOk(SysEnv::CAM_B);
        }
        thread->initialize();
    }
}

void CapSenseTriForm::releaseAllCameras()
{
    for (auto* thread : std::as_const(cameraThreads)) {
        thread->stop();
        delete thread;
    }
    cameraThreads.clear();
    m_cameraThreadList.clear(); // 清空列表
}

void CapSenseTriForm::upSaveImgPath(const QString &newPath)
{
    qDebug() << "Save path changed to:" << newPath;

    if (m_imageSaveManager) {
        m_imageSaveManager->updateSavePath(newPath);
    }
}

void CapSenseTriForm::updateRuntimeDisplay()
{
    qint64 hours = m_runtimeSeconds / 3600;
    qint64 minutes = (m_runtimeSeconds % 3600) / 60;
    qint64 seconds = m_runtimeSeconds % 60;

    m_runtimeString = QString("%1:%2:%3")
                          .arg(hours, 5, 10, QLatin1Char('0'))
                          .arg(minutes, 2, 10, QLatin1Char('0'))
                          .arg(seconds, 2, 10, QLatin1Char('0'));

    // 更新配置数据显示
    QList<QPair<QString, QVariant>> configData = {
        {"机械编号:", 1},
        {"运行时间:", m_runtimeString}
    };
    updateDynamicGroupBox(ui->gBox_config, configData);
}

void CapSenseTriForm::setSaveForm(SaveForm *saveForm)
{
    m_saveForm = saveForm;
    m_imageSaveManager->setSaveForm(m_saveForm);
}

void CapSenseTriForm::onAlgParamChanged(const InThresholdParam &param)
{
    m_currentAlgParam = param;
    Obj<WeldingDetector>::GetInstance()->SetThresholdParams(param);
    QStringList headers;
    headers << tr("HXID")
            << tr("Weight(%1~%2mm)").arg(param.min_w).arg(param.max_w)
            << tr("Height(%1~%2mm)").arg(param.min_h).arg(param.max_h)
            << tr("Area(%1~%2mm²)").arg(param.min_area).arg(param.max_area)
            << tr("TBtoHX(%1~%2mm)").arg(param.min_TBDis).arg(param.max_TBDis)
            << tr("Offset(%1~%2mm)").arg(param.min_PY).arg(param.max_PY)
            << tr("LTsum(0~%1mm²)").arg(param.LT_ALLArea_max)
            << tr("LTmax(0~%1mm²)").arg(param.LT_MAXArea_max)
            << tr("HDsum(0~%1mm²)").arg(param.HD_ALLArea_max)
            << tr("HDmax(0~%1mm²)").arg(param.HD_MAXArea_max)
            << tr("NO_HX_NGResults");

    QVariantList rowData;
    /* 旧数据不动，只更新表头 */
    CapSenseInterface face;
    face.setTableData(0, headers, rowData);
    face.setTableData(1, headers, rowData);
}

// 添加检查和处理图像的函数 (改为非阻塞状态机模式)
void CapSenseTriForm::checkAndProcessCameraImages(int cameraId)
{
    // 1. 获取当前状态
    int imageCount = (cameraId == 1) ? m_camera1ImageCount : m_camera2ImageCount;
    bool& waitingForTrigger = (cameraId == 1) ? m_waitingForTrigger1 : m_waitingForTrigger2;
    auto& targetList = (cameraId == 1) ? m_vListC1 : m_vListC2;

    // 2. 只有当“PLC信号已到”且“4张图像已齐”时，才进行处理
    if (!waitingForTrigger || imageCount < 4) {
        // 条件不满足，静默退出，等待下一次（信号或图像）到达
        return;
    }

    // 3. 到达这里，说明挂号窗口已就绪，开始处理
    qDebug() << QString("Camera %1: 触发条件达成 (信号已就绪, 图像数量: %2/4)").arg(cameraId).arg(imageCount);

    // 安全检查：确保 targetList 中确实有足够的图像
    if (targetList.size() < 4) {
        qWarning() << QString("Camera %1: targetList 数量不足 (%2/4)，跳过处理")
                      .arg(cameraId).arg(targetList.size());
        resetCameraImageCount(cameraId);
        return;
    }

    // 容错处理：如果图像 > 4 张，取前 4 张
    // 注意：必须使用 clone() 深拷贝，因为 targetList 会在异步任务执行前被清空
    std::vector<cv::Mat> imagesToProcess;
    for (int i = 0; i < 4; ++i) {
        imagesToProcess.push_back(targetList[i].clone());
    }

    // 重置触发标志，防止重复进入
    waitingForTrigger = false;
    
    // 开启屏蔽保护期
    m_lastTriggerTime[cameraId-1].restart();

    qDebug() << "[T1] Async Algo dispatch start";
    // 将耗时的算法分发移至子线程，彻底解除主线程阻塞
    const quint64 generation = m_inspectionGeneration;
    const QString fixtureCode = SysEnv::instance().getFixtureCode();
    const QString shellCode = SysEnv::instance().getShellCode();
    QtConcurrent::run([imagesToProcess, cameraId, generation, fixtureCode, shellCode]() {
        Alg::CameraFunc(imagesToProcess, cameraId - 1,
                        generation, fixtureCode, shellCode);
    });
    qDebug() << "[T2] Async Algo dispatch end";

    m_processStartTime = QDateTime::currentMSecsSinceEpoch();

    // 处理完成后，重置计数（注意：resetCameraImageCount 会清空 list）
    resetCameraImageCount(cameraId);

    qDebug() << QString("Camera %1: 图像处理已开始").arg(cameraId);
}

// 添加NG结果生成函数
void CapSenseTriForm::generateNGResult(int cameraId, const QString& reason)
{
    qDebug() << QString("Camera %1: 生成NG结果 - %2").arg(cameraId).arg(reason);

    if (!Alg::SkipWeldPoint(cameraId - 1, m_inspectionGeneration)) {
        qWarning() << "NG result ignored until the next inspection cycle, camera=" << cameraId;
        return;
    }

    // 参考on_Bt_clear_clicked中的makeFake函数，创建简洁的测试数据
    WeldingResult r;
    r.id = cameraId - 1;  // 转换为0-based索引
    r.inspectionGeneration = m_inspectionGeneration;
    r.fixtureCode = SysEnv::instance().getFixtureCode();
    r.shellCode = SysEnv::instance().getShellCode();
    r.NO_HX_NGReults = NG;  // 设为NG
    r.HX_NGReult = 0xFF;     // 全NG位掩码

    // 创建红色NG图像
    r.FusionMap = cv::Mat(480, 640, CV_8UC3, cv::Scalar(0, 0, 255));  // 红色背景
    r.dst = r.FusionMap.clone();

    // 使用一致的绘制方式显示中文提示
    QStringList lines;
    std::vector<LineStyle> styles;
    
    lines << "NG";
    styles.push_back(LineStyle{80, Qt::white, "Consolas", QFont::Bold});
    
    lines << reason;
    styles.push_back(LineStyle{40, Qt::yellow, "Microsoft YaHei", QFont::Normal});

    QImage pix = cvqt::drawImgText(r.dst, lines, styles, cv::Point(100, 150), 30);
    r.dst = cvqt::qImageToCvMat(pix); // 更新 dst 为带文字的图像

    // 添加一条焊锡结果（参考测试数据）
    HXResult h;
    h.hxid = 0;  // 类似测试数据的ID
    h.hx_weight = 0.0f;    // NG时设为0
    h.hx_height = 0.0f;    // NG时设为0
    h.TBtoHX_distance = 0.0f;
    h.center_pianyi = 0.0f;
    r.hxResult.emplace_back(h);

    r.HX_Area = 0.0f;
    r.LT_SumArea = 0.0f;
    r.LT_MaxArea = 0.0f;
    r.HD_SumArea = 0.0f;
    r.HD_MaxArea = 0.0f;

    onAlgorithmResultReady(r);
}

// 添加图像计数重置函数
void CapSenseTriForm::resetCameraImageCount(int cameraId)
{
    if (cameraId == 1) {
        m_camera1ImageCount = 0;
        m_vListC1.clear();
        m_waitingForTrigger1 = false;
        qDebug() << "Camera 1 图像计数已重置";
    } else if (cameraId == 2) {
        m_camera2ImageCount = 0;
        m_vListC2.clear();
        m_waitingForTrigger2 = false;
        qDebug() << "Camera 2 图像计数已重置";
    }
}

// 添加相机1完成信号处理
void CapSenseTriForm::onCamera1Ready()
{
    qDebug() << "收到相机1采集完成信号";

    if (m_isRun) {
        // 信号自愈：如果信号到了，但发现内存里的图是很久以前的残余，直接清理
        if (m_camera1ImageCount > 0 && m_lastFrameTimer[0].isValid() && m_lastFrameTimer[0].elapsed() > 800) {
             qWarning() << "Camera 1: 信号到达时发现过期残余图像，已强制重置对齐";
             resetCameraImageCount(1);
        }

        m_waitingForTrigger1 = true;
        checkAndProcessCameraImages(1);
    } else {
        qDebug() << "系统未运行，忽略相机1信号";
    }
}

// 添加相机2完成信号处理
void CapSenseTriForm::onCamera2Ready()
{
    qDebug() << "收到相机2采集完成信号";

    if (m_isRun) {
        // 信号自愈：如果信号到了，但发现内存里的图是很久以前的残余，直接清理
        if (m_camera2ImageCount > 0 && m_lastFrameTimer[1].isValid() && m_lastFrameTimer[1].elapsed() > 800) {
             qWarning() << "Camera 2: 信号到达时发现过期残余图像，已强制重置对齐";
             resetCameraImageCount(2);
        }

        m_waitingForTrigger2 = true;
        checkAndProcessCameraImages(2);
    } else {
        qDebug() << "系统未运行，忽略相机2信号";
    }
}

// 修改现有的清除数据函数
void CapSenseTriForm::onClearData()
{
    // 清除结果数据
    m_resultData[0].clear();
    m_resultData[1].clear();
    m_hxIndex = initialDisplayWeldPointIndexes();
    m_vImgIndexs = {1, 1};

    // 重置图像计数
    resetCameraImageCount(1);
    resetCameraImageCount(2);
    // 扫码内容只允许由真实鼠标清除或由新治具码替换。
    ++m_inspectionGeneration;
    Alg::ForceResetWeldPointCounter(m_inspectionGeneration);

    // 清空两个显示表格的内容
    CapSenseInterface face;
    QVariantList emptyData;
    QStringList emptyHeaders;

    // 清空表格1（索引0）
    face.setTableData(0, emptyHeaders, emptyData);
    face.clearTableData(0);

    // 清空表格2（索引1）
    face.setTableData(1, emptyHeaders, emptyData);
    face.clearTableData(1);

    // 清除颜色设置
    face.clearAllRowTextColors(0);
    face.clearAllCellTextColors(0);
    face.clearAllRowTextColors(1);
    face.clearAllCellTextColors(1);

    qDebug() << "所有数据已清除，图像计数已重置";
}


void CapSenseTriForm::loadQssFile()
{
    QFile file(":/qss/CapSense.qss");
    if (file.open(QFile::ReadOnly))
    {
        QTextStream filetext(&file);
        QString stylesheet = filetext.readAll();
        this->setStyleSheet(stylesheet);
        file.close();
    }
}

void CapSenseTriForm::on_Bt_clear_clicked()
{
    QMessageBox::StandardButton btn =
        QMessageBox::question(this,
                              tr("info"),
                              tr("Are you sure you want to clear all statistics?"),
                              QMessageBox::Yes | QMessageBox::No,
                              QMessageBox::No);

    if (btn != QMessageBox::Yes)
        return;


    m_runtimeSeconds = 0;
    QList<QPair<QString, QVariant>> configData = {
        {"机械编号:", 1},
        {"运行时间:", "00000:00:00"}
    };
    QList<QPair<QString, QVariant>> okRateData = {
        {tr("CCD1:"), "00.00%"},
        {tr("CCD2:"), "00.00%"}
    };
    QList<QPair<QString, QVariant>> ngCountData = {
        {tr("CCD1:"), "00"},
        {tr("CCD2:"), "00"}
    };
    QList<QPair<QString, QVariant>> totalData = {
        {tr("CCD1:"), "00"},
        {tr("CCD2:"), "00"}
    };
    m_ccd1_rate = 0;
    m_ccd1_ng = 0;
    m_ccd1_num = 0;

    m_ccd2_rate = 0;
    m_ccd2_ng = 0;
    m_ccd2_num = 0;
    updateDynamicGroupBox(ui->gBox_config, configData);
    updateDynamicGroupBox(ui->gBox_ok,     okRateData);
    updateDynamicGroupBox(ui->gBox_ng,     ngCountData);
    updateDynamicGroupBox(ui->gBox_total,  totalData);

    // m_isRun = 1;
    // /* 随机数引擎 */
    // static std::mt19937 rng(std::random_device{}());
    // static std::uniform_int_distribution<int> dist(OK, NG);

    // // 映射函数：将 DetectionResult 转换为对应的 HX_NGReult 位掩码
    // auto mapDetectionToHXMask = [](DetectionRsult result) -> unsigned char {
    //     switch(result) {
    //     case OK: return 0x00;            // 完全OK
    //     case HXAREA_Err: return 0b10000000;   // 面积错误
    //     case HXSIZE_W_Err: return 0b01000000; // 宽度错误
    //     case HXSIZE_H_Err: return 0b00100000; // 高度错误
    //     case YJ_Err: return 0b00010000;       // 引脚错误
    //     case HXPY_Err: return 0b00001000;     // 偏移错误
    //     case TBDIS_Err: return 0b00000100;    // 铜板距离错误
    //     case LT_Err: return 0b00000010;       // 漏铜错误
    //     case HD_Err: return 0b00000001;       // 黑点错误
    //     case NG: return 0xFF;                 // 完全NG
    //     default: return 0x00;
    //     }
    // };

    // auto makeFake = [&](int camId) -> WeldingResult
    // {
    //     WeldingResult r;
    //     r.id = camId;

    //     // 随机选择一个 DetectionResult
    //     r.NO_HX_NGReults = static_cast<DetectionRsult>(dist(rng));

    //     // 转换为对应的 HX_NGReult 位掩码
    //     r.HX_NGReult = mapDetectionToHXMask(r.NO_HX_NGReults);

    //     // 创建简单的测试图像，不绘制文本（onAlgorithmResultReady 会处理）
    //     if (r.NO_HX_NGReults == OK) {
    //         r.FusionMap = cv::Mat(480, 640, CV_8UC3, cv::Scalar(50, 205, 50)); // 浅绿色
    //     } else if (r.NO_HX_NGReults == NG) {
    //         r.FusionMap = cv::Mat(480, 640, CV_8UC3, cv::Scalar(50, 50, 205)); // 浅蓝色
    //     } else {
    //         // 具体错误类型
    //         r.FusionMap = cv::Mat(480, 640, CV_8UC3, cv::Scalar(205, 165, 50)); // 浅橙色
    //     }
    //     r.dst = r.FusionMap.clone();

    //     // 随便填充焊锡结果数据
    //     for (int i = 0; i < 1; ++i) {
    //         HXResult h;
    //         h.hxid = 1000 + camId * 10 + i;
    //         h.hx_weight = 5.0f + i * 2.0f;
    //         h.hx_height = 3.0f + i * 1.0f;
    //         h.TBtoHX_distance = 1.0f + i * 0.5f;
    //         h.center_pianyi = 0.5f + i * 0.3f;
    //         r.hxResult.emplace_back(h);
    //     }

    //     // 随便填充其他字段
    //     r.HX_Area = 25.0 + camId * 5.0;
    //     r.HD_SumArea = (r.NO_HX_NGReults == HD_Err) ? 5.0 : 0.0;
    //     r.HD_MaxArea = (r.NO_HX_NGReults == HD_Err) ? 2.0 : 0.0;
    //     r.LT_SumArea = (r.NO_HX_NGReults == LT_Err) ? 8.0 : 0.0;
    //     r.LT_MaxArea = (r.NO_HX_NGReults == LT_Err) ? 4.0 : 0.0;

    //     r.NG_OKflag = (r.NO_HX_NGReults != OK);
    //     r.HD_Flag = (r.NO_HX_NGReults == HD_Err);
    //     r.LT_Flag = (r.NO_HX_NGReults == LT_Err);

    //     return r;
    // };

    // // 生成并处理测试数据
    // onAlgorithmResultReady(makeFake(0));
    // onAlgorithmResultReady(makeFake(1));
}

void CapSenseTriForm::handleError(QString message)
{
    qDebug()<< message;
}

void CapSenseTriForm::initWindow()
{
    gridLayout_displays = new QGridLayout();
    gridLayout_displays->setSpacing(2);

    displays.clear();
    for (int i = 0; i < 2; ++i) {
        CapSenseForm* display = new CapSenseForm(this);
        display->setObjectName(QString("display_%1").arg(i + 1));
        displays.append(display);

        gridLayout_displays->addWidget(display, 0, i);
    }

    QWidget* leftContainer = new QWidget(this);
    leftContainer->setLayout(gridLayout_displays);

    QHBoxLayout* mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    mainLayout->setSpacing(2);
    mainLayout->addWidget(leftContainer, 5);
    mainLayout->addWidget(ui->display_sidebar, 1);

    QList<QPair<QString, QVariant>> configData = {
        {"机械编号:", 1},
        {"运行时间:", "00000:00:00"}
    };
    QList<QPair<QString, QVariant>> okRateData = {
        {tr("CCD1:"), "00.00%"},
        {tr("CCD2:"), "00.00%"}
    };
    QList<QPair<QString, QVariant>> ngCountData = {
        {tr("CCD1:"), "00"},
        {tr("CCD2:"), "00"}
    };
    QList<QPair<QString, QVariant>> totalData = {
        {tr("CCD1:"), "00"},
        {tr("CCD2:"), "00"}
    };
    updateDynamicGroupBox(ui->gBox_config, configData);
    updateDynamicGroupBox(ui->gBox_ok, okRateData);
    updateDynamicGroupBox(ui->gBox_ng, ngCountData);
    updateDynamicGroupBox(ui->gBox_total, totalData);

    QGroupBox* weldPointGroup = new QGroupBox(tr("焊点计数"), this);
    weldPointGroup->setObjectName("gBox_weldPoint");
    weldPointGroup->setAlignment(Qt::AlignCenter);
    updateDynamicGroupBox(weldPointGroup, {
        {tr("N端:"), "0"},
        {tr("P端:"), "0"}
    });
    ui->verticalLayout->insertWidget(
        ui->verticalLayout->indexOf(ui->gBox_total) + 1, weldPointGroup);

    QGroupBox* scannerCodeGroup = new QGroupBox(tr("扫码内容"), this);
    scannerCodeGroup->setObjectName("gBox_scannerCode");
    scannerCodeGroup->setAlignment(Qt::AlignCenter);
    updateDynamicGroupBox(scannerCodeGroup, {
        {tr("治具码:"), tr("未扫码")},
        {tr("外壳码:"), tr("未扫码")}
    });
    ui->verticalLayout->insertWidget(
        ui->verticalLayout->indexOf(weldPointGroup) + 1, scannerCodeGroup);

    QPushButton* clearScannerBtn = new QPushButton(tr("清除扫码"), this);
    clearScannerBtn->setObjectName("Bt_clearScanner");
    clearScannerBtn->setFocusPolicy(Qt::NoFocus);
    clearScannerBtn->installEventFilter(this);
    ui->verticalLayout->insertWidget(
        ui->verticalLayout->indexOf(scannerCodeGroup) + 1, clearScannerBtn);
    // 可选：设置自启动复选框样式
    ui->cb_autoStart->setStyleSheet(
        "QCheckBox {"
        "    spacing: 5px;"
        "    font-weight: bold;"
        "}"
        "QCheckBox::indicator {"
        "    width: 16px;"
        "    height: 16px;"
        "}"
        );
}

void CapSenseTriForm::onWeldPointResetMessage(const QString& message, bool warning)
{
    QGroupBox* groupBox = findChild<QGroupBox*>("gBox_weldPoint");
    if (!groupBox) return;

    groupBox->setToolTip(message);
    if (warning) {
        qWarning() << message;
    } else {
        qInfo() << message;
    }
}

void CapSenseTriForm::onWeldPointCountChanged(int nWeldPoint, int pWeldPoint,
                                              const QString& message, bool warning)
{
    Q_UNUSED(warning);
    QGroupBox* groupBox = findChild<QGroupBox*>("gBox_weldPoint");
    if (!groupBox) return;

    groupBox->setToolTip(message);
    updateDynamicGroupBox(groupBox, {
        {tr("N端:"), QString::number(nWeldPoint)},
        {tr("P端:"), QString::number(pWeldPoint)}
    });
}

void CapSenseTriForm::onScannerCodesChanged(const QString& fixtureCode,
                                             const QString& shellCode)
{
    if (!shellCode.isEmpty()) {
        ++m_inspectionGeneration;
        Alg::ResetWeldPointCounter(m_inspectionGeneration);
    }

    QGroupBox* groupBox = findChild<QGroupBox*>("gBox_scannerCode");
    if (!groupBox) return;

    groupBox->setToolTip(QString("%1\n%2").arg(fixtureCode, shellCode));
    updateDynamicGroupBox(groupBox, {
        {tr("治具码:"), fixtureCode.isEmpty() ? tr("未扫码") : fixtureCode},
        {tr("外壳码:"), shellCode.isEmpty() ? tr("未扫码") : shellCode}
    });
}

bool CapSenseTriForm::eventFilter(QObject *watched, QEvent *event)
{
    if (watched && watched->objectName() == QStringLiteral("Bt_clearScanner")) {
        auto *button = qobject_cast<QPushButton *>(watched);
        if (button && event->type() == QEvent::MouseButtonPress) {
            const auto *mouseEvent = static_cast<QMouseEvent *>(event);
            m_clearScannerMousePressed = mouseEvent->spontaneous()
                    && mouseEvent->button() == Qt::LeftButton
                    && button->rect().contains(mouseEvent->pos());
        } else if (button && event->type() == QEvent::MouseButtonRelease) {
            const auto *mouseEvent = static_cast<QMouseEvent *>(event);
            const bool isPhysicalClick = m_clearScannerMousePressed
                    && mouseEvent->spontaneous()
                    && mouseEvent->button() == Qt::LeftButton
                    && button->rect().contains(mouseEvent->pos());
            m_clearScannerMousePressed = false;
            if (isPhysicalClick) {
                QTimer::singleShot(0, this, [this]() {
                    clearScannerCodesFromMouse();
                });
            }
        }
    }
    return QWidget::eventFilter(watched, event);
}

void CapSenseTriForm::clearScannerCodesFromMouse()
{
    const auto button = QMessageBox::question(
        this, tr("确认"), tr("确定要清除当前扫码内容吗？清除后需要重新扫码。"),
        QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    if (button == QMessageBox::Yes) {
        onClearData();
        SysEnv::instance().clearScannerCodes();
    }
}

void CapSenseTriForm::on_Bt_start_clicked()
{
    enableLocalMode = false;
    if(m_isRun){
        ui->Bt_start->setText(tr("start"));
        m_runtimeTimer->stop();
        for (CameraThread* thread : m_cameraThreadList) {
            thread->stop();
        }
    } else {
        ui->Bt_start->setText(tr("stop"));
        m_runtimeTimer->start(1000);
        for (CameraThread* thread : m_cameraThreadList) {
            thread->start();
        }
    }
    m_isRun = 1 - m_isRun;
}

void CapSenseTriForm::readConfig()
{
    QString Path = "configs/configs.ini";
    QSettings iniRead(Path, QSettings::IniFormat);
    iniRead.setIniCodec(QTextCodec::codecForName("UTF_8"));

    // 相机1~2的key
    m_cameraKey1 = iniRead.value("CameraKey/cameraKey1").toString();
    m_cameraKey2 = iniRead.value("CameraKey/cameraKey2").toString();

}

void CapSenseTriForm::on_cb_autoStart_clicked(bool checked)
{
    // 保存设置到 SysEnv
    SysEnv::instance().setAutoStart(checked);
    qDebug() << "Auto start setting changed to:" << checked;
}
