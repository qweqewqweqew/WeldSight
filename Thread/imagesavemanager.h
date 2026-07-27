#ifndef IMAGESAVEMANAGER_H
#define IMAGESAVEMANAGER_H

#include <QObject>
#include "imagenamer.h"
#include "Form/saveform.h"
#include "Common/common.h"

class ImageSaveManager : public QObject
{
    Q_OBJECT
public:
    explicit ImageSaveManager(QObject *parent = nullptr);
    ~ImageSaveManager();

    void initialize();
    void setSaveForm(SaveForm* saveForm);

    // 保存原始图像（直接从相机捕获）
    bool saveRawImage(int cameraId, const cv::Mat& image, int triggerId, int rawIndex = 1);

    // 保存合成图像（处理后的结果）
    bool saveCompositeImage(int cameraId, const cv::Mat& image, int triggerId);

    // 保存检测结果图像（带有标注的结果）
    bool saveResultImage(int cameraId, const WeldingResult& result, int triggerId,
                         const QString& fileName = QString());

    void updateSavePath(const QString &newBasePath);
private:
    ImageNamer* m_imageNamer;
    SaveForm* m_saveForm;
};

#endif // IMAGESAVEMANAGER_H
