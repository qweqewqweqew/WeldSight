#ifndef CVQT_TEXT_HPP
#define CVQT_TEXT_HPP

#include <opencv2/core.hpp>
#include <QStringList>
#include <QColor>
#include <QPixmap>
#include <QImage>
#include <QPainter>
#include <QFontMetrics>

struct LineStyle
{
    int     fontSize   = 20;
    QColor  color      = Qt::black;
    QString fontFamily = "Microsoft YaHei";
    int     weight     = QFont::Normal;   // 0–99；QFont::Bold = 75
};

namespace cvqt {

inline QImage cvMatToQImage(const cv::Mat& src)
{
    switch (src.type()) {
    case CV_8UC3: return QImage(src.data, src.cols, src.rows,
                      static_cast<int>(src.step),
                      QImage::Format_BGR888).copy();
    case CV_8UC4: return QImage(src.data, src.cols, src.rows,
                      static_cast<int>(src.step),
                      QImage::Format_ARGB32).copy();
    default: return {};
    }
}

inline cv::Mat qImageToCvMat(const QImage& src)
{
    QImage img = src.convertToFormat(QImage::Format_BGR888);
    return cv::Mat(img.height(), img.width(), CV_8UC3,
                   const_cast<uchar*>(img.constBits()),
                   img.bytesPerLine()).clone();
}

inline QPixmap drawText(const cv::Mat& src,
                        const QStringList& lines,
                        const std::vector<LineStyle>& styles,
                        cv::Point org = {20, 60},
                        int lineSpacing = 10)
{
    if (lines.size() != static_cast<int>(styles.size())) return {};

    QImage img = cvMatToQImage(src);
    if (img.isNull()) return {};

    QPainter p(&img);
    p.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
    QPoint cur(org.x, org.y);

    for (size_t i = 0; i < lines.size(); ++i) {
        const auto& txt = lines[i];
        const auto& st  = styles[i];

        QFont f(st.fontFamily.isEmpty() ? "Microsoft YaHei" : st.fontFamily,
                st.fontSize,
                st.weight);          // 关键：weight 控制粗细
        p.setFont(f);
        p.setPen(st.color);
        p.drawText(cur, txt);
        cur.ry() += QFontMetrics(f).height() + lineSpacing;
    }
    p.end();
    return QPixmap::fromImage(img);
}





inline QImage drawImgText(const cv::Mat& src,
                       const QStringList& lines,
                       const std::vector<LineStyle>& styles,
                       cv::Point org = {20, 60},
                       int lineSpacing = 10)
{
    if (lines.size() != static_cast<int>(styles.size()))
        return {};

    QImage img = cvMatToQImage(src);
    if (img.isNull())
        return {};

    QPainter p(&img);
    p.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
    QPoint cur(org.x, org.y);

    for (size_t i = 0; i < lines.size(); ++i) {
        const auto& txt = lines[i];
        const auto& st  = styles[i];

        QFont f(st.fontFamily.isEmpty() ? QStringLiteral("Microsoft YaHei")
                                        : st.fontFamily,
                st.fontSize,
                st.weight);
        p.setFont(f);
        p.setPen(st.color);
        p.drawText(cur, txt);
        cur.ry() += QFontMetrics(f).height() + lineSpacing;
    }
    p.end();
    return img;              // 直接返回 QImage
}

} // namespace cvqt
#endif // CVQT_TEXT_HPP
