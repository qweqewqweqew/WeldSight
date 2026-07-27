#include "Api_Welding.h"
#include "solderjointalgorithmcore.h"
#ifdef emit
#undef emit
#define MZ_RESTORE_QT_EMIT
#endif
#ifdef signals
#undef signals
#define MZ_RESTORE_QT_SIGNALS
#endif
#include "infer.h"
#ifdef MZ_RESTORE_QT_SIGNALS
#define signals Q_SIGNALS
#undef MZ_RESTORE_QT_SIGNALS
#endif
#ifdef MZ_RESTORE_QT_EMIT
#define emit Q_EMIT
#undef MZ_RESTORE_QT_EMIT
#endif
#include <sstream>
#include <iomanip>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <string>
#include <filesystem>
#include <algorithm>
#include <QDebug>
#include <QDateTime>
#include <QElapsedTimer>
#include <windows.h>
#include <mutex>
#include <utility>
#include <Common/SysEnv.h>
namespace fs = std::filesystem;
long long count = 1;

struct WeldingDetector::DetectorModels
{
    std::vector<OVYOLOV26> detector;
    std::vector<OVYOLOV26> segmenter;

    DetectorModels()
        : detector(WORKNUM)
        , segmenter(WORKNUM)
    {}
};

namespace {

std::mutex g_weldPointMutex;
int g_nWeldPointNo = 1;
int g_pWeldPointNo = 0;
int g_lastNWeldPointNo = 0;
int g_lastPWeldPointNo = 0;
bool g_weldPointCycleComplete = false;
quint64 g_inspectionGeneration = 0;

void publishWeldPointCountsLocked(const QString& message = QString(), bool warning = false)
{
    SysEnv::instance().publishWeldPointCountChanged(
        g_lastNWeldPointNo, g_lastPWeldPointNo, message, warning);
}

bool allWeldPointsComplete()
{
    const SysEnv& env = SysEnv::instance();
    return g_nWeldPointNo > env.productNEnd() &&
           g_pWeldPointNo < env.productPEnd();
}

void resetWeldPointCountersLocked()
{
    g_nWeldPointNo = 1;
    g_pWeldPointNo = SysEnv::instance().productPStart();
    g_lastNWeldPointNo = 0;
    g_lastPWeldPointNo = 0;
    g_weldPointCycleComplete = false;
}

int nextWeldPointNo(int cameraId, quint64 inspectionGeneration)
{
    std::lock_guard<std::mutex> lock(g_weldPointMutex);
    if (inspectionGeneration != g_inspectionGeneration) {
        qWarning() << "Ignoring stale inspection task, task generation="
                   << inspectionGeneration << "current generation="
                   << g_inspectionGeneration;
        return 0;
    }
    if (g_weldPointCycleComplete) {
        const QString message = QObject::tr(
            "%1 inspections complete. Waiting for the next inspection cycle.")
                .arg(SysEnv::instance().productTotalPoints());
        SysEnv::instance().publishWeldPointResetMessage(message, true);
        publishWeldPointCountsLocked(message, true);
        return 0;
    }

    int weldPoint = 0;
    const SysEnv& env = SysEnv::instance();
    if (cameraId == 0 && g_nWeldPointNo <= env.productNEnd()) {
        weldPoint = g_nWeldPointNo++;
        g_lastNWeldPointNo = weldPoint;
    } else if (cameraId == 1 && g_pWeldPointNo >= env.productPEnd()) {
        weldPoint = g_pWeldPointNo--;
        g_lastPWeldPointNo = weldPoint;
    } else {
        publishWeldPointCountsLocked();
        return 0;
    }

    g_weldPointCycleComplete = allWeldPointsComplete();
    if (g_weldPointCycleComplete) {
        const QString message = QObject::tr(
            "%1 inspections complete. Waiting for the next inspection cycle.")
                .arg(SysEnv::instance().productTotalPoints());
        SysEnv::instance().publishWeldPointResetMessage(message, true);
        publishWeldPointCountsLocked(message, true);
    } else {
        publishWeldPointCountsLocked();
    }
    return weldPoint;
}

void resetWeldPointNo(quint64 inspectionGeneration)
{
    std::lock_guard<std::mutex> lock(g_weldPointMutex);
    g_inspectionGeneration = inspectionGeneration;
    resetWeldPointCountersLocked();
    const QString message = QObject::tr(
        "Inspection cycle ready. Waiting for %1 inspections to complete.")
            .arg(SysEnv::instance().productTotalPoints());
    SysEnv::instance().publishWeldPointResetMessage(message, false);
    publishWeldPointCountsLocked(message, false);
}

void forceResetWeldPointNo(quint64 inspectionGeneration)
{
    std::lock_guard<std::mutex> lock(g_weldPointMutex);
    g_inspectionGeneration = inspectionGeneration;
    resetWeldPointCountersLocked();
    const QString message = QObject::tr("Current inspection data cleared.");
    SysEnv::instance().publishWeldPointResetMessage(message, false);
    publishWeldPointCountsLocked(message, false);
}

bool isWeldPointCycleComplete()
{
    std::lock_guard<std::mutex> lock(g_weldPointMutex);
    return g_weldPointCycleComplete;
}

void rotateByExplicitWeldPoint(cv::Mat& image, int cameraId, int weldPoint)
{
    if (image.empty() || weldPoint <= 0) return;
    if (!SysEnv::instance().productUsesSpecialRotation()) return;

    if ((cameraId == 0 && weldPoint == 1) ||
        (cameraId == 1 && weldPoint == 19)) {
        cv::rotate(image, image, cv::ROTATE_90_COUNTERCLOCKWISE);
    } else if (cameraId == 1 && weldPoint == 18) {
        cv::rotate(image, image, cv::ROTATE_90_CLOCKWISE);
    }
}

}

WeldingDetector::WeldingDetector()
    : m_models(std::make_unique<DetectorModels>())
{
    m_Consumers = std::vector<std::thread>(WORKNUM);
    conf_thresh = 0.5;
    nms_thresh = 0.3;
    Segconf_thresh = 0.5;
    Segnms_thresh = 0.3;
    m_initalized = false;
}

WeldingDetector::~WeldingDetector() = default;

void WeldingDetector::ResetWeldPointCounter(quint64 inspectionGeneration)
{
    resetWeldPointNo(inspectionGeneration);
}

void WeldingDetector::ForceResetWeldPointCounter(quint64 inspectionGeneration)
{
    forceResetWeldPointNo(inspectionGeneration);
}

bool WeldingDetector::SkipWeldPoint(int cameraId, quint64 inspectionGeneration)
{
    return nextWeldPointNo(cameraId, inspectionGeneration) > 0;
}

bool WeldingDetector::IsWeldPointCycleComplete()
{
    return isWeldPointCycleComplete();
}

void WeldingDetector::StartProcessing()
{
    m_running.store(true);
}

void WeldingDetector::StopProcessing()
{
    m_running.store(false);
    cvr.notify_all();
}

void WeldingDetector::SetThresholdParams(const InThresholdParam& params)
{
    std::lock_guard<std::mutex> lock(m_paramMtx);
    m_inParam = params;
}

void WeldingDetector::SetAutoCalibration(bool enabled)
{
    std::lock_guard<std::mutex> lock(m_paramMtx);
    m_inParam.AutoBDswitch = enabled;
}

bool WeldingDetector::Initialize(Params params)
{
    std::string weldingdetectorfile = params.weldingdetectorfile;
    conf_thresh = params.conf_thresh;
    nms_thresh = params.nms_thresh;
    std::string Segweldingdetectorfile = params.Seg_weldingdetectorfile;
    Segconf_thresh = params.Seg_conf_thresh;
    Segnms_thresh = params.Seg_nms_thresh;

    weldingdetectorfile = GetProgramDir() + "/weights/" + weldingdetectorfile;
    Segweldingdetectorfile = GetProgramDir() + "/weights/" + Segweldingdetectorfile;
    qDebug() << "1" << QString::fromStdString(weldingdetectorfile);
    qDebug() << "2" << QString::fromStdString(Segweldingdetectorfile);
    
    for (int i = 0; i < WORKNUM; i++)
    {
        qDebug() << "Loading models for worker" << i << "...";
        m_models->segmenter[i].LoadOVModel(Segweldingdetectorfile.c_str(), -1, false);
        qDebug() << "Seg model" << i << "loaded OK";
        m_models->detector[i].LoadOVModel(weldingdetectorfile.c_str(), -1, false);
        qDebug() << "Detector model" << i << "loaded OK";
        cv::Mat image = cv::Mat::zeros(cv::Size(640, 640), CV_8UC3);
        std::vector<BBox> bboxs;
        std::vector<Instance> Segbboxs;
        float confidence = 0.5;
        for (int j = 0; j < 5; j++)
        {
            m_models->detector[i].Infer(image, bboxs, confidence);
            m_models->segmenter[i].SegInfer(image, Segbboxs, confidence);
        }
    }

    m_initalized = true;
    return true;
}
float scale_grobal = 1, scale_grobalPlus = 1;
cv::Mat WeldingDetector::resize_with_padding(const cv::Mat& input, int target_width, int target_height,
                                             const cv::Scalar& padding_color) {
    // 获取原始尺寸
    int original_width = input.cols;
    int original_height = input.rows;

    // 计算缩放比例
    float scale = (std::min)(static_cast<float>(target_width) / original_width,
                             static_cast<float>(target_height) / original_height);
    scale_grobal = scale;
    scale_grobalPlus = scale_grobal;

    // 计算新尺寸（保持宽高比）
    int new_width = static_cast<int>(original_width * scale);
    int new_height = static_cast<int>(original_height * scale);

    // 调整图像大小
    cv::Mat resized;
    cv::resize(input, resized, cv::Size(new_width, new_height), 0, 0, cv::INTER_LINEAR);

    // 创建目标图像并填充
    cv::Mat output(target_height, target_width, input.type());
    output.setTo(padding_color);

    // 计算放置位置（居中）
    int x_offset = (target_width - new_width) / 2;
    int y_offset = (target_height - new_height) / 2;

    // 确保 ROI 不超出边界
    x_offset = (((0) > (x_offset)) ? (0) : (x_offset));
    y_offset = (((0) > (y_offset)) ? (0) : (y_offset));
    new_width = (((new_width) < (target_width - x_offset)) ? (new_width) : (target_width - x_offset));
    new_height = (((new_height) < (target_height - y_offset)) ? (new_height) : (target_height - y_offset));

    // 将调整后的图像复制到中心
    if (new_width > 0 && new_height > 0) {
        cv::Mat roi(output, cv::Rect(x_offset, y_offset, new_width, new_height));

        // 确保类型匹配
        if (resized.type() != roi.type()) {
            cv::Mat converted;
            resized.convertTo(converted, roi.type());
            converted.copyTo(roi);
        }
        else {
            resized.copyTo(roi);
        }
    }

    return output;
}

cv::Mat WeldingDetector::resize_with_paddingPlus(const cv::Mat& input, int target_width, int target_height,
                                                 const cv::Scalar& padding_color) {
    // 获取原始尺寸
    int original_width = input.cols;
    int original_height = input.rows;

    // 计算缩放比例
    float scale = (std::min)(static_cast<float>(target_width) / original_width,
                             static_cast<float>(target_height) / original_height);
    scale_grobalPlus = scale * scale_grobalPlus;

    // 计算新尺寸（保持宽高比）
    int new_width = static_cast<int>(original_width * scale);
    int new_height = static_cast<int>(original_height * scale);

    // 调整图像大小
    cv::Mat resized;
    cv::resize(input, resized, cv::Size(new_width, new_height), 0, 0, cv::INTER_LINEAR);

    // 创建目标图像并填充
    cv::Mat output(target_height, target_width, input.type());
    output.setTo(padding_color);

    // 计算放置位置（居中）
    int x_offset = (target_width - new_width) / 2;
    int y_offset = (target_height - new_height) / 2;

    // 确保 ROI 不超出边界
    x_offset = (((0) > (x_offset)) ? (0) : (x_offset));
    y_offset = (((0) > (y_offset)) ? (0) : (y_offset));
    new_width = (((new_width) < (target_width - x_offset)) ? (new_width) : (target_width - x_offset));
    new_height = (((new_height) < (target_height - y_offset)) ? (new_height) : (target_height - y_offset));

    // 将调整后的图像复制到中心
    if (new_width > 0 && new_height > 0) {
        cv::Mat roi(output, cv::Rect(x_offset, y_offset, new_width, new_height));

        // 确保类型匹配
        if (resized.type() != roi.type()) {
            cv::Mat converted;
            resized.convertTo(converted, roi.type());
            converted.copyTo(roi);
        }
        else {
            resized.copyTo(roi);
        }
    }

    return output;
}

// 多光源图像拼接函数
cv::Mat WeldingDetector::MergeMultiLightImages(const std::vector<cv::Mat>& images, int fusionMethod = 0)
{
    if (images.empty()) return cv::Mat();

    // Step 1: 图像对齐（假设已粗略对齐，否则需特征匹配）
    std::vector<cv::Mat> alignedImages = images;

    // Step 2: 光照归一化（CLAHE增强对比度）
    std::vector<cv::Mat> normalizedImages;
    cv::Ptr<cv::CLAHE> clahe = cv::createCLAHE(2.0, cv::Size(32, 32));
    for (const auto& img : alignedImages) {
        cv::Mat normalized;
        if (img.channels() == 3) {
            std::vector<cv::Mat> channels;
            cv::split(img, channels);
            for (auto& ch : channels) clahe->apply(ch, ch);
            cv::merge(channels, normalized);
        }
        else {
            clahe->apply(img, normalized);
        }
        normalizedImages.push_back(normalized);
    }
    //std::vector<cv::Mat> normalizedImages = images;
    // Step 3: 图像融合（根据需求选择方法）
    cv::Mat result;
    if (fusionMethod == 0) {
        // 方法1: 简单平均（适用于均匀光照补偿）
        result = cv::Mat::zeros(normalizedImages[0].size(), normalizedImages[0].type());
        for (const auto& img : normalizedImages) {
            result += img / static_cast<int>(normalizedImages.size());
        }
    }
    else {
        // 方法2: 最大值合成（突出不同光源的特征）
        result = normalizedImages[0].clone();
        for (size_t i = 1; i < normalizedImages.size(); ++i) {
            //std::max(result, normalizedImages[i], result);
        }
    }
    return result;
}

void WeldingDetector::SetIuputImages(std::vector<cv::Mat>& image, int id,
                                     quint64 inspectionGeneration,
                                     const QString& fixtureCode,
                                     const QString& shellCode)
{
    if (image.empty() || image.size() != 4)
        return;
    else
    {
        //拼接裁剪图像
        cv::Mat src;
        int firstWidth = image[0].cols;
        int firstHeight = image[0].rows;
        for (int i = 0; i < image.size(); i++)
        {
            if (image[i].cols != firstWidth || image[i].rows != firstHeight)
            {
                src = image[0];
                break;
            }
        }
        if (src.empty())
        {
            src = MergeMultiLightImages(image);
        }
        const int weldPoint = nextWeldPointNo(id, inspectionGeneration);
        if (weldPoint <= 0) {
            qWarning() << "Inspection ignored until the next inspection cycle, camera=" << id + 1;
            return;
        }
        rotateByExplicitWeldPoint(src, id, weldPoint);
        // 使用 m_queueMtx 保护队列操作，防止并发写入导致数据竞争
        {
            std::lock_guard<std::mutex> lock(m_queueMtx);
            WeldingInputTask task;
            task.image = src.clone();
            task.cameraId = id;
            task.inspectionGeneration = inspectionGeneration;
            task.fixtureCode = fixtureCode;
            task.shellCode = shellCode;
            m_Images.push(std::move(task));
        }
    }
}

void WeldingDetector::SetIuputImagesForWeldPoint(std::vector<cv::Mat>& image, int id, int weldPoint)
{
    if (image.empty() || image.size() != 4) return;

    cv::Mat src;
    const int firstWidth = image[0].cols;
    const int firstHeight = image[0].rows;
    for (const cv::Mat& frame : image) {
        if (frame.cols != firstWidth || frame.rows != firstHeight) {
            src = image[0];
            break;
        }
    }
    if (src.empty()) {
        src = MergeMultiLightImages(image);
    }
    rotateByExplicitWeldPoint(src, id, weldPoint);

    std::lock_guard<std::mutex> lock(m_queueMtx);
    WeldingInputTask task;
    task.image = src.clone();
    task.cameraId = id;
    m_Images.push(std::move(task));
}

void WeldingDetector::SetIuputImagessignalForWeldPoint(cv::Mat& image, int id, int weldPoint)
{
    if (image.empty()) return;

    cv::Mat src = image.clone();
    rotateByExplicitWeldPoint(src, id, weldPoint);
    std::lock_guard<std::mutex> lock(m_queueMtx);
    WeldingInputTask task;
    task.image = src;
    task.cameraId = id;
    m_Images.push(std::move(task));
}
void WeldingDetector::SetIuputImagessignal(cv::Mat& image, int id,
                                           quint64 inspectionGeneration,
                                           const QString& fixtureCode,
                                           const QString& shellCode)
{
    if (image.empty())
        return;
    else
    {
        //拼接裁剪图像
        cv::Mat src;
        //src = MergeMultiLightImages(image);
        src = image;
        // 使用 m_queueMtx 保护队列操作，防止并发写入导致数据竞争
        {
            std::lock_guard<std::mutex> lock(m_queueMtx);
            WeldingInputTask task;
            task.image = src.clone();
            task.cameraId = id;
            task.inspectionGeneration = inspectionGeneration;
            task.fixtureCode = fixtureCode;
            task.shellCode = shellCode;
            m_Images.push(std::move(task));
        }
    }
}

void WeldingDetector::ProducerFunc()
{
    while (m_running.load())
    {
        if (m_initalized)
        {
            WeldingInputTask imageData;
            bool hasData = false;
            
            // 使用 m_queueMtx 保护 m_Images 的读取操作
            {
                std::lock_guard<std::mutex> queueLock(m_queueMtx);
                if (!m_Images.empty())
                {
                    imageData = m_Images.front();
                m_Images.pop();
                    hasData = true;
                }
            }
            
            // 如果有数据，使用 mtx 保护 m_ImageTeams 的写入
            if (hasData)
            {
                std::lock_guard<std::mutex> lock(mtx);
                m_ImageTeams.push(imageData);
                cvr.notify_one();
            }
        }
        // 添加短暂休眠，避免空转消耗CPU
        std::this_thread::sleep_for(std::chrono::microseconds(100));
    }
}

void WeldingDetector::ConsumerFunc(int id)
{
    Q_UNUSED(id)
    while (m_running.load())
    {
        if (m_initalized)
        {
            WeldingInputTask task;
            {
                std::unique_lock<std::mutex> lock(mtx);
                cvr.wait(lock, [this] {
                    return !m_running.load() || !m_ImageTeams.empty();
                });

                if (!m_running.load()) {
                    break;
                }
                if (!m_ImageTeams.empty()) {
                    task = std::move(m_ImageTeams.front());
                    m_ImageTeams.pop();
                }
            }

            if (task.cameraId >= 0 && task.cameraId < WORKNUM && !task.image.empty())
            {
                WeldingResult result;
                result.id = task.cameraId;
                result.inspectionGeneration = task.inspectionGeneration;
                result.fixtureCode = task.fixtureCode;
                result.shellCode = task.shellCode;
                try {
                    // OVDetector/OpenVINO inference is serialized across both cameras.
                    std::lock_guard<std::mutex> detectorLock(m_detectorMtx);
                    Process(task.cameraId, task.image, result);
                } catch (const std::exception& e) {
                    qWarning() << "Algorithm processing exception, camera="
                               << task.cameraId + 1 << e.what();
                    result.NO_HX_NGReults = NG;
                    result.HX_NGReult = 0x01ff;
                    result.FusionMap = task.image.clone();
                    result.dst = task.image.clone();
                    result.hxResult.clear();
                    result.hxResult.push_back(HXResult{});
                } catch (...) {
                    qWarning() << "Unknown algorithm processing exception, camera="
                               << task.cameraId + 1;
                    result.NO_HX_NGReults = NG;
                    result.HX_NGReult = 0x01ff;
                    result.FusionMap = task.image.clone();
                    result.dst = task.image.clone();
                    result.hxResult.clear();
                    result.hxResult.push_back(HXResult{});
                }
                {
                    std::lock_guard<std::mutex> resultLock(m_resultMtx);
                    m_Result.push(std::move(result));
                }
            }
            printf("process count: %ld\n", count++);
        }
    }
}

int WeldingDetector::findFirstSetBitPlatformIndependent(unsigned int num) {
    if (num == 0) return -1;

    int position = 0;
    unsigned int mask = 0x80;

    // 检查每一位
    while ((num & mask) == 0) {
        mask >>= 1;  // y右移掩码
        position++;

        // 防止无限循环（对于32位整数）
        if (position >= sizeof(num) * 8) {
            return -1;
        }
    }
    return position + 1;
}

void WeldingDetector::Process(int id, const cv::Mat& src, WeldingResult& result)
{
    InThresholdParam params;
    {
        std::lock_guard<std::mutex> lock(m_paramMtx);
        params = m_inParam;
    }
    SolderJointAlgorithm::process(id, src, result, params,
                                  m_models->detector[id], m_models->segmenter[id],
                                  conf_thresh, Segconf_thresh);
}

#if 0
void WeldingDetector::LegacyProcess(int id, const cv::Mat& src, WeldingResult& result)
{
    //焊点信息默认值加载
    HXResult hxResult{};
    hxResult.center_pianyi = 0;
    hxResult.Pixeldis = 0;
    hxResult.PixelPY = 0;
    hxResult.hx_height = 0;
    hxResult.hxid = 0;
    hxResult.hx_weight = 0;
    hxResult.TBtoHX_distance = 0;
    result.hxResult.push_back(hxResult);
    QElapsedTimer totalTimer;
    totalTimer.start();
    clock_t start = clock();
    result.HX_NGReult = 0;//判定初始化，每次图进来更新NG/OK默认状态
    std::vector<BBox>bboxs;
    int ROI_X = 0, ROI_Y = 0;
    QElapsedTimer stepTimer;
    stepTimer.start();
    cv::Mat image = src.clone();
    result.t_clone = stepTimer.elapsed();
    //1.面积计算及尺寸测量
    float area, hx_w, hx_h;
    TBResult YJResult{};
    float pix2mmScaleX, pix2mmScaleY;
    result.FusionMap = image;
    stepTimer.restart();
    image = resize_with_padding(image, 480, 480, cv::Scalar(114, 114, 114));
    result.t_resize = stepTimer.elapsed();

    stepTimer.restart();
    m_models->detector[id].Infer(image, bboxs, conf_thresh, nms_thresh, 300);
    result.t_infer = stepTimer.elapsed();
    result.inferTime = result.t_infer;
    qDebug() << "[Infer]" << "cam=" << (id + 1) << "ms=" << result.inferTime;
    if (id == 0)
    {
        pix2mmScaleX = InParam.pix2mmScale_F_x;
        pix2mmScaleY = InParam.pix2mmScale_F_y;
    }
    else
    {
        pix2mmScaleX = InParam.pix2mmScale_Z_x;
        pix2mmScaleY = InParam.pix2mmScale_Z_y;
    }
    //添加处理
    if (bboxs.size() == 0)
    {
        result.HX_NGReult = result.HX_NGReult | 0xFF;
    }
    QElapsedTimer bboxTimer;
    bboxTimer.start();
    qint64 roiMs = 0;
    qint64 imwriteMs = 0;
    qint64 resizePlusMs = 0;
    qint64 segMs = 0;
    for (int i = 0; i < bboxs.size(); i++)
    {
        if (bboxs[i].label == 0)//焊锡处理
        {
            result.hxResult.clear();
            float bbox_x1 = bboxs[i].x1;
            float bbox_y1 = bboxs[i].y1;
            float bbox_x2 = bboxs[i].x2;
            float bbox_y2 = bboxs[i].y2;
            hx_w = bbox_x2 - bbox_x1;
            hx_h = bbox_y2 - bbox_y1;
            //输出结果
            hxResult.hxid = i;
            QElapsedTimer roiTimer;
            roiTimer.start();
            cv::Mat ROIsrc = image.clone();
            //取检测框ROI区域，适当扩大范围
            //范围约束
            int EXPAN = 80;
            int roi_x = int((((bboxs[i].x1 - EXPAN) < (0)) ? 0 : (bboxs[i].x1 - EXPAN)));
            int roi_y = int((((bboxs[i].y1 - EXPAN) < (0)) ? 0 : (bboxs[i].y1 - EXPAN)));
            int roi_w = int((roi_x + hx_w + 2 * EXPAN) > (image.cols) ? (image.cols - roi_x) : (hx_w + 2 * EXPAN));
            int roi_h = int((roi_y + hx_h + 2 * EXPAN) > (image.rows) ? (image.rows - roi_y) : (hx_h + 2 * EXPAN));
            //
            ROI_X = roi_x + roi_w/2;
            ROI_Y = roi_y + roi_h/2;
            cv::Rect HXroi(roi_x, roi_y, roi_w, roi_h);
            cv::Mat HX_Crop = ROIsrc(HXroi);
            roiMs += roiTimer.elapsed();
            // QElapsedTimer writeTimer;
            // writeTimer.start();
            // cv::imwrite("HX_Crop.jpg", HX_Crop);
            // imwriteMs += writeTimer.elapsed();
            QElapsedTimer resizePlusTimer;
            resizePlusTimer.start();
            qDebug()<<"resize_with_paddingPlus开始";
            HX_Crop = resize_with_paddingPlus(HX_Crop, 480, 480, cv::Scalar(114, 114, 114));
            qDebug()<<"resize_with_paddingPlus结束";
            resizePlusMs += resizePlusTimer.elapsed();
            QElapsedTimer segTimer;
            segTimer.start();
            bool AreaFlag = getContourArea(id, HX_Crop, result);
            segMs += segTimer.elapsed();
            if (!AreaFlag)
            {
                std::cerr << "未检测到瑕疵面积! " << std::endl;
            }
            else
            {
                if (result.HX_Area != 0)
                {
                    //NG判定逻辑代码
                    if (result.HX_Area < InParam.min_area || result.HX_Area > InParam.max_area)
                    {
                        result.HX_NGReult = result.HX_NGReult | 0b10000000;//尺寸NG
                    }
                }
            }
            if (InParam.AutoBDswitch)
            {
                float fs_hx_w = hx_w/scale_grobal;
                float fs_hx_h = hx_h/scale_grobal;
                std::ostringstream ss_w,ss_h;
                ss_w << std::fixed << std::setprecision(2) << fs_hx_w;
                ss_h << std::fixed << std::setprecision(2) << fs_hx_h;
                cv::putText(image, "BDPix: X:" + ss_w.str(), cv::Point(240, 80), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(0, 0, 255), 2);
                cv::putText(image, "Y:" + ss_h.str(), cv::Point(360, 110), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(0, 0, 255), 2);
            }
            //尺寸换算
            hx_w = (hx_w * pix2mmScaleX) / scale_grobal;
            hx_h = (hx_h * pix2mmScaleY) / scale_grobal;
            //NG判定逻辑代码
            if (hx_h < InParam.min_h || hx_h > InParam.max_h)
            {
                result.HX_NGReult = result.HX_NGReult | 0b00100000;//高度NG
            }
            if (hx_w < InParam.min_w || hx_w > InParam.max_w)
            {
                result.HX_NGReult = result.HX_NGReult | 0b01000000;//宽度NG
            }
            hxResult.hx_weight = hx_w;
            hxResult.hx_height = hx_h;
            hxResult.x1 = bbox_x1;
            hxResult.y1 = bbox_y1;
            hxResult.x2 = bbox_x2;
            hxResult.y2 = bbox_y2;
            result.hxResult.push_back(hxResult);
            cv::rectangle(image, cv::Point(bbox_x1, bbox_y1), cv::Point(bbox_x2, bbox_y2),cv::Scalar(0,255,0),2);
        }
        else if (bboxs[i].label == 1)//引脚处理(后面添加引脚管理)
        {
            YJResult.x1 = bboxs[i].x1;
            YJResult.y1 = bboxs[i].y1;
            YJResult.x2 = bboxs[i].x2;
            YJResult.y2 = bboxs[i].y2;
            YJResult.YJ_H = bboxs[i].y2 - bboxs[i].y1;
            YJResult.YJ_W = bboxs[i].x2 - bboxs[i].x1;
            if (bboxs.size() == 1)
            {
                result.HX_NGReult = result.HX_NGReult | 0b00010000;//焊锡丢失问题，或者归纳为单个引脚问题
            }
            cv::rectangle(image, cv::Point(YJResult.x1, YJResult.y1), cv::Point(YJResult.x2, YJResult.y2), cv::Scalar(255, 0, 0), 2);
        }
        //else if (bboxs[i].label == 1)
        //{
        //    for (int i = 0; i < result; i++)
        //    {

            //    }
            //}
    }
    result.t_bbox = bboxTimer.elapsed();
    result.t_roi = roiMs;
    result.t_imwrite = imwriteMs;
    result.t_resizePlus = resizePlusMs;
    result.t_seg = segMs;
    if (YJResult.x2 == 0)//只检测焊锡未检测引脚
    {
        result.HX_NGReult = result.HX_NGReult | 0b00010000;
    }
    //二轮判定 焊锡中心偏移检测
    stepTimer.restart();
    for (int i = 0; i < result.hxResult.size(); i++)
    {
        float center_PYRatio, hx_center, yj_center, center_PYpixel;
        hx_center = (result.hxResult[i].x1 + result.hxResult[i].x2) / 2;
        yj_center = (YJResult.x1 + YJResult.x2) / 2;
        center_PYpixel = yj_center - hx_center;//像素距离
        center_PYRatio = (yj_center - result.hxResult[i].x1) / (result.hxResult[i].x2 - yj_center + 1e-8);
        //center_PYRatio = center_PYRatio / 2 + 1 / (2 * center_PYRatio);//对勾函数
        //center_PY = (std::abs(center_PYpixel) * pix2mmScaleX) / scale_grobal;//实际距离
        result.hxResult[i].center_pianyi = center_PYRatio;
        result.hxResult[i].PixelPY = center_PYpixel;
        if (center_PYRatio > InParam.max_PY || center_PYRatio < InParam.min_PY)
        {
            result.HX_NGReult = result.HX_NGReult | 0b00001000;
        }
        else if (center_PYRatio < 0)
        {
            result.hxResult[i].center_pianyi = 0;
            result.hxResult[i].PixelPY = 0;
        }
    }
    result.t_post1 = stepTimer.elapsed();
    //三轮判定 焊接位置不良
    stepTimer.restart();
    for (int i = 0; i < result.hxResult.size(); i++)
    {
        float dis, tbtohx_dis, tbtohx_pixeldis;
        dis = YJResult.YJ_H;
        tbtohx_pixeldis = (((dis) > (0)) ? (dis) : (0)) - 1;
        tbtohx_dis = (tbtohx_pixeldis * pix2mmScaleY) / scale_grobal;//实际距离
        result.hxResult[i].TBtoHX_distance = tbtohx_dis;
        result.hxResult[i].Pixeldis = tbtohx_pixeldis;
        if (tbtohx_dis > InParam.max_TBDis)
        {
            result.HX_NGReult = result.HX_NGReult | 0b00000100;
        }
    }
    result.t_post2 = stepTimer.elapsed();
    //四轮判定，瑕疵检测
    //黑点及露铜检测
    stepTimer.restart();
    double plus_scale = scale_grobalPlus / scale_grobal;
    if (result.LT_Flag)
    {
        if (result.LT_MaxArea > InParam.LT_MAXArea_max || result.LT_SumArea > InParam.LT_ALLArea_max)
        {
            result.HX_NGReult = result.HX_NGReult | 0b00000010;
            for (int i = 0; i < result.LTrect.size(); i++)
            {
                result.LTrect[i].x = (result.LTrect[i].x - 240) / plus_scale + ROI_X;
                result.LTrect[i].y = (result.LTrect[i].y - 240) / plus_scale + ROI_Y;
                result.LTrect[i].width = result.LTrect[i].width / plus_scale;
                result.LTrect[i].height = result.LTrect[i].height / plus_scale;
                cv::rectangle(image, result.LTrect[i], cv::Scalar(0, 255, 255), 2);
            }
        }
    }
    result.t_post3 = stepTimer.elapsed();
    if (result.HD_Flag)
    {
        if (result.HD_MaxArea > InParam.HD_MAXArea_max || result.HD_SumArea > InParam.HD_ALLArea_max)
        {
            result.HX_NGReult = result.HX_NGReult | 0b00000001;
            for (int i = 0; i < result.HDrect.size(); i++)
            {
                result.HDrect[i].x = (result.HDrect[i].x - 240) / plus_scale + ROI_X;
                result.HDrect[i].y = (result.HDrect[i].y - 240) / plus_scale + ROI_Y;
                result.HDrect[i].width = result.HDrect[i].width / plus_scale;
                result.HDrect[i].height = result.HDrect[i].height / plus_scale;
                cv::rectangle(image, result.HDrect[i], cv::Scalar(0, 125, 155), 2);
            }
        }
    }
    result.t_post4 = stepTimer.elapsed();
    //NG类型文件夹保存指定
    if (result.HX_NGReult == 0x00)
    {
        result.NO_HX_NGReults = DetectionRsult::OK;
    }
    else if (result.HX_NGReult == 0xff)
    {
        result.NO_HX_NGReults = DetectionRsult::NG;
    }
    else
    {
        int pos = findFirstSetBitPlatformIndependent(result.HX_NGReult);
        printf("First set bit position: %d\n", pos);
        result.NO_HX_NGReults = DetectionRsult(pos);
    }

    stepTimer.restart();
    if (!result.HX_NGReult)
    {
        if (bboxs.size() > 0)
        {
            for (int i = 0; i < result.hxResult.size(); i++)
            {
                int center_x = int((result.hxResult[i].x1 + result.hxResult[i].x2) / 2);
                int center_y = int((result.hxResult[i].y1 + result.hxResult[i].y2) / 2);
                int center_y_down = int(result.hxResult[i].y2 + 20);
                cv::line(image, cv::Point(center_x + result.hxResult[i].PixelPY, center_y_down), cv::Point(center_x + result.hxResult[i].PixelPY, YJResult.y1 - 20), cv::Scalar(221, 10, 221), 2);
                cv::line(image, cv::Point(result.hxResult[i].x1 - 20, YJResult.y1), cv::Point(result.hxResult[i].x2 + 20, YJResult.y1), cv::Scalar(255, 255, 0), 2);
            }
        }
    }
    else
    {
        for (int i = 0; i < result.hxResult.size(); i++)
        {
            int center_x = int((result.hxResult[i].x1 + result.hxResult[i].x2) / 2);
            int center_y = int((result.hxResult[i].y1 + result.hxResult[i].y2) / 2);
            int center_y_down = int(result.hxResult[i].y2 + 20);
            cv::line(image, cv::Point(center_x + result.hxResult[i].PixelPY, center_y_down), cv::Point(center_x + result.hxResult[i].PixelPY, YJResult.y1 - 20), cv::Scalar(221, 10, 221), 2);
            cv::line(image, cv::Point(result.hxResult[i].x1 - 20, YJResult.y1), cv::Point(result.hxResult[i].x2 + 20, YJResult.y1), cv::Scalar(255, 255, 0), 2);
        }
    }
    if (image.empty())
    {
        result.dst = src;
    }
    else
    {
        result.dst = image;
    }
    result.t_draw = stepTimer.elapsed();
    result.t_total = totalTimer.elapsed();
    clock_t end = clock();

    std::cout << "consume : " << static_cast<double>(end - start) / CLOCKS_PER_SEC << " s\n";
}

/**
 * @brief 处理图像并返回分割轮廓的面积
 * @param id 处理ID，用于选择分割检测器
 * @param image 输入图像
 * @return 分割轮廓面积（像素单位），失败返回-1
 */
bool WeldingDetector::getContourArea(int id, cv::Mat& image, WeldingResult& result) {
    //面积转换系数
    double ratio = (InParam.pix2mmScale_F_x * InParam.pix2mmScale_F_y) / (scale_grobalPlus * scale_grobalPlus);
    try {
        // 检查输入图像有效性
        if (image.empty()) {
            throw std::runtime_error("输入图像为空");
        }

        // 检查ID有效性
        if (id < 0 || id >= static_cast<int>(m_models->segmenter.size())) {
            throw std::runtime_error("无效的分割检测器ID");
        }

        // 定义分割网络输出和参数
        std::vector<Instance> Segbboxs; // 分割结果
        float conf_thresh = 0.5; // 置信度阈值，可根据需要调整
        float nms_thresh = 0.3;  // NMS阈值，可根据需要调整
        int time_out = 300;   // 超时控制
        //clock_t start = clock();
        // 调用分割网络
        m_models->segmenter[id].Infer1(image, Segbboxs, conf_thresh, nms_thresh, time_out);//容易超时检测不到
        //clock_t end = clock();

        //std::cout << "consume : " << static_cast<double>(end - start) / CLOCKS_PER_SEC << " s\n";
        // 检查是否有分割结果
        if (Segbboxs.empty()) {
            return FALSE; // 未检测到任何分割区域
        }
        //检测输出id
        for (int i = 0; i < Segbboxs.size(); i++)
        {
            if (Segbboxs[i].label == 0)//以焊锡为主体进行检测
            {
                printf("焊锡检出\n");
                cv::Mat HX_mask = cv::Mat::zeros(image.size(), CV_8UC1);
                HX_mask = Segbboxs[i].mask;
                cv::Mat binary_mask;
                cv::threshold(HX_mask, binary_mask, 0, 255, cv::THRESH_BINARY);
                // 查找轮廓
                std::vector<std::vector<cv::Point>> contours;
                cv::findContours(binary_mask, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

                if (contours.empty()) {
                    return FALSE; // 未检测到轮廓
                }
                double maxArea = 0.0;
                //小焊锡球在这里做NG判断。
                for (const auto& contour : contours) {
                    double area = cv::contourArea(contour);
                    if (maxArea < area)
                    {
                        maxArea = area;
                    }
                }
                result.HX_Area = maxArea * ratio;
            }
            else if (Segbboxs[i].label == 1)//黑点检测
            {
                printf("黑点检出\n");
                result.HD_Flag = true;
                int HDcounts = 0;
                cv::Mat HD_mask = cv::Mat::zeros(image.size(), CV_8UC1);
                HD_mask = Segbboxs[i].mask;
                cv::Mat binary_mask;
                cv::threshold(HD_mask, binary_mask, 0, 255, cv::THRESH_BINARY);
                // 查找轮廓
                std::vector<std::vector<cv::Point>> contours;
                cv::findContours(binary_mask, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

                if (contours.empty()) {
                    return FALSE; // 未检测到轮廓
                }
                double Maxarea = 0, SumArea = 0;
                for (const auto& contour : contours) {
                    result.HDrect.push_back(cv::boundingRect(contour));
                    double area = cv::contourArea(contour);
                    area = area * ratio;
                    if (area > InParam.HD_min)
                    {
                        if (Maxarea < area)
                        {
                            Maxarea = area;
                        }
                        HDcounts++;
                        SumArea += area;
                    }
                }
                result.HD_MaxArea = Maxarea;
                result.HD_SumArea = SumArea;
            }
            else if (Segbboxs[i].label == 2)
            {
                printf("漏铜检出\n");
                result.LT_Flag = true;
                int LTcounts = 0;
                cv::Mat LT_mask = cv::Mat::zeros(image.size(), CV_8UC1);
                LT_mask = Segbboxs[i].mask;
                cv::Mat binary_mask;
                cv::threshold(LT_mask, binary_mask, 0, 255, cv::THRESH_BINARY);
                // 查找轮廓
                std::vector<std::vector<cv::Point>> contours;
                cv::findContours(binary_mask, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

                if (contours.empty()) {
                    return FALSE; // 未检测到轮廓
                }
                double Maxarea = 0, SumArea = 0;
                for (const auto& contour : contours) {
                    result.LTrect.push_back(cv::boundingRect(contour));
                    double area = cv::contourArea(contour);
                    area = area * ratio;
                    if (Maxarea < area)
                    {
                        Maxarea = area;
                    }
                    LTcounts++;
                    SumArea += area;
                }
                result.LT_MaxArea = Maxarea;
                result.LT_SumArea = SumArea;
            }

        }
        return TRUE;//分割检测完成
    }
    catch (const std::exception& e) {
        std::cerr << "处理异常 (ID: " << id << "): " << e.what() << std::endl;
        return FALSE;
    }
}

#endif

void WeldingDetector::SegandObjDetec(WeldingResult& result)
{
    //确定图像文件夹路径循环读取。4个一组
    std::string folderPath = "D:/HJC20250916/1T4/image/train/xianshao/xiansao_bai/MV-L5027MG140_EF46972AAK00019/";
    // 检查文件夹是否存在
    if (!fs::exists(folderPath)) {
        std::cerr << "错误: 文件夹不存在: " << folderPath << std::endl;
    }
    //std::vector<std::vector<cv::Mat>> imageGroups = loadImageGroups(folderPath);
    // 获取所有BMP文件
    std::vector<std::string> imageFiles;
    for (const auto& entry : fs::directory_iterator(folderPath)) {
        if (entry.path().extension() == ".BMP" || entry.path().extension() == ".bmp") {
            imageFiles.push_back(entry.path().filename().string());
        }
    }

    // 按文件名排序
    std::sort(imageFiles.begin(), imageFiles.end());
    cv::Mat ResImg;
    int count_id = 1;
    std::vector<BBox>bboxs;
    for (auto& filename : imageFiles) {
        std::string fullPath = folderPath + "/" + filename;
        ResImg = cv::imread(fullPath, cv::IMREAD_COLOR);
        //ResImg = MergeMultiLightImages(group);
        //cv::imwrite(folderPath + "/res_new/" + std::to_string(count_id) + ".jpg", ResImg);
        ResImg = resize_with_padding(ResImg, 480, 480, cv::Scalar(114, 114, 114));
        //图像校准
        int index = count_id % 2;
        m_models->detector[index].Infer(ResImg, bboxs, conf_thresh, 300);
        printf("目前检测第%d张图\n", count_id);
        clock_t start = clock();
        for (int i = 0; i < bboxs.size(); i++)
        {
            if (bboxs[i].label == 1)//焊锡处理
            {
                //cv::rectangle(ResImg, cv::Point(bboxs[i].x1, bboxs[i].y1), cv::Point(bboxs[i].x2, bboxs[i].y2), cv::Scalar(255, 0, 0), 2);
                float hx_w = bboxs[i].x2 - bboxs[i].x1;
                float hx_h = bboxs[i].y2 - bboxs[i].y1;
                cv::Mat ROIsrc = ResImg.clone();
                //取检测框ROI区域，适当扩大范围
                //范围约束
                int EXPAN = 8;
                int roi_x = int((((bboxs[i].x1 - EXPAN) < (0)) ? 0 : (bboxs[i].x1 - EXPAN)));
                int roi_y = int((((bboxs[i].y1) < (0)) ? 0 : (bboxs[i].y1)));
                int roi_w = int((roi_x + hx_w + 2 * EXPAN) > (ResImg.cols) ? (ResImg.cols - roi_x) : (hx_w + 2 * EXPAN));
                int roi_h = int((roi_y + hx_h) > (ResImg.rows) ? (ResImg.rows - roi_y) : (hx_h));
                //
                cv::Rect HXroi(roi_x, roi_y, roi_w, roi_h);
                //cv::rectangle(ROIsrc, cv::Point(bboxs[i].x1, bboxs[i].y1), cv::Point(bboxs[i].x2, bboxs[i].y2), cv::Scalar(0, 255, 0), 2);
                cv::Mat HX_Crop = ROIsrc(HXroi);
                //HX_Crop = resize_with_paddingPlus(HX_Crop, 480, 480, cv::Scalar(114, 114, 114));
                //bool AreaFlag = getContourArea(index, HX_Crop, result);
                //if (!AreaFlag)
                //{
                //    std::cerr << "未检测到瑕疵面积! "<< std::endl;
                //}
                cv::imwrite(folderPath + "/res_DR/" + std::to_string(count_id) + "-" + std::to_string(i) + ".bmp", HX_Crop);
            }
            if (bboxs[i].label == 2)//黑点处理
            {
                //cv::rectangle(ResImg, cv::Point(bboxs[i].x1, bboxs[i].y1), cv::Point(bboxs[i].x2, bboxs[i].y2), cv::Scalar(255, 0, 0), 2);
                float hx_w = bboxs[i].x2 - bboxs[i].x1;
                float hx_h = bboxs[i].y2 - bboxs[i].y1;
                cv::Mat ROIsrc = ResImg.clone();
                //取检测框ROI区域，适当扩大范围
                //范围约束
                int EXPAN = 80;
                int roi_x = int((((bboxs[i].x1 - EXPAN) < (0)) ? 0 : (bboxs[i].x1 - EXPAN)));
                int roi_y = int((((bboxs[i].y1 - EXPAN) < (0)) ? 0 : (bboxs[i].y1 - EXPAN)));
                int roi_w = int((roi_x + hx_w + 2 * EXPAN) > (ResImg.cols) ? (ResImg.cols - roi_x) : (hx_w + 2 * EXPAN));
                int roi_h = int((roi_y + hx_h + 2 * EXPAN) > (ResImg.rows) ? (ResImg.rows - roi_y) : (hx_h + 2 * EXPAN));
                //
                cv::Rect HXroi(roi_x, roi_y, roi_w, roi_h);
                cv::Mat HX_Crop = ROIsrc(HXroi);
                //HX_Crop = resize_with_paddingPlus(HX_Crop, 480, 480, cv::Scalar(114, 114, 114));
                //bool AreaFlag = getContourArea(index, HX_Crop, result);
                //if (!AreaFlag)
                //{
                //    std::cerr << "未检测到瑕疵面积! " << std::endl;
                //}
                cv::imwrite(folderPath + "/res_HD/" + std::to_string(count_id) + "-" + std::to_string(i) + ".jpg", HX_Crop);
            }
            if (bboxs[i].label == 3)//露铜处理
            {
                //cv::rectangle(ResImg, cv::Point(bboxs[i].x1, bboxs[i].y1), cv::Point(bboxs[i].x2, bboxs[i].y2), cv::Scalar(255, 0, 0), 2);
                float hx_w = bboxs[i].x2 - bboxs[i].x1;
                float hx_h = bboxs[i].y2 - bboxs[i].y1;
                cv::Mat ROIsrc = ResImg.clone();
                //取检测框ROI区域，适当扩大范围
                //范围约束
                int EXPAN = 80;
                int roi_x = int((((bboxs[i].x1 - EXPAN) < (0)) ? 0 : (bboxs[i].x1 - EXPAN)));
                int roi_y = int((((bboxs[i].y1 - EXPAN) < (0)) ? 0 : (bboxs[i].y1 - EXPAN)));
                int roi_w = int((roi_x + hx_w + 2 * EXPAN) > (ResImg.cols) ? (ResImg.cols - roi_x) : (hx_w + 2 * EXPAN));
                int roi_h = int((roi_y + hx_h + 2 * EXPAN) > (ResImg.rows) ? (ResImg.rows - roi_y) : (hx_h + 2 * EXPAN));
                //
                cv::Rect HXroi(roi_x, roi_y, roi_w, roi_h);
                cv::Mat HX_Crop = ROIsrc(HXroi);
                HX_Crop = resize_with_paddingPlus(HX_Crop, 480, 480, cv::Scalar(114, 114, 114));
                //bool AreaFlag = getContourArea(index, HX_Crop, result);
                //if (!AreaFlag)
                //{
                //    std::cerr << "未检测到瑕疵面积! " << std::endl;
                //}
                cv::imwrite(folderPath + "/res_LT/" + std::to_string(count_id) + "-" + std::to_string(i) + ".jpg", HX_Crop);
            }
        }
        count_id++;
        clock_t end = clock();
        std::cout << "Area_time : " << static_cast<double>(end - start) / CLOCKS_PER_SEC << " s\n";
    }
}

std::vector<std::vector<cv::Mat>> WeldingDetector::loadImageGroups(const std::string& folderPath) {
    std::vector<std::vector<cv::Mat>> groups;

    if (!fs::exists(folderPath)) {
        return groups;
    }

    // 获取所有BMP文件
    std::vector<std::string> imageFiles;
    for (const auto& entry : fs::directory_iterator(folderPath)) {
        if (entry.path().extension() == ".bmp" || entry.path().extension() == ".BMP") {
            imageFiles.push_back(entry.path().filename().string());
        }
    }

    // 按文件名排序
    std::sort(imageFiles.begin(), imageFiles.end());

    // 分组图像
    std::vector<cv::Mat> currentGroup;
    int currentGroupId = -1;

    for (const auto& filename : imageFiles) {
        // 查找最后两个下划线的位置
        size_t lastUnderscore = filename.rfind('_');
        if (lastUnderscore == std::string::npos) continue;

        size_t secondLastUnderscore = filename.rfind('_', lastUnderscore - 1);
        if (secondLastUnderscore == std::string::npos) continue;

        //size_t dotPos = filename.rfind('.');
        //if (dotPos == std::string::npos || dotPos <= lastUnderscore) continue;

        try {
            // 提取主序号和子序号
            int mainId = std::stoi(filename.substr(secondLastUnderscore + 1, lastUnderscore - secondLastUnderscore - 1));
            //int subId = std::stoi(filename.substr(lastUnderscore + 1, dotPos - lastUnderscore - 1));

            if (mainId != currentGroupId) {
                if (!currentGroup.empty() && currentGroup.size() == 4) {
                    groups.push_back(currentGroup);
                }
                currentGroupId = mainId;
                currentGroup.clear();
            }

            std::string fullPath = folderPath + "/" + filename;
            cv::Mat image = cv::imread(fullPath, cv::IMREAD_COLOR);

            if (!image.empty()) {
                currentGroup.push_back(image);
            }
        }
        catch (...) {
            continue;
        }
    }

    // 添加最后一组
    if (!currentGroup.empty() && currentGroup.size() == 4) {
        groups.push_back(currentGroup);
    }

    return groups;
}

void WeldingDetector::Report(std::function<void(WeldingResult)> callback)
{
    while (m_running.load())
    {
        WeldingResult result;
        bool hasResult = false;
        {
            std::lock_guard<std::mutex> lock(m_resultMtx);
            if (!m_Result.empty()) {
                result = std::move(m_Result.front());
                m_Result.pop();
                hasResult = true;
            }
        }
        if (hasResult) {
            callback(std::move(result));
        } else {
            Sleep(4);
        }
    }
}
