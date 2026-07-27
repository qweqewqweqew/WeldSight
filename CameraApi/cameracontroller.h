#ifndef CAMERACONTROLLER_H
#define CAMERACONTROLLER_H

#include <QObject>
#include <IMVAPI/IMVApi.h>
#include <Common/common.h>

class CameraController : public QObject
{
    Q_OBJECT
public:
    explicit CameraController(QObject *parent = nullptr);
    ~CameraController() override;

    enum ETrigType
    {
        trigContinous = 0,   // 连续拉流
        trigSoftware  = 1,   // 软件触发
        trigLine      = 2,   // 外部触发
    };

    void SetCamera(const QString& strKey);
    bool open();
    bool close();
    bool startGrabbing();
    bool stopGrabbing();

    bool setExposureTime(double exposureTime);
    bool setGain(double gain);
    bool setTriggerMode(ETrigType trigType);
    bool executeSoftwareTrigger();
    bool setRoi(int offsetX, int offsetY, int width, int height);
    bool getSensorSize(int& width, int& height);

Q_SIGNALS:
    void frameReceived(const CFrameInfo& frameInfo);

private:
    static void FrameCallback(IMV_Frame* pFrame, void* pUser);
    void processFrame(IMV_Frame* frame);

    QString m_currentCameraKey;
    IMV_HANDLE m_devHandle = nullptr;
    bool m_isGrabbing = false;
    int m_roiOffsetX = 0;
    int m_roiOffsetY = 0;
    int m_roiWidth = 0;
    int m_roiHeight = 0;
};

#endif // CAMERACONTROLLER_H
