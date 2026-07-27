#include "imagesavemanager.h"
#include <QCoreApplication>
#include <QDir>
#include <QDebug>
#include <Common/SysEnv.h>

ImageSaveManager::ImageSaveManager(QObject *parent)
    : QObject(parent)
    , m_imageNamer(new ImageNamer(this))
    , m_saveForm(nullptr)
{
}

ImageSaveManager::~ImageSaveManager()
{
    delete m_imageNamer;
}

void ImageSaveManager::initialize()
{
    // 从系统环境获取保存路径，而不是使用硬编码路径
    QString basePath = SysEnv::instance().getImageSavePath();

    // 如果路径为空或不存在，使用默认路径
    if (basePath.isEmpty() || !QDir(basePath).exists()) {
        basePath = QCoreApplication::applicationDirPath() + "/capture_root";
        SysEnv::instance().setImageSavePath(basePath); // 设置默认路径
    }

    // 确保基础目录存在
    QDir dir(basePath);
    if (!dir.exists()) {
        dir.mkpath(".");
    }

    // 初始化所有2个相机
    m_imageNamer->initializeCamera(1, basePath + tr("/ccd1"));
    m_imageNamer->initializeCamera(2, basePath + tr("/ccd2"));
}

void ImageSaveManager::setSaveForm(SaveForm* saveForm)
{
    m_saveForm = saveForm;
}

bool ImageSaveManager::saveRawImage(int cameraId, const cv::Mat& image, int triggerId, int rawIndex)
{
    if (!m_saveForm) {
        return false;
    }
    // 获取该相机的保存设置
    CameraSw settings = m_saveForm->getCameraSaveSettings(cameraId);
    // 如果设置了保存原图，则保存
    if (settings.m_nSource) {
        bool success = m_imageNamer->saveRaw(cameraId, image, triggerId, rawIndex);
        if (!success) {
            qWarning() << "Failed to save raw image for camera" << cameraId;
        }
        return success;
    }
    return true;
}

bool ImageSaveManager::saveCompositeImage(int cameraId, const cv::Mat& image, int triggerId)
{
    if (!m_saveForm) {
        return false;
    }
    // 获取该相机的保存设置
    CameraSw settings = m_saveForm->getCameraSaveSettings(cameraId);
    // 如果设置了保存合成图，则保存
    if (settings.m_nComposite) {
        bool success = m_imageNamer->saveComposite(cameraId, image, triggerId);
        if (!success) {
            qWarning() << "Failed to save composite image for camera" << cameraId;
        }
        return success;
    }

    return true; // 不需要保存也返回成功
}

bool ImageSaveManager::saveResultImage(int cameraId, const WeldingResult& result, int triggerId,
                                       const QString& fileName)
{
    if (!m_saveForm) {
        return false;
    }
    if (result.dst.empty()) {
        qWarning() << "saveResultImage: img null not save!";
    }
    // 获取该相机的保存设置
    CameraSw settings = m_saveForm->getCameraSaveSettings(cameraId);
    bool success = true;

    // 根据检测结果决定保存到OK还是NG文件夹
    if (result.NO_HX_NGReults == OK) {
        // 检测通过，保存到OK文件夹
        if (settings.m_nOK) {
            success = m_imageNamer->saveResult(cameraId, result.dst, triggerId, true, OK, fileName);
            if (!success) {
                qWarning() << "Failed to save OK result image for camera" << cameraId;
            }
        }
    } else {
        // 检测未通过，保存到NG文件夹
        if (settings.m_nNG) {
            // 修改：传递具体的错误类型
            success = m_imageNamer->saveResult(cameraId, result.dst, triggerId, false,
                                               result.NO_HX_NGReults, fileName);
            if (!success) {
                qWarning() << "Failed to save NG result image for camera" << cameraId;
            }
        }
    }

    return success;
}

void ImageSaveManager::updateSavePath(const QString &newBasePath)
{
    // 确保新路径的目录存在
    QDir dir(newBasePath);
    if (!dir.exists()) {
        if (!dir.mkpath(".")) {
            qWarning() << "Failed to create new save path:" << newBasePath;
            return;
        }
    }

    // 更新所有相机的保存路径 - 修改为 N端 和 P端
    bool success1 = m_imageNamer->updateCameraPath(1, newBasePath + tr("/ccd1"));
    bool success2 = m_imageNamer->updateCameraPath(2, newBasePath + tr("/ccd2"));

    if (!success1) {
        qWarning() << "Failed to update path for N端 camera";
    }
    if (!success2) {
        qWarning() << "Failed to update path for P端 camera";
    }

    qDebug() << "All camera save paths updated to base:" << newBasePath;
}
