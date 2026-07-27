#ifndef WELDINGALGORITHMMANAGER_H
#define WELDINGALGORITHMMANAGER_H

#include <QObject>
#include <opencv2/core/mat.hpp>
#include <atomic>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <vector>
#include <QThread>
#include <Common/common.h>
#include <Alg/Api_Welding.h>


namespace Alg {

// 焊接算法管理器类
class WeldingAlgorithmManager
{
public:
    using ResultCallback = std::function<void(const WeldingResult&)>;
    static WeldingAlgorithmManager& instance();

    void setResultCallback(ResultCallback callback);

    bool initialize();
    void processImages(const std::vector<cv::Mat> &images, int id,
                       quint64 inspectionGeneration = 0,
                       const QString& fixtureCode = QString(),
                       const QString& shellCode = QString());
    void processImages(const cv::Mat &image, int id,
                       quint64 inspectionGeneration = 0,
                       const QString& fixtureCode = QString(),
                       const QString& shellCode = QString());
    void processImagesForWeldPoint(const std::vector<cv::Mat> &images, int id, int weldPoint);
    void processImagesForWeldPoint(const cv::Mat &image, int id, int weldPoint);
    int getSignal() const;
    cv::Mat getFusionResult() const;
    cv::Mat getDetectionResult() const;
    WeldingResult getTableData() const;
    void resetWeldPointCounter(quint64 inspectionGeneration);
    void forceResetWeldPointCounter(quint64 inspectionGeneration);
    bool skipWeldPoint(int cameraId, quint64 inspectionGeneration);
    bool isWeldPointCycleComplete();

    void startWorkerThreads();
    void stopWorkerThreads();
private:
    WeldingAlgorithmManager();
    ~WeldingAlgorithmManager();

    // 禁止拷贝和赋值
    WeldingAlgorithmManager(const WeldingAlgorithmManager&) = delete;
    WeldingAlgorithmManager& operator=(const WeldingAlgorithmManager&) = delete;

    void setupWeldingDetector();
    void processingCallback(WeldingResult result);

    void handleInitResult(bool success);

    void fillData(const WeldingResult& result);

    mutable std::mutex m_mutex;
    std::condition_variable m_condition;

    int m_signal;
    cv::Mat m_fusionResult;
    cv::Mat m_detectionResult;
    WeldingResult m_lastResult;
    bool m_resultReceived;

    std::atomic<bool> m_threadsRunning;


    QThread m_initThread;
    std::thread m_producerThread;
    std::thread m_consumerThreads[WORKNUM];
    std::thread m_reportThread;

    bool m_initialized = false;
    bool m_initializing = false;

    ResultCallback m_resultCallback;
};


// 对外接口函数
void CameraFunc(const std::vector<cv::Mat> &images, int id,
                quint64 inspectionGeneration = 0,
                const QString& fixtureCode = QString(),
                const QString& shellCode = QString());
void CameraFunc(const cv::Mat &image, int id,
                quint64 inspectionGeneration = 0,
                const QString& fixtureCode = QString(),
                const QString& shellCode = QString());
void CameraFuncForWeldPoint(const std::vector<cv::Mat> &images, int id, int weldPoint);
void CameraFuncForWeldPoint(const cv::Mat &image, int id, int weldPoint);
int GetSignal();
cv::Mat GetFusionResult();
cv::Mat GetDetectionResult();
WeldingResult GetTableData();
void ResetWeldPointCounter(quint64 inspectionGeneration);
void ForceResetWeldPointCounter(quint64 inspectionGeneration);
bool SkipWeldPoint(int cameraId, quint64 inspectionGeneration);
bool IsWeldPointCycleComplete();

} // namespace Alg


#endif // WELDINGALGORITHMMANAGER_H
