#include "cameracontroller.h"
#include <Common/common.h>
#include <QDebug>

#define CHECK_IMV_RET(ret) if (ret != IMV_OK) { qWarning() << "IMV Error:" << ret; return false; }
#define CHECK_RET(expr) \
    do { \
        int ret = (expr); \
        if (ret != IMV_OK) { \
            qWarning() << "IMV Error:" << ret; \
        } \
    } while (0)

CameraController::CameraController(QObject *parent)
    : QObject{parent}
{}

CameraController::~CameraController()
{
    close();  // 确保资源释放
}

void CameraController::SetCamera(const QString &strKey)
{
    m_currentCameraKey = strKey;
}

bool CameraController::open()
{
    if (m_currentCameraKey.isEmpty() || m_devHandle)
        return false;

    QByteArray keyBytes = m_currentCameraKey.toLocal8Bit();
    IMV_HANDLE newHandle = nullptr;
    int ret = IMV_CreateHandle(&newHandle, modeByCameraKey, keyBytes.data());
    CHECK_IMV_RET(ret);

    ret = IMV_Open(newHandle);
    if (ret != IMV_OK) {
        IMV_DestroyHandle(newHandle);
        return false;
    }

    m_devHandle = newHandle;
    return true;
}

bool CameraController::close()
{
    if (!m_devHandle)
        return true;

    stopGrabbing();
    IMV_Close(m_devHandle);
    IMV_DestroyHandle(m_devHandle);
    m_devHandle = nullptr;
    return true;
}

bool CameraController::startGrabbing()
{
    if (!m_devHandle || m_isGrabbing)
        return false;

    int ret = IMV_AttachGrabbing(m_devHandle, FrameCallback, this);
    CHECK_IMV_RET(ret);

    ret = IMV_StartGrabbing(m_devHandle);
    CHECK_IMV_RET(ret);

    m_isGrabbing = true;
    return true;
}

bool CameraController::stopGrabbing()
{
    if (!m_isGrabbing || !m_devHandle)
        return true;
    int ret = IMV_StopGrabbing(m_devHandle);
    CHECK_IMV_RET(ret);

    m_isGrabbing = false;
    return true;
}

bool CameraController::setExposureTime(double exposureTime)
{
    if (exposureTime <= 0) return false;
    return (IMV_SetDoubleFeatureValue(m_devHandle, "ExposureTime", exposureTime) == IMV_OK);
}

bool CameraController::setGain(double gain)
{
    if (gain < 0) return false;
    return (IMV_SetDoubleFeatureValue(m_devHandle, "GainRaw", gain) == IMV_OK);
}

bool CameraController::setTriggerMode(ETrigType trigType)
{
    const char* mode = (trigType == trigContinous) ? "Off" : "On";
    return (IMV_SetEnumFeatureSymbol(m_devHandle, "TriggerMode", mode) == IMV_OK);
}

bool CameraController::executeSoftwareTrigger()
{
    return (IMV_ExecuteCommandFeature(m_devHandle, "TriggerSoftware") == IMV_OK);
}


bool CameraController::setRoi(int offsetX, int offsetY, int width, int height)
{
    if (!m_devHandle) return false;

    int ret = IMV_OK;
    int64_t widthValue = 0, heightValue = 0;
    int64_t offsetXValue = 0, offsetYValue = 0;
    int64_t widthMin = 0, widthMax = 0, widthInc = 0;
    int64_t heightMin = 0, heightMax = 0, heightInc = 0;
    int64_t offsetXMin = 0, offsetXMax = 0, offsetXInc = 0;
    int64_t offsetYMin = 0, offsetYMax = 0, offsetYInc = 0;

    // 获取当前参数范围和步长
    CHECK_RET(IMV_GetIntFeatureMin(m_devHandle, "Width", &widthMin));
    CHECK_RET(IMV_GetIntFeatureMax(m_devHandle, "Width", &widthMax));
    CHECK_RET(IMV_GetIntFeatureInc(m_devHandle, "Width", &widthInc));
    CHECK_RET(IMV_GetIntFeatureMin(m_devHandle, "Height", &heightMin));
    CHECK_RET(IMV_GetIntFeatureMax(m_devHandle, "Height", &heightMax));
    CHECK_RET(IMV_GetIntFeatureInc(m_devHandle, "Height", &heightInc));
    CHECK_RET(IMV_GetIntFeatureMin(m_devHandle, "OffsetX", &offsetXMin));
    CHECK_RET(IMV_GetIntFeatureMax(m_devHandle, "OffsetX", &offsetXMax));
    CHECK_RET(IMV_GetIntFeatureInc(m_devHandle, "OffsetX", &offsetXInc));
    CHECK_RET(IMV_GetIntFeatureMin(m_devHandle, "OffsetY", &offsetYMin));
    CHECK_RET(IMV_GetIntFeatureMax(m_devHandle, "OffsetY", &offsetYMax));
    CHECK_RET(IMV_GetIntFeatureInc(m_devHandle, "OffsetY", &offsetYInc));

    // 调整参数到合法值
    offsetX = std::clamp(offsetX, (int)offsetXMin, (int)offsetXMax);
    offsetY = std::clamp(offsetY, (int)offsetYMin, (int)offsetYMax);
    width = std::clamp(width, (int)widthMin, (int)widthMax);
    height = std::clamp(height, (int)heightMin, (int)heightMax);

    // 对齐到步长
    offsetX = (offsetX / offsetXInc) * offsetXInc;
    offsetY = (offsetY / offsetYInc) * offsetYInc;
    width = (width / widthInc) * widthInc;
    height = (height / heightInc) * heightInc;

    // 设置ROI参数
    CHECK_RET(IMV_SetIntFeatureValue(m_devHandle, "OffsetX", offsetX));
    CHECK_RET(IMV_SetIntFeatureValue(m_devHandle, "OffsetY", offsetY));
    CHECK_RET(IMV_SetIntFeatureValue(m_devHandle, "Width", width));
    CHECK_RET(IMV_SetIntFeatureValue(m_devHandle, "Height", height));

    // 验证设置是否成功
    CHECK_RET(IMV_GetIntFeatureValue(m_devHandle, "Width", &widthValue));
    CHECK_RET(IMV_GetIntFeatureValue(m_devHandle, "Height", &heightValue));
    CHECK_RET(IMV_GetIntFeatureValue(m_devHandle, "OffsetX", &offsetXValue));
    CHECK_RET(IMV_GetIntFeatureValue(m_devHandle, "OffsetY", &offsetYValue));

    qDebug() << "ROI set to:"
        << "X:" << offsetXValue
        << "Y:" << offsetYValue
        << "W:" << widthValue
        << "H:" << heightValue;

    m_roiOffsetX = offsetXValue;
    m_roiOffsetY = offsetYValue;
    m_roiWidth = widthValue;
    m_roiHeight = heightValue;

    return true;
}

bool CameraController::getSensorSize(int& width, int& height)
{
    if (!m_devHandle) return false;

    int64_t widthMax = 0, heightMax = 0;
    if (IMV_GetIntFeatureMax(m_devHandle, "Width", &widthMax) != IMV_OK) {
        return false;
    }
    if (IMV_GetIntFeatureMax(m_devHandle, "Height", &heightMax) != IMV_OK) {
        return false;
    }

    width = static_cast<int>(widthMax);
    height = static_cast<int>(heightMax);
    return true;
}

void CameraController::FrameCallback(IMV_Frame *pFrame, void *pUser)
{
    auto* controller = static_cast<CameraController*>(pUser);
    if (controller)
        controller->processFrame(pFrame);
}

void CameraController::processFrame(IMV_Frame* frame) {
    if (!frame || !frame->pData || frame->frameInfo.width == 0 || frame->frameInfo.height == 0) {
        qWarning() << "Invalid frame data received";
        if (frame) IMV_ReleaseFrame(m_devHandle, frame);
        return;
    }

    CFrameInfo frameInfo;
    frameInfo.cameraKey     = m_currentCameraKey;
    frameInfo.m_nWidth      = static_cast<int>(frame->frameInfo.width);
    frameInfo.m_nHeight     = static_cast<int>(frame->frameInfo.height);
    frameInfo.m_nBufferSize = static_cast<int>(frame->frameInfo.size);
    frameInfo.m_nPaddingX   = static_cast<int>(frame->frameInfo.paddingX);
    frameInfo.m_nPaddingY   = static_cast<int>(frame->frameInfo.paddingY);
    frameInfo.m_ePixelType  = frame->frameInfo.pixelFormat;
    
    // 情况1：Mono8，直接拷贝原始数据
    if (frameInfo.m_ePixelType == gvspPixelMono8)
    {
        unsigned char* pMonoData = static_cast<unsigned char*>(malloc(frameInfo.m_nBufferSize));
        if (!pMonoData) {
            qCritical() << "Failed to allocate mono buffer";
            IMV_ReleaseFrame(m_devHandle, frame);
            return;
        }
        memcpy(pMonoData, frame->pData, frameInfo.m_nBufferSize);
        frameInfo.m_pImageBuf.reset(pMonoData, free);
    }

    // 非Mono8格式需要转码为RGB8
    else
    {
        const int nDstBufferSize = frameInfo.m_nWidth * frameInfo.m_nHeight * 3;
        unsigned char* pDstData = static_cast<unsigned char*>(malloc(nDstBufferSize));
        if (!pDstData) {
            qCritical() << "Failed to allocate RGB buffer";
            IMV_ReleaseFrame(m_devHandle, frame);
            return;
        }

        IMV_PixelConvertParam stParam;
        memset(&stParam, 0, sizeof(IMV_PixelConvertParam));
        stParam.nWidth           = frameInfo.m_nWidth;
        stParam.nHeight          = frameInfo.m_nHeight;
        stParam.ePixelFormat     = frameInfo.m_ePixelType;
        stParam.pSrcData         = frame->pData;    //使用相机指针，减少一次深拷贝
        stParam.nSrcDataLen      = frameInfo.m_nBufferSize;
        stParam.eDstPixelFormat  = gvspPixelRGB8;
        stParam.eBayerDemosaic   = demosaicEdgeSensing; // 使用厂商定义的枚举值
        stParam.pDstBuf          = pDstData;
        stParam.nDstBufSize      = nDstBufferSize;

        int ret = IMV_PixelConvert(m_devHandle, &stParam);
        if (ret != IMV_OK) {
            qWarning() << "PixelConvert failed. Error code:" << ret;
            free(pDstData);
            IMV_ReleaseFrame(m_devHandle, frame);
            return;
        }

        // 更新为转换后的数据
        frameInfo.m_ePixelType  = gvspPixelRGB8;
        frameInfo.m_nBufferSize = nDstBufferSize;
        frameInfo.m_pImageBuf.reset(pDstData, free);
    }

    // 发送信号

    emit frameReceived(frameInfo);

    IMV_ReleaseFrame(m_devHandle, frame);
}
