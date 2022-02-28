#ifndef VIDEOPREVIEW_H
#define VIDEOPREVIEW_H

#include <QWidget>
#include <QMouseEvent>
#include <QResizeEvent>

class VideoPreview : public QWidget
{
    Q_OBJECT
public:
    explicit VideoPreview(QWidget *parent = nullptr);
    void paintEvent(QPaintEvent *event);
    int getBBThreshold();
    void setBBThreshold(int t);
    void setFrame(QImage &i);

public slots:
    void toggleInvertColor();
    void toggleResize();
    void toggleBB();
    void toggleCurveBB();

private:
    void blackboardRgba(QImage &image);
    void blackboardRgb888(QImage &image);
    void updateVewportSize();

    // override event handlers
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseDoubleClickEvent(QMouseEvent *event);
    void wheelEvent(QWheelEvent *we);
    void resizeEvent(QResizeEvent *event);

    QImage image;
    volatile bool isInvertColor;
    volatile bool isBloackBoard;
    volatile bool isCutoff;
    volatile int blackboardThreshold;
    int xRect; // x of the image's visible rectangle
    int yRect; // y of the image's visible rectangle
    int xPressed; // x value when mouse pressed
    int yPressed; // y value when mouse pressed
    int imageWidth; // original frame image width
    int imageHeight; // original frame image height
    int scaleWidth;
    int scaleHeight;
    int fitWidth;
    int fitHeight;
};

#endif // VIDEOPREVIEW_H
