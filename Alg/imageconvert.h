#ifndef IMAGECONVERT_H
#define IMAGECONVERT_H

#include <QImage>
#include <opencv2/core/mat.hpp>

class ImageConvert {
public:
    // QImage → cv::Mat
    static cv::Mat QImageToCvMat(const QImage& img, bool clone = true);

    // cv::Mat → QImage
    static QImage CvMatToQImage(const cv::Mat& mat, bool clone = true);
};

#endif // IMAGECONVERT_H



// // QImage 转 cv::Mat
// cv::Mat AlgorithmWorker::qImageToCvMat(const QImage &image) const
// {
//     switch (image.format()) {
//     case QImage::Format_RGB32:
//     case QImage::Format_ARGB32:
//     case QImage::Format_ARGB32_Premultiplied:
//     {
//         cv::Mat mat(image.height(), image.width(), CV_8UC4,
//                     const_cast<uchar*>(image.bits()), image.bytesPerLine());
//         cv::Mat result;
//         cv::cvtColor(mat, result, cv::COLOR_RGBA2BGR);
//         return result;
//     }
//     case QImage::Format_RGB888:
//     {
//         cv::Mat mat(image.height(), image.width(), CV_8UC3,
//                     const_cast<uchar*>(image.bits()), image.bytesPerLine());
//         cv::Mat result;
//         cv::cvtColor(mat, result, cv::COLOR_RGB2BGR);
//         return result;
//     }
//     case QImage::Format_Grayscale8:
//     {
//         return cv::Mat(image.height(), image.width(), CV_8UC1,
//                        const_cast<uchar*>(image.bits()), image.bytesPerLine()).clone();
//     }
//     default:
//         // 转换为支持的格式
//         QImage converted = image.convertToFormat(QImage::Format_RGB888);
//         return qImageToCvMat(converted);
//     }
// }

// // cv::Mat 转 QImage
// QImage AlgorithmWorker::cvMatToQImage(const cv::Mat & src) const
// {
//     cv::Mat rgb;
//     switch (src.channels())
//     {
//     case 1:   // 灰度
//         cv::cvtColor(src, rgb, cv::COLOR_GRAY2RGB);
//         break;
//     case 3:   // BGR
//         cv::cvtColor(src, rgb, cv::COLOR_BGR2RGB);
//         break;
//     case 4:   // BGRA
//         cv::cvtColor(src, rgb, cv::COLOR_BGRA2RGBA);
//         break;
//     default:
//         return QImage();
//     }

//     // 深拷贝，让 QImage 自己持有数据
//     QImage img(rgb.data, rgb.cols, rgb.rows,
//                static_cast<int>(rgb.step),
//                rgb.channels() == 4 ? QImage::Format_RGBA8888
//                                    : QImage::Format_RGB888);
//     return img.copy();
// }
