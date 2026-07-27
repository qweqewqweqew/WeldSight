// capsensetriform.h
#ifndef CAPSENSETRIFORM_H
#define CAPSENSETRIFORM_H

#include "Thread/imagesavemanager.h"
#include <CapSenseTri/capsenseform.h>
#include <QGraphicsDropShadowEffect>
#include <QGroupBox>
#include <QWidget>
#include <QGridLayout>
#include <QThread>
#include <Common/common.h>
#include <Thread/camerathread.h>
#include <SqliteUtil/sqliteutil.h>



namespace Ui {
class CapSenseTriForm;
}

class DbInsertWorker : public QObject
{
    Q_OBJECT
public:
    explicit DbInsertWorker(const QString &connectionName, QObject *parent = nullptr);

public slots:
    void initialize();
    void insertResult(const DB_WeldingResult &result);

private:
    SqliteUtil m_sql;
    QString m_connectionName;
    bool m_initialized = false;
};

class CapSenseTriForm : public QWidget
{
    Q_OBJECT
public:
    explicit CapSenseTriForm(QWidget *parent = nullptr);
    ~CapSenseTriForm();
    bool isDetectionRunning() const { return m_isRun != 0; }

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

public:
    enum DataGroup {
        ConfigData,
        OkRateData,
        NgCountData,
        TotalData
    };

    void updateDataGroup(DataGroup group, const QList<QPair<QString, QVariant>> &data);


    // void onAlgorithmResultReady(int id, const QList<CheckResult>& results,const cv::Mat& fusionResult, const cv::Mat& detectionResult);

    CapSenseForm* getDisplay(int index) const;

    int getCameraID(const QString& cameraKey);
signals:
    void sigCamResult(const WeldingResult& results);
    void sigFramResult(const CFrameResult& results);
    void sigDbInsert(const DB_WeldingResult& result);

private:
    inline QGraphicsDropShadowEffect* createShadow();
private slots:
    void on_Bt_clear_clicked();
    void on_Bt_start_clicked();

    void on_cb_autoStart_clicked(bool checked);

public  slots:
    void handleFrame(CFrameInfo frameInfo);
    void handleError(QString message);
    void onAlgorithmResultReady(const WeldingResult& result);
    void onSysRuning(bool on);
public:

    void initWindow();
    void loadQssFile();
    void readConfig();
    void updateDynamicGroupBox(QGroupBox *groupBox, const QList<QPair<QString, QVariant>> &data);

    // 相机
    void initCameras();
    void createCameraThread(IMV_DeviceList m_deviceInfoList);
    void releaseAllCameras();
    void upSaveImgPath(const QString &newPath);

    QVector<CapSenseForm*> displays;
private:
    // 添加运行时间计时相关成员
    QTimer *m_runtimeTimer;
    qint64 m_runtimeSeconds;
    QString m_runtimeString;

    // 添加更新运行时间的函数
    void updateRuntimeDisplay();
private:
    Ui::CapSenseTriForm *ui;
    QGridLayout* gridLayout_displays;

    int m_isRun = 0;

    IMV_DeviceList m_deviceInfoList;	// 发现的相机列表 | List of cameras found

    QMap<QString, CameraThread*> cameraThreads;
    QList<CameraThread*> m_cameraThreadList; // 存储所有相机线程的列表


    qint64 m_captureTime = 0; // 抓取开始
    qint64 m_processStartTime = 0;  // 算法开始
    qint64 m_algorithmEndTime = 0;  // 算法结束

    bool enableLocalMode;

    std::vector<cv::Mat> m_vListC1;
    std::vector<cv::Mat> m_vListC2;
    QString m_cameraKey1;
    QString m_cameraKey2;
    int m_test = 0;
    QList<int> m_vImgIndexs{1, 1};
    QList<int> m_vTriggerIndexs{1, 1};


    ImageSaveManager* m_imageSaveManager;

    QVector<QVariantList> m_resultData{2};
    QVector<int> m_hxIndex = {1, 0};
    quint64 m_inspectionGeneration = 0;

    float m_ccd1_rate = 0;
    int m_ccd1_ng = 0;
    int m_ccd1_num = 0;

    float m_ccd2_rate = 0;
    int m_ccd2_ng = 0;
    int m_ccd2_num = 0;

    SqliteUtil m_sql;
    QThread* m_dbThread = nullptr;
    DbInsertWorker* m_dbWorker = nullptr;
    bool m_enableDbInsert = true;  //临时添加数据库写入
private:
    bool m_clearScannerMousePressed = false;
    void clearScannerCodesFromMouse();
    SaveForm* m_saveForm;
public:
    void setSaveForm(SaveForm* saveForm);
public slots:
    void onAlgParamChanged(const InThresholdParam &param);
private:
    InThresholdParam m_currentAlgParam;
private:
    // 添加图像计数相关成员
    int m_camera1ImageCount = 0;  // 相机1收集的图像数量
    int m_camera2ImageCount = 0;  // 相机2收集的图像数量
    bool m_waitingForTrigger1 = false; // 相机1等待触发标志
    bool m_waitingForTrigger2 = false; // 相机2等待触发标志

    // 添加处理函数声明
    void checkAndProcessCameraImages(int cameraId);
    void generateNGResult(int cameraId, const QString& reason);
    void resetCameraImageCount(int cameraId);

    // 周期同步保护计时器
    QElapsedTimer m_lastTriggerTime[2];
    // 上一帧图像到达计时器 (用于检测跨周期残余)
    QElapsedTimer m_lastFrameTimer[2];

    // 添加DI信号处理槽函数
public slots:
    void onCamera1Ready();    // 处理相机1完成信号
    void onCamera2Ready();    // 处理相机2完成信号
    void onClearData();       // 处理清除数据信号
    void onWeldPointResetMessage(const QString& message, bool warning);
    void onWeldPointCountChanged(int nWeldPoint, int pWeldPoint,
                                 const QString& message, bool warning);
    void onScannerCodesChanged(const QString& fixtureCode, const QString& shellCode);

};

#endif // CAPSENSETRIFORM_H
