#ifndef IMAGENAMER_H
#define IMAGENAMER_H

#include <QObject>
#include <QDir>
#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <QString>
#include <queue>
#include <QMap>
#include <opencv2/core/mat.hpp>
#include "Common/common.h"

// 保存任务结构体
struct SaveTask {
    enum TaskType { RAW, COMPOSITE, RESULT };

    TaskType type;
    cv::Mat image;
    int triggerId;
    int rawIndex; // 仅对RAW类型有效
    bool ok;      // 仅对RESULT类型有效
    DetectionRsult errorType; // 新增：错误类型
    QString fileName;
};

// 每个相机的保存工作线程
class CameraSaverWorker : public QObject {
    Q_OBJECT
public:
    explicit CameraSaverWorker(int cameraId, const QString &rootPath, QObject *parent = nullptr);
    ~CameraSaverWorker();

    void addTask(const SaveTask &task);
    void stop();
    void updateRootPath(const QString &newRootPath);  //更新根路径
signals:
    void taskCompleted(bool success);

public slots:
    void processTasks();


private:
    int m_cameraId;
    QDir m_rootDir;
    QDir m_rawDir;
    QDir m_compDir;
    QDir m_okDir;
    QDir m_ngDir;

    // 新增：NG子目录映射
    QMap<DetectionRsult, QDir> m_ngSubDirs;

    QMutex m_mutex;
    QWaitCondition m_condition;
    std::queue<SaveTask> m_taskQueue;
    bool m_stopped;

    QString makeName(int triggerId, const QString &type, int rawIndex = 0) const;
    QString makeNGName(int triggerId, DetectionRsult errorType) const;  // 新增：NG文件命名
    QString errorTypeToString(DetectionRsult errorType) const;  // 新增：错误类型转字符串
    bool saveMat(const cv::Mat &img, const QString &folder, const QString &fileName);
};

// 主管理类
class ImageNamer : public QObject {
    Q_OBJECT
public:
    explicit ImageNamer(QObject *parent = nullptr);
    ~ImageNamer();

    bool initializeCamera(int cameraId, const QString &rootPath);
    bool saveRaw(int cameraId, const cv::Mat &img, int triggerId, int rawIndex);
    bool saveComposite(int cameraId, const cv::Mat &img, int triggerId);
    bool saveResult(int cameraId, const cv::Mat &img, int triggerId, bool ok,
                    DetectionRsult errorType = NG, const QString& fileName = QString());
    bool updateCameraPath(int cameraId, const QString &newRootPath);
    int getCameraCount() const;

private:
    QMutex m_mutex;
    QMap<int, CameraSaverWorker*> m_workers;
    QMap<int, QThread*> m_threads;
};

#endif // IMAGENAMER_H
