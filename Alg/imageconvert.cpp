#include "imageconvert.h"
#include <QImage>
#include <opencv2/imgproc.hpp>

cv::Mat ImageConvert::QImageToCvMat(const QImage& img, bool clone) {
    QImage swapped = img.convertToFormat(QImage::Format_RGBA8888).rgbSwapped();
    cv::Mat mat(swapped.height(), swapped.width(), CV_8UC4,
                const_cast<uchar*>(swapped.bits()), swapped.bytesPerLine());
    return clone ? mat.clone() : mat;
}

QImage ImageConvert::CvMatToQImage(const cv::Mat& mat, bool clone) {
    cv::Mat rgb;
    if (mat.channels() == 3) {
        cv::cvtColor(mat, rgb, cv::COLOR_BGR2RGB);
    } else if (mat.channels() == 4) {
        cv::cvtColor(mat, rgb, cv::COLOR_BGRA2RGBA);
    } else {
        rgb = mat;
    }
    QImage img(rgb.data, rgb.cols, rgb.rows, rgb.step, QImage::Format_RGB888);
    return clone ? img.copy() : img;
}
