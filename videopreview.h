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

private:
    VideoSurface *vsurface;
};

#endif // VIDEOPREVIEW_H
