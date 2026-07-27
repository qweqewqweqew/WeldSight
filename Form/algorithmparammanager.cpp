#include "algorithmparammanager.h"
#include <QSettings>
#include <QTextCodec>
#include <QDebug>

AlgorithmParamManager* AlgorithmParamManager::m_instance = nullptr;
QMutex AlgorithmParamManager::m_mutex;

AlgorithmParamManager* AlgorithmParamManager::instance()
{
    if (m_instance == nullptr) {
        m_instance = new AlgorithmParamManager();
    }
    return m_instance;
}

AlgorithmParamManager::AlgorithmParamManager(QObject *parent)
    : QObject(parent)
{
    readConfig();
}

InThresholdParam AlgorithmParamManager::getAlgParam() const
{
    return m_algParam;
}

void AlgorithmParamManager::setAlgParam(const InThresholdParam &param)
{
    m_algParam = param;
    emit algParamChanged(m_algParam);
}

void AlgorithmParamManager::readConfig()
{
    QString Path = "configs/configs.ini";
    QSettings iniRead(Path, QSettings::IniFormat);
    iniRead.setIniCodec(QTextCodec::codecForName("UTF_8"));

    // 读取尺寸阈值
    m_algParam.min_w = iniRead.value("Algorithm/min_w", 5.0).toFloat();
    m_algParam.min_h = iniRead.value("Algorithm/min_h", 5.0).toFloat();
    m_algParam.max_w = iniRead.value("Algorithm/max_w", 15.0).toFloat();
    m_algParam.max_h = iniRead.value("Algorithm/max_h", 15.0).toFloat();

    // 读取面积阈值
    m_algParam.min_area = iniRead.value("Algorithm/min_area", 18.0).toFloat();
    m_algParam.max_area = iniRead.value("Algorithm/max_area", 80.0).toFloat();

    // 读取铜板距离阈值
    m_algParam.max_TBDis = iniRead.value("Algorithm/max_TBDis", 3.0).toFloat();
    m_algParam.min_TBDis = iniRead.value("Algorithm/min_TBDis", 0.01).toFloat();

    // 读取焊锡偏离程度
    m_algParam.min_PY = iniRead.value("Algorithm/min_PY", 0.5).toFloat();
    m_algParam.max_PY = iniRead.value("Algorithm/max_PY", 2.0).toFloat();
    m_algParam.min_radius = iniRead.value("Algorithm/min_radius", 160.0).toFloat();
    m_algParam.HX_DiffArea_NG = iniRead.value("Algorithm/HX_DiffArea_NG", 1.5).toFloat();

    // 读取相机标定
    m_algParam.pix2mmScale_Z_x = iniRead.value("Algorithm/pix2mmScale_Z_x", 0.02117).toFloat();
    m_algParam.pix2mmScale_Z_y = iniRead.value("Algorithm/pix2mmScale_Z_y", 0.02117).toFloat();
    m_algParam.pix2mmScale_F_x = iniRead.value("Algorithm/pix2mmScale_F_x", 0.02117).toFloat();
    m_algParam.pix2mmScale_F_y = iniRead.value("Algorithm/pix2mmScale_F_y", 0.02117).toFloat();

    // 读取瑕疵面积控制参数
    m_algParam.LT_ALLArea_max = iniRead.value("Algorithm/LT_ALLArea_max", 0.15).toFloat();
    m_algParam.LT_MAXArea_max = iniRead.value("Algorithm/LT_MAXArea_max", 0.2).toFloat();
    m_algParam.HD_ALLArea_max = iniRead.value("Algorithm/HD_ALLArea_max", 12.0).toFloat();
    m_algParam.HD_MAXArea_max = iniRead.value("Algorithm/HD_MAXArea_max", 8.0).toFloat();
    emit algParamChanged(m_algParam);
}

void AlgorithmParamManager::saveConfig()
{
    QString Path = "configs/configs.ini";
    QSettings iniWrite(Path, QSettings::IniFormat);
    iniWrite.setIniCodec(QTextCodec::codecForName("UTF-8"));

    // 保存尺寸阈值
    iniWrite.setValue("Algorithm/min_w", m_algParam.min_w);
    iniWrite.setValue("Algorithm/min_h", m_algParam.min_h);
    iniWrite.setValue("Algorithm/max_w", m_algParam.max_w);
    iniWrite.setValue("Algorithm/max_h", m_algParam.max_h);

    // 保存面积阈值
    iniWrite.setValue("Algorithm/min_area", m_algParam.min_area);
    iniWrite.setValue("Algorithm/max_area", m_algParam.max_area);

    // 保存铜板距离阈值
    iniWrite.setValue("Algorithm/max_TBDis", m_algParam.max_TBDis);
    iniWrite.setValue("Algorithm/min_TBDis", m_algParam.min_TBDis);

    // 保存焊锡偏离程度
    iniWrite.setValue("Algorithm/min_PY", m_algParam.min_PY);
    iniWrite.setValue("Algorithm/max_PY", m_algParam.max_PY);
    iniWrite.setValue("Algorithm/min_radius", m_algParam.min_radius);
    iniWrite.setValue("Algorithm/HX_DiffArea_NG", m_algParam.HX_DiffArea_NG);

    // 保存相机标定
    iniWrite.setValue("Algorithm/pix2mmScale_Z_x", m_algParam.pix2mmScale_Z_x);
    iniWrite.setValue("Algorithm/pix2mmScale_Z_y", m_algParam.pix2mmScale_Z_y);
    iniWrite.setValue("Algorithm/pix2mmScale_F_x", m_algParam.pix2mmScale_F_x);
    iniWrite.setValue("Algorithm/pix2mmScale_F_y", m_algParam.pix2mmScale_F_y);

    // 保存瑕疵面积控制参数
    iniWrite.setValue("Algorithm/LT_ALLArea_max", m_algParam.LT_ALLArea_max);
    iniWrite.setValue("Algorithm/LT_MAXArea_max", m_algParam.LT_MAXArea_max);
    iniWrite.setValue("Algorithm/HD_ALLArea_max", m_algParam.HD_ALLArea_max);
    iniWrite.setValue("Algorithm/HD_MAXArea_max", m_algParam.HD_MAXArea_max);

    iniWrite.sync();
}
