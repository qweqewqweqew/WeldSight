#include "imagezoomview.h"
#include <QPainter>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QDebug>
#include <QMenu>
#include <QAction>
#include <QContextMenuEvent>
#include <QTimer>

ImageZoomView::ImageZoomView(QWidget *parent)
    : QLabel(parent), scaleFactor(1.0), maxScaleFactorValue(8.0),
    offset(0, 0), dragging(false), cacheDirty(true),
    drawingROI(false), currentROI(QRect())
{
    setAlignment(Qt::AlignCenter);
    setScaledContents(false);

    contextMenu = new QMenu(this);
    QAction *resetAction = contextMenu->addAction("Recover");
    QAction *clearROIAction = contextMenu->addAction("Clear ROI");
    connect(resetAction, &QAction::triggered, this, &ImageZoomView::resetZoom);
    connect(clearROIAction, &QAction::triggered, this, &ImageZoomView::clearROI);
}

void ImageZoomView::setMaxScaleFactor(float maxFactor)
{
    if (maxFactor > 1.0f) {
        maxScaleFactorValue = maxFactor;
    }
}

float ImageZoomView::maxScaleFactor() const
{
    return maxScaleFactorValue;
}

void ImageZoomView::setPixmap(const QPixmap &pixmap)
{
    originalPixmap = pixmap;
    scaleFactor = 1.0;
    offset = QPoint(0, 0);
    clearROI();
    cacheDirty = true;
    updateScaledPixmap();
}

void ImageZoomView::setOriginalPixmap(const QPixmap &pixmap)
{
    originalPixmap = pixmap;
    scaleFactor = 1.0;
    offset = QPoint(0, 0);
    clearROI();
    cacheDirty = true;
    updateScaledPixmap();
}

void ImageZoomView::updateScaledPixmap()
{
    if (!originalPixmap.isNull()) {
        QSize labelSize = size();
        QSize imageSize = originalPixmap.size();

        float widthScaleFactor = static_cast<float>(labelSize.width()) / imageSize.width();
        float heightScaleFactor = static_cast<float>(labelSize.height()) / imageSize.height();

        scaleFactor = qMin(widthScaleFactor, heightScaleFactor);

        cacheDirty = true;
        update();
    }
}

void ImageZoomView::resetZoom()
{
    scaleFactor = 1.0;
    offset = QPoint(0, 0);
    cacheDirty = true;
    updateScaledPixmap();
    update();
}

QPoint ImageZoomView::windowToImage(const QPoint& windowPoint)
{
    if (originalPixmap.isNull()) {
        return QPoint();
    }

    QSize scaledSize = cachedPixmap.size();
    QPoint drawPoint = rect().center() - QPoint(scaledSize.width() / 2, scaledSize.height() / 2) + offset;

    QPoint imagePoint = windowPoint - drawPoint;

    imagePoint = QPoint(
        static_cast<int>(imagePoint.x() / scaleFactor),
        static_cast<int>(imagePoint.y() / scaleFactor)
        );

    QSize imageSize = originalPixmap.size();
    imagePoint.setX(qBound(0, imagePoint.x(), imageSize.width()));
    imagePoint.setY(qBound(0, imagePoint.y(), imageSize.height()));

    return imagePoint;
}

void ImageZoomView::startROIDrawing()
{
    drawingROI = true;
    currentROI = QRect();
    setCursor(Qt::CrossCursor);
    update();
}

void ImageZoomView::stopROIDrawing()
{
    drawingROI = false;
    setCursor(Qt::ArrowCursor);
    update();
}

void ImageZoomView::clearROI()
{
    currentROI = QRect();
    emit roiCleared();
    update();
}

bool ImageZoomView::hasROI() const
{
    return !currentROI.isNull() && currentROI.isValid();
}

QRect ImageZoomView::getROI() const
{
    return currentROI;
}

void ImageZoomView::wheelEvent(QWheelEvent *event)
{
    QPoint numDegrees = event->angleDelta();
    if (!numDegrees.isNull() && !originalPixmap.isNull()) {
        float oldScale = scaleFactor;

        float newScale = scaleFactor;
        if (numDegrees.y() > 0) {
            newScale *= 1.1f;
            if (newScale > maxScaleFactorValue) {
                newScale = maxScaleFactorValue;

                setCursor(Qt::ForbiddenCursor);
                QTimer::singleShot(500, this, [this]() { setCursor(Qt::ArrowCursor); });
            }
        } else {
            newScale /= 1.1f;

            if (cursor().shape() == Qt::ForbiddenCursor) {
                setCursor(Qt::ArrowCursor);
            }
        }


        if (qAbs(newScale - scaleFactor) > 0.01f) {
            scaleFactor = newScale;


            QPointF mousePos = event->position();
            QSize scaledSize = originalPixmap.size() * oldScale;
            QPointF drawPoint = QPointF(rect().center()) - QPointF(scaledSize.width()/2.0, scaledSize.height()/2.0) + offset;
            QPointF imagePos = (mousePos - drawPoint) / oldScale;


            QSize newScaledSize = originalPixmap.size() * scaleFactor;
            QPointF newDrawPoint = QPointF(rect().center()) - QPointF(newScaledSize.width()/2.0, newScaledSize.height()/2.0);
            offset = (mousePos - newDrawPoint - imagePos * scaleFactor).toPoint();

            cacheDirty = true;
            update();
        }
    }
}

void ImageZoomView::resizeEvent(QResizeEvent *event)
{
    QLabel::resizeEvent(event);
    updateScaledPixmap();
}

void ImageZoomView::mouseDoubleClickEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
    resetZoom();
}

void ImageZoomView::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        if (drawingROI) {
            roiStartPoint = event->pos();
            roiEndPoint = event->pos();
        } else {
            dragging = true;
            lastMousePos = event->pos();
        }
    }
}

void ImageZoomView::mouseMoveEvent(QMouseEvent *event)
{
    if (dragging) {
        QPoint delta = event->pos() - lastMousePos;
        offset += delta;
        lastMousePos = event->pos();
        update();
    } else if (drawingROI) {
        roiEndPoint = event->pos();
        update();
    }
}

void ImageZoomView::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        if (dragging) {
            dragging = false;
        } else if (drawingROI) {
            QRect windowRect = QRect(roiStartPoint, roiEndPoint).normalized();

            QPoint topLeft = windowToImage(windowRect.topLeft());
            QPoint bottomRight = windowToImage(windowRect.bottomRight());

            currentROI = QRect(topLeft, bottomRight);

            emit roiDrawn(currentROI);
        }
    }
}

void ImageZoomView::contextMenuEvent(QContextMenuEvent *event)
{
    contextMenu->exec(event->globalPos());
}

void ImageZoomView::paintEvent(QPaintEvent *event)
{
    if (originalPixmap.isNull()) {
        QLabel::paintEvent(event);
        return;
    }

    QPainter painter(this);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);


    if (cacheDirty) {
        QSize scaledSize = originalPixmap.size() * scaleFactor;


        Qt::TransformationMode mode = (scaleFactor > 2.0f) ?
                                          Qt::FastTransformation :
                                          Qt::SmoothTransformation;

        cachedPixmap = originalPixmap.scaled(
            scaledSize,
            Qt::KeepAspectRatio,
            mode
            );
        cacheDirty = false;
    }

    QSize pixSize = cachedPixmap.size();
    QRect widgetRect = rect();
    int drawX = widgetRect.center().x() - pixSize.width()/2 + offset.x();
    int drawY = widgetRect.center().y() - pixSize.height()/2 + offset.y();

    painter.drawPixmap(drawX, drawY, cachedPixmap);

    if (drawingROI) {
        QRect roiRect = QRect(roiStartPoint, roiEndPoint).normalized();
        painter.setPen(QPen(Qt::red, 2, Qt::DashLine));
        painter.setBrush(Qt::NoBrush);
        painter.drawRect(roiRect);
    }
    else if (hasROI()) {
        int roiX = drawX + currentROI.x() * scaleFactor;
        int roiY = drawY + currentROI.y() * scaleFactor;
        int roiW = currentROI.width() * scaleFactor;
        int roiH = currentROI.height() * scaleFactor;
        QRect windowRect(roiX, roiY, roiW, roiH);

        painter.setPen(QPen(Qt::green, 2));
        painter.setBrush(Qt::NoBrush);
        painter.drawRect(windowRect);
    }
}
