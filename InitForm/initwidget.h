#ifndef INITWIDGET_H
#define INITWIDGET_H
#include <QMenuBar>
#include <QMouseEvent>
#include <QWidget>
#include <QActionGroup>

class ScannerHook;
class QThread;
class QMessageBox;
class CapSenseTriForm;
class ioActionForm;
class AlgorithmTester;
class SaveForm;
class AlgConfigForm;

QT_BEGIN_NAMESPACE
namespace Ui { class InitWidget; }
QT_END_NAMESPACE

class InitWidget : public QWidget
{
    Q_OBJECT

public:
    InitWidget(QWidget *parent = nullptr);
    ~InitWidget();
    CapSenseTriForm* triForm() const { return m_triForm; }

    void initWindow();  //初始化窗口控件
    void initMenuBar(); // 初始化菜单栏
    void loadQssFile(); //加载qss

    void addWidget(QWidget *widget);
protected:
    void mousePressEvent(QMouseEvent*event);
    void mouseMoveEvent(QMouseEvent*event);
    void mouseReleaseEvent(QMouseEvent*event);
    void mouseDoubleClickEvent(QMouseEvent*event);

private:
    void windowMin();
    void windowMax();
    void windowClose();
    void updateModuleStatus(quint8 statusBits);

public slots:
    void show();
    void updateTime();
    void upSaveImg();
    void upSavePath(const QString &newPath);

private slots:
    /* 菜单栏槽函数 */
    void onIoChannel();
    void onLocalTest();
    void onAlgoParam();
    void onSaveImageCfg();
    void onCameraInfo();
    void onLiveImage();
    void onVersionInfo();
    void onVendorInfo();
    void onHistoryData();  // 添加历史数据查询槽函数
    void onProductModeSelected(QAction* action);
    void onProductModeDIRequested(const QString &mode);  // DI5-7触发的模式切换

    /* 语言切换 */
    void switchLanguage(const QString &lang);
    void retranslateMenuBar();
signals:
    void appQuit();
    void fixtureCodeReceived();
    void shellCodeReceived();
private:
    Ui::InitWidget *ui;

    bool isPressed;
    bool isMaxWin;
    QPoint curPos;
    CapSenseTriForm *m_triForm;

    QTimer *m_timer = nullptr;
    QMenuBar *m_menuBar; // 菜单栏

    QAction *m_ioAction = nullptr;
    QAction *m_localTestAction = nullptr;
    QAction *m_historyDataAction = nullptr;  // 添加历史数据菜单项
    QActionGroup *m_productModeGroup = nullptr;
    quint8 m_lastModuleStatusBits = 0;
    bool m_hasModuleStatus = false;

    ioActionForm  *m_ioForm;
    AlgorithmTester *m_algorithmTester;
    SaveForm *m_saveForm;
    AlgConfigForm *m_algConfigForm;
    ScannerHook *m_scannerHook = nullptr;  // 后台持续扫码监听
    QThread *m_scannerThread = nullptr;
    QString m_pendingFixtureCode;
    QString m_pendingShellCode;
    QMessageBox *m_scannerErrorBox = nullptr;  // 扫码错误提示弹窗

    void startScannerListening();
    void handleScannerCode(const QString& code);
    void showScannerError(const QString& message);
    void onScannerCodesCleared();
    void updateProductModeLabel(const QString& mode);
    void refreshModuleStatusText();

private:
    // 添加定时器用于定期清理
    QTimer *m_cleanupTimer = nullptr;
    void cleanupDirectoryRecursive(const QString &directoryPath, const QDateTime &cutoffTime,
                                   int &deletedCount, int &errorCount);
    void scheduleDailyCleanup(); // 安排每日清理
    bool isFileExpired(const QString &fileName, const QDateTime &cutoffTime);
private slots:
    void cleanupExpiredImages();

};
#endif // INITWIDGET_H
