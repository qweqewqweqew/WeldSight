// camerathread.h
#ifndef CAMERATHREAD_H
#define CAMERATHREAD_H

#include <QObject>
#include <QThread>
#include <Common/common.h>
#include "Thread/cameraworker.h"

class CameraThread : public QObject
{
    Q_OBJECT
public:
    explicit CameraThread(const QString& cameraKey, QObject *parent = nullptr);
    ~CameraThread();

    void initialize();
    void start();
    void stop();
    QString cameraKey() const { return m_cameraKey; }

signals:
    void sigInitialize();
    void sigStart();
    void sigStop();
    void sendAreaCamImage(const CFrameInfo& frameInfo);
    void sendScanCamImage(const CFrameInfo& frameInfo);
    void errorOccurred(const QString& message);

public:
    QThread m_workThread;
    CameraWorker* m_worker;
    QString m_cameraKey;
};
#endif // CAMERATHREAD_H
