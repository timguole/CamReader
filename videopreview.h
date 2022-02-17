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

public slots:
    void toggleInvertColor(bool);
    void toggleScale(bool);

private:
    VideoSurface *vsurface;
    volatile bool isInvertColor;
    volatile bool isScaleImage;
};

#endif // VIDEOPREVIEW_H
