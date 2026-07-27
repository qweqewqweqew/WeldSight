// cameraworker.cpp
#include "cameraworker.h"
#include <QDebug>

CameraWorker::CameraWorker(const QString& cameraKey, QObject *parent)
    : QObject(parent), m_cameraKey(cameraKey)
{
    m_nIndex = 1;
    m_nId = 1;
    m_controller = new CameraController(this);
    m_controller->SetCamera(m_cameraKey);
    connect(m_controller, &CameraController::frameReceived, this, &CameraWorker::handleFrameReceived);
}

CameraWorker::~CameraWorker()
{
    m_controller->close();
    m_isInitialized = false;
}

void CameraWorker::initialize()
{
    if (!m_controller->open()) {
        emit errorOccurred(tr("Failed to open camera: %1").arg(m_cameraKey));
        return;
    }
    m_isInitialized = true;
    if (!m_isInitialized) {
        emit errorOccurred(tr("Camera not initialized: %1").arg(m_cameraKey));
        return;
    }
}

void CameraWorker::start()
{
    if (!m_controller->startGrabbing()) {
        emit errorOccurred(tr("Failed to start grabbing: %1").arg(m_cameraKey));
    }
}

void CameraWorker::stop()
{
    m_controller->stopGrabbing();

}

void CameraWorker::handleFrameReceived(const CFrameInfo& frameInfo)
{
    if (m_isAreaScan) {
        // 面阵相机直接显示
        emit sendAreaCamImage(frameInfo);





    } else {
        // 线扫相机提交给木材处理器
        emit sendScanCamImage(frameInfo);
    }
}
