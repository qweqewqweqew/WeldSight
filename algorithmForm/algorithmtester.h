#ifndef ALGORITHMTESTER_H
#define ALGORITHMTESTER_H

#include <QWidget>
#include <QVector>
#include <QImage>
#include <QTimer>
#include <QStringList>
#include <QElapsedTimer>
#include <QFile>
#include <opencv2/core/mat.hpp>
#include <Common/common.h>

QT_BEGIN_NAMESPACE
class QProgressDialog;
class QThread;
namespace Ui {
class AlgorithmTester;
}
QT_END_NAMESPACE

class AlgorithmWorker;

class AlgorithmTester : public QWidget
{
    Q_OBJECT
public:
    explicit AlgorithmTester(QWidget *parent = nullptr);
    ~AlgorithmTester();
    void initWindow();
    // 添加测试方法
    void startMemoryLeakTest(const QString &testDir, int intervalMs = 1000);
    void stopMemoryLeakTest();

private slots:
    void onLoadFour();
    void onLoadOne();
    void onNpBatchTest();
    void onLoopTest();
    void onAlgorithmProgress(int percent, const QString &msg);

    void onAlgorithmError(const QString &err);
    void onTestTimer();

    void on_checkBox_toggled(bool checked);

public slots:
    void onAlgorithmFinished(const WeldingResult &records);
private:
    void runAlgorithm();
    void runAlgorithmOne();
    void clearImages();
    void setupInputImage(int index, const QString &path);

    Ui::AlgorithmTester *ui;

    std::vector<cv::Mat> m_inputImages;
    QStringList m_currentTestImages; // 保存当前处理的图像路径用于验证
    bool m_isFourMode = true;

    AlgorithmWorker *m_worker = nullptr;
    QThread *m_workerThread = nullptr;
    QProgressDialog *m_progressDlg = nullptr;

    // 添加测试相关成员变量
    QTimer *m_testTimer = nullptr;
    QString m_testDir;
    int m_testCount = 0;

    QString m_lastDir; // 添加成员变量记录上次目录

    // 循环测试相关
    cv::Mat m_loopNImage;  // 预加载的 N 端融合图像
    cv::Mat m_loopPImage;  // 预加载的 P 端融合图像
    int m_loopTotal = 0;                 // 总循环次数
    int m_loopCurrent = 0;               // 当前循环计数
    int m_loopResultCount = 0;           // 每轮结果计数
    int m_loopExpectCount = 2;           // 每轮期望结果数（1=单路，2=双路）
    int m_loopMode = 0;                  // 0=双路, 1=仅N端, 2=仅P端
    bool m_isLoopTesting = false;        // 是否正在循环测试
    QElapsedTimer m_loopTimer;           // 每轮计时
    QFile* m_loopLogFile = nullptr;      // 循环测试日志文件

public slots:
    void onAlgParamChanged(const InThresholdParam &param);
private:
    InThresholdParam m_currentAlgParam;

    QElapsedTimer m_Elapsetimer;
};
#endif
