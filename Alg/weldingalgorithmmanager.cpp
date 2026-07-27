#include "weldingalgorithmmanager.h"
#include "Alg/weldinginitializationworker.h"
#include "qapplication.h"
#include <QElapsedTimer>
#include <QThread>
#include <QDebug>
#include <exception>
#include <QCoreApplication>

namespace Alg {

// WeldingAlgorithmManager 实现
WeldingAlgorithmManager::WeldingAlgorithmManager()
    : m_signal(0)
    , m_resultReceived(false)
    , m_threadsRunning(false)
    , m_initialized(false)
{
}

WeldingAlgorithmManager::~WeldingAlgorithmManager()
{
    stopWorkerThreads();
    if (m_initThread.isRunning()) {
        m_initThread.quit();
        m_initThread.wait();
    }
}

WeldingAlgorithmManager& WeldingAlgorithmManager::instance()
{
    static WeldingAlgorithmManager instance;
    return instance;
}



bool WeldingAlgorithmManager::initialize()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_initialized || m_initializing)
        return false;

    m_initializing = true;

    WeldingInitializationWorker* worker = new WeldingInitializationWorker();
    worker->moveToThread(&m_initThread);

    // 线程结束时自动销毁 worker
    QObject::connect(&m_initThread, &QThread::finished,
                     worker, &QObject::deleteLater);

    QObject::connect(worker, &WeldingInitializationWorker::initializationFinished,
                     worker,
                     [this](bool success)
                     {
                         // 把结果异步抛回主线程
                         QMetaObject::invokeMethod(
                             QCoreApplication::instance(),
                             [=]()
                             {
                                 handleInitResult(success);
                             },
                             Qt::QueuedConnection);
                     });

    m_initThread.start();
    QMetaObject::invokeMethod(worker, "doInitialize", Qt::QueuedConnection);
    return true;
}

void WeldingAlgorithmManager::handleInitResult(bool success)
{
    if (success)
    {
        qDebug() << "Welding algorithm initialized, starting worker threads";
        startWorkerThreads();
    }
    else
    {
        qDebug() << "Welding algorithm initialization failed";
    }

    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_initialized = success;
        m_initializing = false;
    }
    m_initThread.quit();
    m_initThread.wait();
}

void WeldingAlgorithmManager::startWorkerThreads()
{
    if (m_threadsRunning.exchange(true)) {
        return;
    }
    Obj<WeldingDetector>::GetInstance()->StartProcessing();

    // 启动生产者线程
    m_producerThread = std::thread([this]() {
        Obj<WeldingDetector>::GetInstance()->ProducerFunc();
    });

    // A single consumer preserves FIFO order while serializing both cameras.
    m_consumerThreads[0] = std::thread([this]() {
        Obj<WeldingDetector>::GetInstance()->ConsumerFunc(0);
    });

    // 启动报告线程
    m_reportThread = std::thread([this]() {
        Obj<WeldingDetector>::GetInstance()->Report([this](WeldingResult result) {
            // 将结果通过Qt信号槽机制传递到主线程
            QMetaObject::invokeMethod(QApplication::instance(), [this, result]() {
                processingCallback(std::move(result));
            }, Qt::QueuedConnection);
        });
    });
}

void WeldingAlgorithmManager::stopWorkerThreads()
{
    if (!m_threadsRunning.exchange(false)) {
        return;
    }

    Obj<WeldingDetector>::GetInstance()->StopProcessing();
    if (m_producerThread.joinable()) {
        m_producerThread.join();
    }
    for (int i = 0; i < WORKNUM; ++i) {
        if (m_consumerThreads[i].joinable()) {
            m_consumerThreads[i].join();
        }
    }
    if (m_reportThread.joinable()) {
        m_reportThread.join();
    }
}

void WeldingAlgorithmManager::setResultCallback(ResultCallback callback)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_resultCallback = std::move(callback);
}

void WeldingAlgorithmManager::processingCallback(WeldingResult result)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_lastResult = result;
    // 调用回调函数
    if (m_resultCallback) {
        m_resultCallback(result);
    }
}

void WeldingAlgorithmManager::processImages(const std::vector<cv::Mat>& images, int id,
                                             quint64 inspectionGeneration,
                                             const QString& fixtureCode,
                                             const QString& shellCode)
{
    std::unique_lock<std::mutex> lock(m_mutex);

    if (!m_initialized) {
        qWarning() << "Algorithm not initialized";
        m_signal = -1;
        return;
    }

    if (id < 0 || id >= WORKNUM) {
        qWarning() << "Invalid camera id:" << id;
        m_signal = -1;
        return;
    }

    if (images.size() != 4) {
        qWarning() << "Expected 4 images, got" << images.size();
        m_signal = -1;
        return;
    }
    // 转换图像格式
    std::vector<cv::Mat> inputImages;
    for (const cv::Mat& img : images) {
        inputImages.push_back(img.clone());
    }

    // 设置输入图像
    Obj<WeldingDetector>::GetInstance()->SetIuputImages(
        inputImages, id, inspectionGeneration, fixtureCode, shellCode);
}

void WeldingAlgorithmManager::processImages(const cv::Mat &image, int id,
                                             quint64 inspectionGeneration,
                                             const QString& fixtureCode,
                                             const QString& shellCode)
{
    std::unique_lock<std::mutex> lock(m_mutex);

    if (!m_initialized) {
        qWarning() << "Algorithm not initialized";
        m_signal = -1;
        return;
    }
    if (id < 0 || id >= WORKNUM || image.empty()) {
        qWarning() << "Invalid single-image algorithm request, camera=" << id;
        m_signal = -1;
        return;
    }

    // 转换图像格式
    std::vector<cv::Mat> inputImages;
    inputImages.push_back(image.clone());
    qWarning() << "WeldingAlgorithmManager::processImages(const cv::Mat &image, int id):"<<id;
    // 设置输入图像
    Obj<WeldingDetector>::GetInstance()->SetIuputImagessignal(
        inputImages[0], id, inspectionGeneration, fixtureCode, shellCode);
}

void WeldingAlgorithmManager::processImagesForWeldPoint(const std::vector<cv::Mat>& images,
                                                         int id, int weldPoint)
{
    std::unique_lock<std::mutex> lock(m_mutex);
    if (!m_initialized || id < 0 || id >= WORKNUM || images.size() != 4) {
        m_signal = -1;
        return;
    }

    std::vector<cv::Mat> inputImages;
    inputImages.reserve(images.size());
    for (const cv::Mat& image : images) {
        inputImages.push_back(image.clone());
    }
    Obj<WeldingDetector>::GetInstance()->SetIuputImagesForWeldPoint(inputImages, id, weldPoint);
}

void WeldingAlgorithmManager::processImagesForWeldPoint(const cv::Mat& image,
                                                         int id, int weldPoint)
{
    std::unique_lock<std::mutex> lock(m_mutex);
    if (!m_initialized || id < 0 || id >= WORKNUM || image.empty()) {
        m_signal = -1;
        return;
    }

    cv::Mat inputImage = image.clone();
    Obj<WeldingDetector>::GetInstance()->SetIuputImagessignalForWeldPoint(inputImage, id, weldPoint);
}


int WeldingAlgorithmManager::getSignal() const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_signal;
}

cv::Mat WeldingAlgorithmManager::getFusionResult() const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_fusionResult.clone();
}

cv::Mat WeldingAlgorithmManager::getDetectionResult() const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_detectionResult.clone();
}

WeldingResult WeldingAlgorithmManager::getTableData() const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_lastResult;
}

void WeldingAlgorithmManager::resetWeldPointCounter(quint64 inspectionGeneration)
{
    Obj<WeldingDetector>::GetInstance()->ResetWeldPointCounter(inspectionGeneration);
}

void WeldingAlgorithmManager::forceResetWeldPointCounter(quint64 inspectionGeneration)
{
    Obj<WeldingDetector>::GetInstance()->ForceResetWeldPointCounter(inspectionGeneration);
}

bool WeldingAlgorithmManager::skipWeldPoint(int cameraId, quint64 inspectionGeneration)
{
    return Obj<WeldingDetector>::GetInstance()->SkipWeldPoint(
        cameraId, inspectionGeneration);
}

bool WeldingAlgorithmManager::isWeldPointCycleComplete()
{
    return Obj<WeldingDetector>::GetInstance()->IsWeldPointCycleComplete();
}

// 对外接口函数实现
void CameraFunc(const std::vector<cv::Mat>& images, int id,
                quint64 inspectionGeneration,
                const QString& fixtureCode,
                const QString& shellCode)
{
    WeldingAlgorithmManager::instance().processImages(
        images, id, inspectionGeneration, fixtureCode, shellCode);
}

int GetSignal()
{
    return WeldingAlgorithmManager::instance().getSignal();
}

cv::Mat GetFusionResult()
{
    return WeldingAlgorithmManager::instance().getFusionResult();
}

cv::Mat GetDetectionResult()
{
    return WeldingAlgorithmManager::instance().getDetectionResult();
}

WeldingResult GetTableData()
{
    return WeldingAlgorithmManager::instance().getTableData();
}

void CameraFunc(const cv::Mat &image, int id,
                quint64 inspectionGeneration,
                const QString& fixtureCode,
                const QString& shellCode)
{
    WeldingAlgorithmManager::instance().processImages(
        image, id, inspectionGeneration, fixtureCode, shellCode);
}

void CameraFuncForWeldPoint(const std::vector<cv::Mat>& images, int id, int weldPoint)
{
    WeldingAlgorithmManager::instance().processImagesForWeldPoint(images, id, weldPoint);
}

void CameraFuncForWeldPoint(const cv::Mat& image, int id, int weldPoint)
{
    WeldingAlgorithmManager::instance().processImagesForWeldPoint(image, id, weldPoint);
}

void ResetWeldPointCounter(quint64 inspectionGeneration)
{
    WeldingAlgorithmManager::instance().resetWeldPointCounter(inspectionGeneration);
}

void ForceResetWeldPointCounter(quint64 inspectionGeneration)
{
    WeldingAlgorithmManager::instance().forceResetWeldPointCounter(inspectionGeneration);
}

bool SkipWeldPoint(int cameraId, quint64 inspectionGeneration)
{
    return WeldingAlgorithmManager::instance().skipWeldPoint(
        cameraId, inspectionGeneration);
}

bool IsWeldPointCycleComplete()
{
    return WeldingAlgorithmManager::instance().isWeldPointCycleComplete();
}

} // namespace Alg
