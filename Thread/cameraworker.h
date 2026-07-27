// cameraworker.h
#ifndef CAMERAWORKER_H
#define CAMERAWORKER_H

#include <QObject>
#include <queue>
#include <opencv2/core/mat.hpp>
#include <CameraApi/cameracontroller.h>

class CameraWorker : public QObject
{
    Q_OBJECT
public:
    explicit CameraWorker(const QString& cameraKey, QObject *parent = nullptr);
    ~CameraWorker();

public slots:
    void initialize();
    void start();
    void stop();

signals:
    void sendAreaCamImage(const CFrameInfo& frameInfo);
    void sendScanCamImage(const CFrameInfo& frameInfo);
    void errorOccurred(const QString& message);

private slots:
    void handleFrameReceived(const CFrameInfo& frameInfo);

private:
    CameraController* m_controller;
    QString m_cameraKey;
    bool m_isInitialized = false;
public:
    int m_nId;
    bool m_isAreaScan = false; // 区分线扫/面阵
    std::queue<cv::Mat> m_lineScanBuffer; // 线扫图像缓冲
    int m_nIndex;
    
};
#endif // CAMERAWORKER_H
