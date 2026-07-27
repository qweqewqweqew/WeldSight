#include "imagenamer.h"
#include <QDateTime>
#include <QDebug>
#include <QTextCodec>
#include <opencv2/imgcodecs.hpp>

// CameraSaverWorker 实现
CameraSaverWorker::CameraSaverWorker(int cameraId, const QString &rootPath, QObject *parent)
    : QObject(parent), m_cameraId(cameraId), m_stopped(false)
{
    // 为每个相机创建独立的子目录
    m_rootDir.setPath(rootPath);
    m_rawDir = m_rootDir.filePath("raw");
    m_compDir = m_rootDir.filePath("composite");
    m_okDir = m_rootDir.filePath("ok");
    m_ngDir = m_rootDir.filePath("ng");

    // 创建目录
    m_rootDir.mkpath(".");
    m_rawDir.mkpath(".");
    m_compDir.mkpath(".");
    m_okDir.mkpath(".");
    m_ngDir.mkpath(".");

    //创建NG子目录
    QList<DetectionRsult> errorTypes = {
        HXAREA_Err, HXSIZE_H_Err, HXSIZE_W_Err,
        YJ_Err, HXPY_Err, TBDIS_Err, LT_Err, HD_Err, HXGAP_Err, NG
    };

    for (DetectionRsult errorType : errorTypes) {
        QString dirName = errorTypeToString(errorType);
        QDir subDir = m_ngDir.filePath(dirName);
        subDir.mkpath(".");
        m_ngSubDirs.insert(errorType, subDir);
    }
}

CameraSaverWorker::~CameraSaverWorker() {
    stop();
}

void CameraSaverWorker::addTask(const SaveTask &task) {
    QMutexLocker locker(&m_mutex);
    m_taskQueue.push(task);
    m_condition.wakeOne();
}

void CameraSaverWorker::stop() {
    QMutexLocker locker(&m_mutex);
    m_stopped = true;
    m_condition.wakeOne();
}

void CameraSaverWorker::updateRootPath(const QString &newRootPath)
{
    qDebug() << "CameraSaverWorker::updateRootPath" << newRootPath;
    QMutexLocker locker(&m_mutex);

    m_rootDir.setPath(newRootPath);
    m_rawDir = m_rootDir.filePath("raw");
    m_compDir = m_rootDir.filePath("composite");
    m_okDir = m_rootDir.filePath("ok");
    m_ngDir = m_rootDir.filePath("ng");

    // 创建新目录
    m_rootDir.mkpath(".");
    m_rawDir.mkpath(".");
    m_compDir.mkpath(".");
    m_okDir.mkpath(".");
    m_ngDir.mkpath(".");

    // 重新创建NG子目录
    m_ngSubDirs.clear();
    QList<DetectionRsult> errorTypes = {
        HXAREA_Err, HXSIZE_H_Err, HXSIZE_W_Err,
        YJ_Err, HXPY_Err, TBDIS_Err, LT_Err, HD_Err, HXGAP_Err, NG
    };

    for (DetectionRsult errorType : errorTypes) {
        QString dirName = errorTypeToString(errorType);
        QDir subDir = m_ngDir.filePath(dirName);
        subDir.mkpath(".");
        m_ngSubDirs.insert(errorType, subDir);
    }

    qDebug() << "Camera" << m_cameraId << "save path updated to:" << newRootPath;
}

void CameraSaverWorker::processTasks() {
    while (true) {
        QMutexLocker locker(&m_mutex);

        while (m_taskQueue.empty() && !m_stopped) {
            m_condition.wait(&m_mutex);
        }

        if (m_stopped && m_taskQueue.empty()) {
            break;
        }

        if (!m_taskQueue.empty()) {
            SaveTask task = m_taskQueue.front();
            m_taskQueue.pop();
            locker.unlock();

            bool success = false;
            QString folder;
            QString fileName;

            switch (task.type) {
            case SaveTask::RAW:
                folder = m_rawDir.absolutePath();
                fileName = makeName(task.triggerId, "R", task.rawIndex);
                success = saveMat(task.image, folder, fileName);
                break;

            case SaveTask::COMPOSITE:
                folder = m_compDir.absolutePath();
                fileName = makeName(task.triggerId, "C");
                success = saveMat(task.image, folder, fileName);
                break;

            case SaveTask::RESULT:
                if (!task.fileName.isEmpty()) {
                    folder = task.ok ? m_okDir.absolutePath()
                                     : m_ngSubDirs.value(task.errorType).absolutePath();
                    fileName = task.fileName;
                } else if (task.ok) {
                    folder = m_okDir.absolutePath();
                    fileName = makeName(task.triggerId, "OK");
                } else {
                    // 修改：根据错误类型选择NG子目录
                    folder = m_ngSubDirs.value(task.errorType).absolutePath();
                    fileName = makeNGName(task.triggerId, task.errorType);
                }
                success = saveMat(task.image, folder, fileName);
                break;
            }

            emit taskCompleted(success);
        }
    }
}

QString CameraSaverWorker::makeName(int triggerId, const QString &type, int rawIndex) const {
    QString ts = QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss_zzz");
    QString base = QString("%1_T%2_C%3").arg(ts)
                       .arg(triggerId, 3, 10, QLatin1Char('0'))
                       .arg(m_cameraId, 2, 10, QLatin1Char('0'));
    if (type == "R")
        return QString(tr("%1_raw%2.jpg")).arg(base).arg(rawIndex);
    if (type == "C")
        return base + tr("_composite.jpg");
    if (type == "OK")
        return base + tr("_ok.jpg");
    return base + "_" + type + ".jpg";
}

QString CameraSaverWorker::makeNGName(int triggerId, DetectionRsult errorType) const {
    QString ts = QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss_zzz");
    QString base = QString("%1_T%2_C%3").arg(ts)
                       .arg(triggerId, 3, 10, QLatin1Char('0'))
                       .arg(m_cameraId, 2, 10, QLatin1Char('0'));
    QString errorStr = errorTypeToString(errorType);
    return base + "_" + errorStr + ".jpg";
}

// 修改：错误类型转中文字符串函数
QString CameraSaverWorker::errorTypeToString(DetectionRsult errorType) const {
    switch (errorType) {
    case HXAREA_Err: return tr("Solder area error");
    case HXSIZE_H_Err: return tr("Solder height error");
    case HXSIZE_W_Err: return tr("Solder width error");
    case YJ_Err: return tr("Pin error");
    case HXPY_Err: return tr("Solder offset error");
    case TBDIS_Err: return tr("Copper spacing error");
    case LT_Err: return tr("Exposed copper error");
    case HD_Err: return tr("Black dot error");
    case HXGAP_Err: return tr("Solder gap error");
    case NG: return tr("Abnormal error");
    default: return tr("Unknown error");
    }
}

bool CameraSaverWorker::saveMat(const cv::Mat &img, const QString &folder, const QString &fileName) {
    if (img.empty()) {
        qWarning() << "saveMat: img null not save!";
        return false;
    }

    QString fullPath = QDir(folder).filePath(fileName);
    QTextCodec *code = QTextCodec::codecForName("GB2312");
    std::string strFileName = code->fromUnicode(fullPath).data();
    return cv::imwrite(strFileName, img);
}

// ImageNamer 实现
ImageNamer::ImageNamer(QObject *parent) : QObject(parent) {
}

ImageNamer::~ImageNamer() {
    // 停止所有工作线程
    for (auto it = m_threads.begin(); it != m_threads.end(); ++it) {
        int cameraId = it.key();
        QThread* thread = it.value();
        CameraSaverWorker* worker = m_workers.value(cameraId);

        if (worker) {
            worker->stop();
        }

        thread->quit();
        thread->wait();
        delete worker;
        delete thread;
    }
}

bool ImageNamer::initializeCamera(int cameraId, const QString &rootPath) {
    QMutexLocker locker(&m_mutex);

    if (m_workers.contains(cameraId)) {
        qWarning() << "Camera" << cameraId << "already initialized";
        return false;
    }

    // 创建工作线程
    CameraSaverWorker* worker = new CameraSaverWorker(cameraId, rootPath);
    QThread* thread = new QThread();

    worker->moveToThread(thread);
    connect(thread, &QThread::started, worker, &CameraSaverWorker::processTasks);

    thread->start();

    m_workers.insert(cameraId, worker);
    m_threads.insert(cameraId, thread);

    return true;
}

bool ImageNamer::saveRaw(int cameraId, const cv::Mat &img, int triggerId, int rawIndex) {
    QMutexLocker locker(&m_mutex);

    if (!m_workers.contains(cameraId)) {
        qWarning() << "Camera" << cameraId << "not initialized";
        return false;
    }

    if (img.empty()) {
        qWarning() << "ImageNamer::saveRaw: img null not save!";
    }
    if (rawIndex < 1 || rawIndex > 4) {
        return false;
    }

    SaveTask task;
    task.type = SaveTask::RAW;
    task.image = img.clone();
    task.triggerId = triggerId;
    task.rawIndex = rawIndex;

    m_workers.value(cameraId)->addTask(task);
    return true;
}

bool ImageNamer::saveComposite(int cameraId, const cv::Mat &img, int triggerId) {
    QMutexLocker locker(&m_mutex);

    if (!m_workers.contains(cameraId)) {
        qWarning() << "Camera" << cameraId << "not initialized";
        return false;
    }
    SaveTask task;
    task.type = SaveTask::COMPOSITE;
    task.image = img.clone();
    task.triggerId = triggerId;

    m_workers.value(cameraId)->addTask(task);
    return true;
}

// 修改：增加错误类型参数
bool ImageNamer::saveResult(int cameraId, const cv::Mat &img, int triggerId, bool ok,
                            DetectionRsult errorType, const QString& fileName) {
    QMutexLocker locker(&m_mutex);

    if (!m_workers.contains(cameraId)) {
        qWarning() << "Camera" << cameraId << "not initialized";
        return false;
    }
    SaveTask task;
    task.type = SaveTask::RESULT;
    task.image = img.clone();
    task.triggerId = triggerId;
    task.ok = ok;
    task.errorType = errorType;  //设置错误类型
    task.fileName = fileName;

    m_workers.value(cameraId)->addTask(task);
    return true;
}

bool ImageNamer::updateCameraPath(int cameraId, const QString &newRootPath)
{

    QMutexLocker locker(&m_mutex);
    if (!m_workers.contains(cameraId)) {
        qWarning() << "Camera" << cameraId << "not initialized, cannot update path";
        return false;
    }

    // 直接更新工作线程的路径（不需要停止线程）
    CameraSaverWorker* worker = m_workers.value(cameraId);
    if (worker) {
        worker->updateRootPath(newRootPath);
        return true;
    }

    return false;
}

int ImageNamer::getCameraCount() const {
    return m_workers.size();
}
