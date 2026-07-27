#ifndef IMAGEZOOMVIEW_H
#define IMAGEZOOMVIEW_H

#include <QLabel>
#include <QPixmap>
#include <QPoint>
#include <QMenu>
#include <QRect>

class ImageZoomView : public QLabel
{
    Q_OBJECT
public:
    explicit ImageZoomView(QWidget *parent = nullptr);

    void setPixmap(const QPixmap &pixmap);
    void resetZoom();
    void updateScaledPixmap();
    void setOriginalPixmap(const QPixmap &pixmap);

    void startROIDrawing();
    void stopROIDrawing();
    void clearROI();
    QRect getROI() const;
    bool hasROI() const;

    void setMaxScaleFactor(float maxFactor);
    float maxScaleFactor() const;

signals:
    void roiDrawn(const QRect& roi);
    void roiCleared();

public slots:

protected:
    void paintEvent(QPaintEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void contextMenuEvent(QContextMenuEvent *event) override;

private:
    QPixmap originalPixmap;
    float scaleFactor;
    float maxScaleFactorValue;
    QPoint offset;
    QPoint lastMousePos;
    bool dragging;
    bool cacheDirty;
    QPixmap cachedPixmap;
    QMenu *contextMenu;

    bool drawingROI;
    QPoint roiStartPoint;
    QPoint roiEndPoint;
    QRect currentROI;

    QPoint windowToImage(const QPoint& windowPoint);
};

#endif
