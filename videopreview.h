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
    void toggleInvertColor(bool);
    void toggleResize(bool);
    void toggleBB(bool);
    void toggleCurveBB(bool);

private:
    void blackboardRgba(QImage &image);
    void blackboardRgb888(QImage &image);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void wheelEvent(QWheelEvent *we);
    void resizeEvent(QResizeEvent *event);

    QImage image;
    volatile bool isInvertColor;
    volatile bool isBloackBoard;
    volatile bool isCutoff;
    volatile int blackboardThreshold;
    int xRect; // x of the image's visible rectangle
    int yRect; // y of the image's visible rectangle
    int xPressed;
    int yPressed;
    int imageWidth;
    int imageHeight;
    int scaleWidth;
    int scaleHeight;
};

#endif // VIDEOPREVIEW_H
