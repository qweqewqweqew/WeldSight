#ifndef IOACTIONFORM_H
#define IOACTIONFORM_H

#include <QWidget>
#include <QTimer>
#include <QMutex>
#include <QList>
#include <QThreadPool>
#include <array>
#include <atomic>
#include <Common/common.h>

namespace Ui {
class ioActionForm;
}

// 防抖过滤器类
class DebounceFilter {
private:
    bool m_state = true;  // 默认高电平
    int m_counter = 0;
    const int m_threshold = 2;  // 连续3次相同才确认

public:
    bool filter(bool rawState) {
        if (rawState != m_state) {
            m_counter++;
            if (m_counter >= m_threshold) {
                m_state = rawState;
                m_counter = 0;
                return true;  // 状态确实变化了
            }
        } else {
            m_counter = 0;
        }
        return false;  // 状态未变或抖动中
    }

    bool currentState() const { return m_state; }
    void reset() { m_state = true; m_counter = 0; }
};

class ioActionForm : public QWidget
{
    Q_OBJECT

public:
    explicit ioActionForm(QWidget *parent = nullptr);
    ~ioActionForm();

    bool initPLC();
    void closePLC();
    void populateCOMPorts();

    // 添加配置读取和保存函数
    void readPLCConfig();
    void savePLCConfig();

signals:
    // 添加清除数据信号
    void clearDataRequested();
    void camera1Ready();     // 相机1采集完成
    void camera2Ready();     // 相机2采集完成
    void cameraSyncReady();  // 两个相机都采集完成

private slots:

    // 添加COM端口改变的处理
    void onComPortChanged(int index);
    void on_Bt_get_clicked();

    void on_Bt_start_clicked();

public slots:
    void slotFrameInfo(const CFrameResult& data);
    void slotFixtureCodeReceived();
    void slotShellCodeReceived();

private slots:
    void onDOIndexChanged(int index);
    void onTimeout();
    void on_Bt_clear_clicked();
    void on_Bt_save_clicked();

    // DI轮询处理
    void pollDIInputs();
    // DI信号处理
    void handleDITrigger(int diPort, bool state);

    void on_Bt_reset_sync_clicked();

    void on_Bt_open_clicked();

    void on_Bt_refresh_clicked();


    void on_checkBox_auto_poll_stateChanged(int arg1);

private:
    Ui::ioActionForm *ui;
    QString m_savedComPort;
    QString m_outputType;
    QList<int> m_do;

    // 更新UI状态的方法
    void updateCameraStatusDisplay();
    void updateSyncStatusDisplay();
private:


    QTimer *m_timer;
    QTimer *m_diPollTimer;      // DI轮询定时器
    bool m_isRunning = false;
    bool m_isOpen = false;

    // DI状态管理
    DebounceFilter m_diFilters[8];      // DI1-DI8的防抖过滤器
    bool m_lastDIStates[8];              // 上一次的DI状态
    bool m_camera1Ready = false;         // 相机1就绪标志
    bool m_camera2Ready = false;         // 相机2就绪标志
    bool m_syncReady = false;
    QMutex m_plcMutex;                   // PCI访问互斥锁
    QThreadPool m_plcTaskPool;
    std::atomic_bool m_diReadPending{false};

    // 私有辅助函数
    void initDIDisplay();
    void updateDIDisplay(int port, bool state);
    void processDIInputs(const std::array<int, 8>& rawStates);
    void queueScannerAckPulse(int point, int durationMs, const QString& codeName);
    void setupDIHandlers();
    void initializeDOChannels();
    void clearDetectionOutputs();
};

#endif // IOACTIONFORM_H
