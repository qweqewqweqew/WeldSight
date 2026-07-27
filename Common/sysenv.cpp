// sysenv.cpp
#include "SysEnv.h"
#include <QStandardPaths>
#include <QDir>
#include <QFileInfo>
#include <QHostAddress>
#include <QCoreApplication>

SysEnv& SysEnv::instance()
{
    static SysEnv env;
    return env;
}

void SysEnv::setConfigInAppDir(const QString &relativePath)
{
    QString configDir = QCoreApplication::applicationDirPath();
    QString fullPath = configDir + "/" + relativePath;

    // 确保目录存在
    QFileInfo fileInfo(fullPath);
    QDir dir = fileInfo.dir();
    if (!dir.exists()) {
        dir.mkpath(".");
    }

    setIniPath(fullPath);
}

void SysEnv::setIniPath(const QString &absoluteFilePath)
{
    if (m_iniPath == absoluteFilePath) {
        return; // 路径未变化
    }

    // 确保目录存在
    QFileInfo fileInfo(absoluteFilePath);
    QDir dir = fileInfo.dir();
    if (!dir.exists()) {
        dir.mkpath(".");
    }

    m_iniPath = absoluteFilePath;

    // 重新初始化 QSettings
    if (m_ini) {
        m_ini->sync();
        delete m_ini;
    }

    m_ini = new QSettings(m_iniPath, QSettings::IniFormat, this);
    load(); // 重新加载配置
}

SysEnv::SysEnv(QObject* parent)
    : QObject(parent)
{
    // 使用默认路径初始化 QSettings
    m_ini = new QSettings(QSettings::IniFormat,
                          QSettings::UserScope,
                          "MyCompany", "MyApp", this);
    load(); // 启动即加载
}

/* ---------- 加载 ---------- */
void SysEnv::load()
{
    if (!m_ini) {
        return;
    }

    /* Persistent */
    m_cfg.serverIP   = m_ini->value("Net/ServerIP", DEFAULT_SERVER_IP).toString();
    m_cfg.serverPort = m_ini->value("Net/ServerPort", DEFAULT_SERVER_PORT).toInt();
    m_cfg.theme      = m_ini->value("UI/Theme", DEFAULT_THEME).toString();
    m_cfg.productMode = normalizeProductMode(
        m_ini->value("Product/Mode", DEFAULT_PRODUCT_MODE).toString());
    m_cfg.imageSavePath = m_ini->value("Save/ImagePath", DEFAULT_IMAGE_SAVE_PATH).toString();
    m_cfg.imageSaveDays = m_ini->value("Save/ImageSaveDays", DEFAULT_IMAGE_SAVE_DAYS).toInt();
    m_cfg.autoStart     = m_ini->value("AutoStart/Enabled", DEFAULT_AUTO_START).toBool();
    m_cfg.classifyStr.clear();
    m_cfg.fixtureCode.clear();
    m_cfg.shellCode.clear();

    // 新增：加载相机保存设置
    for (int i = 0; i < 6; i++) {
        QString group = QString("CameraSave/Camera%1").arg(i + 1);
        m_cfg.cameraSaveSettings[i].saveSource = m_ini->value(group + "/SaveSource", false).toBool();
        m_cfg.cameraSaveSettings[i].saveComposite = m_ini->value(group + "/SaveComposite", false).toBool();
        m_cfg.cameraSaveSettings[i].saveOK = m_ini->value(group + "/SaveOK", false).toBool();
        m_cfg.cameraSaveSettings[i].saveNG = m_ini->value(group + "/SaveNG", false).toBool();
    }

    // 验证加载的配置
    if (!isConfigValid()) {
        // 重置为默认值
        m_cfg.serverIP = DEFAULT_SERVER_IP;
        m_cfg.serverPort = DEFAULT_SERVER_PORT;
        m_cfg.theme = DEFAULT_THEME;
        m_cfg.productMode = DEFAULT_PRODUCT_MODE;
        m_cfg.imageSavePath = DEFAULT_IMAGE_SAVE_PATH;
        m_cfg.imageSaveDays = DEFAULT_IMAGE_SAVE_DAYS;
        m_cfg.autoStart = DEFAULT_AUTO_START;

        // 重置相机设置
        for (int i = 0; i < 6; i++) {
            m_cfg.cameraSaveSettings[i] = ImageSaveSettings();
        }
    }
}

/* ---------- 保存 ---------- */
void SysEnv::save()
{
    if (!m_ini) {
        return;
    }

    // 验证配置 before saving
    if (!isConfigValid()) {
        return;
    }

    /* Persistent */
    m_ini->setValue("Net/ServerIP", m_cfg.serverIP);
    m_ini->setValue("Net/ServerPort", m_cfg.serverPort);
    m_ini->setValue("UI/Theme", m_cfg.theme);
    m_ini->setValue("Product/Mode", m_cfg.productMode);
    m_ini->setValue("Save/ImagePath", m_cfg.imageSavePath);
    m_ini->setValue("Save/ImageSaveDays", m_cfg.imageSaveDays);
    m_ini->setValue("AutoStart/Enabled", m_cfg.autoStart);
    m_ini->remove("Data/ClassifyStr");

    // 新增：保存相机保存设置
    for (int i = 0; i < 6; i++) {
        QString group = QString("CameraSave/Camera%1").arg(i + 1);
        m_ini->setValue(group + "/SaveSource", m_cfg.cameraSaveSettings[i].saveSource);
        m_ini->setValue(group + "/SaveComposite", m_cfg.cameraSaveSettings[i].saveComposite);
        m_ini->setValue(group + "/SaveOK", m_cfg.cameraSaveSettings[i].saveOK);
        m_ini->setValue(group + "/SaveNG", m_cfg.cameraSaveSettings[i].saveNG);
    }

    m_ini->sync(); // 立即写盘
    emit cfgChanged();
}

void SysEnv::setRunning(bool on)
{
    {
        QMutexLocker locker(&m_runtimeMutex);
        if (m_rt.running == on) {
            return;
        }
        m_rt.running = on;
    }
    emit runningChanged(on);
}

bool SysEnv::isRunning() const
{
    QMutexLocker locker(&m_runtimeMutex);
    return m_rt.running;
}

quint8 SysEnv::initMask() const
{
    QMutexLocker locker(&m_runtimeMutex);
    return m_rt.initMask;
}

bool SysEnv::isModuleOk(Module module) const
{
    QMutexLocker locker(&m_runtimeMutex);
    return (m_rt.initMask & module) == 0;
}

QString SysEnv::getImageSavePath() const
{
    if (m_cfg.imageSavePath.isEmpty()) {
        // 如果未设置，返回默认路径（应用程序目录下的capture_root）
        return QCoreApplication::applicationDirPath() + "/capture_root";
    }
    return m_cfg.imageSavePath;
}

void SysEnv::setImageSavePath(const QString &path)
{
    if (m_cfg.imageSavePath == path) {
        return;
    }

    m_cfg.imageSavePath = path;
    save();
}

// 新增：获取图像保存天数
int SysEnv::getImageSaveDays() const
{
    return m_cfg.imageSaveDays;
}

// 新增：设置图像保存天数
void SysEnv::setImageSaveDays(int days)
{
    if (m_cfg.imageSaveDays == days) {
        return;
    }

    m_cfg.imageSaveDays = days;
    save();
}

// 新增：获取自启动设置
bool SysEnv::getAutoStart() const
{
    return m_cfg.autoStart;
}

// 新增：设置自启动
void SysEnv::setAutoStart(bool enable)
{
    if (m_cfg.autoStart == enable) {
        return;
    }

    m_cfg.autoStart = enable;
    save();  // 自动保存到配置文件
}

// 分类标识（二维码）
QString SysEnv::getClassifyStr() const
{
    return m_cfg.classifyStr;
}

void SysEnv::setClassifyStr(const QString& str)
{
    if (m_cfg.classifyStr == str) {
        return;
    }

    m_cfg.classifyStr = str;
    emit classifyStrChanged(str);
}

void SysEnv::setScannerCodes(const QString& fixtureCode, const QString& shellCode)
{
    m_cfg.fixtureCode = fixtureCode;
    m_cfg.shellCode = shellCode;
    m_cfg.classifyStr = shellCode.isEmpty() ? fixtureCode : shellCode;
    emit classifyStrChanged(m_cfg.classifyStr);
    emit scannerCodesChanged(m_cfg.fixtureCode, m_cfg.shellCode);
}

QString SysEnv::getFixtureCode() const
{
    return m_cfg.fixtureCode;
}

QString SysEnv::getShellCode() const
{
    return m_cfg.shellCode;
}

void SysEnv::clearScannerCodes()
{
    m_cfg.classifyStr.clear();
    m_cfg.fixtureCode.clear();
    m_cfg.shellCode.clear();
    emit classifyStrChanged(QString());
    emit scannerCodesChanged(QString(), QString());
    emit scannerCodesCleared();
}

QStringList SysEnv::productModes()
{
    return {QStringLiteral("Z19"), QStringLiteral("U07"), QStringLiteral("E16")};
}

QString SysEnv::normalizeProductMode(const QString& mode)
{
    const QString normalized = mode.trimmed().toUpper();
    return productModes().contains(normalized) ? normalized : DEFAULT_PRODUCT_MODE;
}

QString SysEnv::getProductMode() const
{
    return m_cfg.productMode;
}

void SysEnv::setProductMode(const QString& mode)
{
    const QString normalized = normalizeProductMode(mode);
    if (m_cfg.productMode == normalized) {
        return;
    }

    m_cfg.productMode = normalized;
    save();
    emit productModeChanged(normalized);
}

int SysEnv::productNEnd() const
{
    return m_cfg.productMode == QStringLiteral("Z19") ? 8 : 10;
}

int SysEnv::productPStart() const
{
    return m_cfg.productMode == QStringLiteral("Z19") ? 16 : 20;
}

int SysEnv::productPEnd() const
{
    return m_cfg.productMode == QStringLiteral("Z19") ? 9 : 11;
}

int SysEnv::productTotalPoints() const
{
    return productNEnd() + (productPStart() - productPEnd() + 1);
}

bool SysEnv::productUsesSpecialRotation() const
{
    return m_cfg.productMode == QStringLiteral("E16");
}

void SysEnv::publishWeldPointResetMessage(const QString& message, bool warning)
{
    emit weldPointResetMessageChanged(message, warning);
}

void SysEnv::publishWeldPointCountChanged(int nWeldPoint, int pWeldPoint,
                                          const QString& message, bool warning)
{
    emit weldPointCountChanged(nWeldPoint, pWeldPoint, message, warning);
}

void SysEnv::setBit(quint8 bit, bool bad)
{
    quint8 current = 0;
    {
        QMutexLocker locker(&m_runtimeMutex);
        const quint8 old = m_rt.initMask;
        if (bad) {
            m_rt.initMask |= bit;
        } else {
            m_rt.initMask &= ~bit;
        }
        if (old == m_rt.initMask) {
            return;
        }
        current = m_rt.initMask;
    }
    emit initDone(current);
}

bool SysEnv::isConfigValid() const
{
    // 验证服务器IP格式
    QHostAddress address;
    if (!address.setAddress(m_cfg.serverIP)) {
        return false;
    }

    // 验证端口范围
    if (m_cfg.serverPort < 1 || m_cfg.serverPort > 65535) {
        return false;
    }

    // 验证主题
    if (m_cfg.theme.isEmpty() ||
        (m_cfg.theme != "Light" && m_cfg.theme != "Dark")) {
        return false;
    }

    // 验证图像保存天数范围
    if (m_cfg.imageSaveDays < 1 || m_cfg.imageSaveDays > 3650) {
        return false;
    }

    // 验证图像保存路径（如果设置了自定义路径）
    if (!m_cfg.imageSavePath.isEmpty()) {
        QFileInfo pathInfo(m_cfg.imageSavePath);
        if (!pathInfo.isDir()) {
            // 如果路径不存在，尝试创建
            QDir dir;
            if (!dir.mkpath(m_cfg.imageSavePath)) {
                return false;
            }
        }
    }

    // 自启动设置是布尔值，不需要额外验证

    if (!productModes().contains(m_cfg.productMode)) {
        return false;
    }

    return true;
}

ImageSaveSettings SysEnv::getCameraSaveSettings(int cameraIndex) const
{
    if (cameraIndex >= 1 && cameraIndex <= 6) {
        return m_cfg.cameraSaveSettings[cameraIndex - 1];
    }
    return ImageSaveSettings();
}

void SysEnv::setCameraSaveSettings(int cameraIndex, const ImageSaveSettings& settings)
{
    if (cameraIndex >= 1 && cameraIndex <= 6) {
        m_cfg.cameraSaveSettings[cameraIndex - 1] = settings;
        save(); // 立即保存到配置文件
    }
}

void SysEnv::setCameraSaveSettings(int cameraIndex, bool saveSource, bool saveComposite, bool saveOK, bool saveNG)
{
    if (cameraIndex >= 1 && cameraIndex <= 6) {
        m_cfg.cameraSaveSettings[cameraIndex - 1].saveSource = saveSource;
        m_cfg.cameraSaveSettings[cameraIndex - 1].saveComposite = saveComposite;
        m_cfg.cameraSaveSettings[cameraIndex - 1].saveOK = saveOK;
        m_cfg.cameraSaveSettings[cameraIndex - 1].saveNG = saveNG;
        save(); // 立即保存到配置文件
    }
}

SaveSw SysEnv::getAllCameraSaveSettings() const
{
    SaveSw saveSw;
    for (int i = 0; i < 6; i++) {
        CameraSw camSw = m_cfg.cameraSaveSettings[i].toCameraSw();

        switch(i) {
        case 0: saveSw.m_cSave1 = camSw; break;
        case 1: saveSw.m_cSave2 = camSw; break;
        case 2: saveSw.m_cSave3 = camSw; break;
        case 3: saveSw.m_cSave4 = camSw; break;
        case 4: saveSw.m_cSave5 = camSw; break;
        case 5: saveSw.m_cSave6 = camSw; break;
        }
    }
    return saveSw;
}

void SysEnv::setAllCameraSaveSettings(const SaveSw& saveSw)
{
    // 从SaveSw结构中提取并保存设置
    CameraSw camSw[6] = {
        saveSw.m_cSave1, saveSw.m_cSave2, saveSw.m_cSave3,
        saveSw.m_cSave4, saveSw.m_cSave5, saveSw.m_cSave6
    };

    for (int i = 0; i < 6; i++) {
        m_cfg.cameraSaveSettings[i] = ImageSaveSettings::fromCameraSw(camSw[i]);
    }
    save(); // 保存到配置文件
}
