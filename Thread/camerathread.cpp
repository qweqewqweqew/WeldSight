// camerathread.cpp
#include "camerathread.h"

CameraThread::CameraThread(const QString& cameraKey, QObject *parent)
    : QObject(parent), m_cameraKey(cameraKey)
{
    m_worker = new CameraWorker(m_cameraKey);
    m_worker->moveToThread(&m_workThread);

    connect(this, &CameraThread::sigInitialize, m_worker, &CameraWorker::initialize);
    connect(this, &CameraThread::sigStart, m_worker, &CameraWorker::start);
    connect(this, &CameraThread::sigStop, m_worker, &CameraWorker::stop);
    connect(m_worker, &CameraWorker::sendAreaCamImage, this, &CameraThread::sendAreaCamImage);
    connect(m_worker, &CameraWorker::sendScanCamImage, this, &CameraThread::sendScanCamImage);
    connect(m_worker, &CameraWorker::errorOccurred, this, &CameraThread::errorOccurred);
    connect(&m_workThread, &QThread::finished, m_worker, &QObject::deleteLater);

    m_workThread.start();
}

CameraThread::~CameraThread()
{
    m_workThread.quit();
    m_workThread.wait();
}

void CameraThread::initialize() { emit sigInitialize(); }
void CameraThread::start() { emit sigStart(); }
void CameraThread::stop() { emit sigStop(); }
