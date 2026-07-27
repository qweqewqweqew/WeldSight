// SysEnv.h
#ifndef SYSENV_H
#define SYSENV_H

#include "common.h"

#include <QObject>
#include <QSettings>
#include <QString>
#include <QStringList>
#include <QMutex>

// 默认配置值
const QString DEFAULT_SERVER_IP = "127.0.0.1";
const int DEFAULT_SERVER_PORT = 502;
const QString DEFAULT_THEME = "Light";
const QString DEFAULT_PRODUCT_MODE = "U07";
const QString DEFAULT_IMAGE_SAVE_PATH = ""; // 空字符串表示使用默认路径
const int DEFAULT_IMAGE_SAVE_DAYS = 7;    // 新增：默认保存7天
const bool DEFAULT_AUTO_START = false;  // 新增：默认不自启动

/* ---------- 仅运行时 ---------- */
struct RuntimeOnly {
    quint8 initMask = 0xFF;   // 初始全未知，bit=1 表示失败
    bool   running  = false;  // 用户点击"启动"后置 true
};

/* ---------- 需要持久化 ---------- */
struct Persistent {
    QString serverIP   = DEFAULT_SERVER_IP;
    int     serverPort = DEFAULT_SERVER_PORT;
    QString theme      = DEFAULT_THEME;
    QString productMode   = DEFAULT_PRODUCT_MODE;
    QString imageSavePath = DEFAULT_IMAGE_SAVE_PATH; // 新增：图像保存路径
    int     imageSaveDays = DEFAULT_IMAGE_SAVE_DAYS; // 新增：图像保存天数
    bool    autoStart     = DEFAULT_AUTO_START;  // 新增：自启动开关

    // 新增：相机保存设置数组（对应6个相机）
    ImageSaveSettings cameraSaveSettings[6];

    // 当前扫码分类标识（二维码）
    QString classifyStr;
    QString fixtureCode;
    QString shellCode;
};

class SysEnv : public QObject
{
    Q_OBJECT
public:
    static SysEnv& instance();

    enum Module { CAM_A = 0x01, CAM_B = 0x02, ALGO = 0x04, PLC = 0x08 };

    /* 访问接口 */
    RuntimeOnly& rt()   { return m_rt; }
    Persistent&  cfg()  { return m_cfg; }
    const Persistent& cfg() const { return m_cfg; }

    /* 配置管理 */
    void setConfigInAppDir(const QString& relativePath = "configs/app_config.ini");
    void setIniPath(const QString& absoluteFilePath);
    void load();
    void save();

    /* 运行状态管理 */
    void setRunning(bool on);
    bool isRunning() const;

    /* 图像保存路径管理 */
    QString getImageSavePath() const;
    void setImageSavePath(const QString& path);

    /* 图像保存天数管理 */  // 新增
    int getImageSaveDays() const;
    void setImageSaveDays(int days);

    /* 模块状态管理 */
    void setModuleOk(quint8 bit)  { setBit(bit, false); }
    void setModuleBad(quint8 bit) { setBit(bit, true);  }
    quint8 initMask() const;
    bool isModuleOk(Module module) const;

    /* 图像保存设置管理 */
    ImageSaveSettings getCameraSaveSettings(int cameraIndex) const;
    void setCameraSaveSettings(int cameraIndex, const ImageSaveSettings& settings);
    void setCameraSaveSettings(int cameraIndex, bool saveSource, bool saveComposite, bool saveOK, bool saveNG);

    /* 获取所有相机的保存设置（用于SaveSw结构） */
    SaveSw getAllCameraSaveSettings() const;
    void setAllCameraSaveSettings(const SaveSw& saveSw);

    /* 自启动管理 */
    bool getAutoStart() const;
    void setAutoStart(bool enable);

    /* 分类标识（二维码）管理 */
    QString getClassifyStr() const;
    void setClassifyStr(const QString& str);
    void setScannerCodes(const QString& fixtureCode, const QString& shellCode);
    QString getFixtureCode() const;
    QString getShellCode() const;
    void clearScannerCodes();
    QString getProductMode() const;
    void setProductMode(const QString& mode);
    int productNEnd() const;
    int productPStart() const;
    int productPEnd() const;
    int productTotalPoints() const;
    bool productUsesSpecialRotation() const;
    static QString normalizeProductMode(const QString& mode);
    static QStringList productModes();
    void publishWeldPointResetMessage(const QString& message, bool warning);
    void publishWeldPointCountChanged(int nWeldPoint, int pWeldPoint,
                                      const QString& message, bool warning);

    /* 配置验证 */
    bool isConfigValid() const;

signals:
    void cfgChanged();
    void runningChanged(bool running);
    void initDone(quint8 mask);
    void classifyStrChanged(const QString& str);
    void scannerCodesChanged(const QString& fixtureCode, const QString& shellCode);
    void scannerCodesCleared();
    void weldPointResetMessageChanged(const QString& message, bool warning);
    void weldPointCountChanged(int nWeldPoint, int pWeldPoint,
                               const QString& message, bool warning);
    void productModeChanged(const QString& mode);

private:
    explicit SysEnv(QObject* parent = nullptr);
    ~SysEnv() = default;
    SysEnv(const SysEnv&) = delete;
    SysEnv& operator=(const SysEnv&) = delete;

    void setBit(quint8 bit, bool bad);

    RuntimeOnly m_rt;
    Persistent  m_cfg;
    mutable QMutex m_runtimeMutex;
    QSettings*  m_ini;      // 使用指针以便重新初始化
    QString     m_iniPath;  // INI 文件路径
};

#endif // SYSENV_H
