#ifndef VIDEOPREVIEW_H
#define VIDEOPREVIEW_H

#include <QWidget>
#include "videosurface.h"

class VideoPreview : public QWidget
{
    Q_OBJECT
public:
    explicit VideoPreview(QWidget *parent = nullptr);
    void paintEvent(QPaintEvent *event);
    void setVideoSurface(VideoSurface *vs);
    int getBBThreshold();
    void setBBThreshold(int t);

public slots:
    void toggleInvertColor(bool);
    void toggleScale(bool);
    void toggleBlackboard(bool);

private:
    void blackboardRgba(QImage &image);
    void blackboardRgb(QImage &image);

    VideoSurface *vsurface;
    volatile bool isInvertColor;
    volatile bool isScaleImage;
    volatile bool isBloackBoard;
    volatile int blackboardThreshold;
};

#endif // VIDEOPREVIEW_H
