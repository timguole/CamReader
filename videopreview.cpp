#include "videopreview.h"
#include <QDebug>

VideoPreview::VideoPreview(QWidget *parent)
    : QWidget(parent)
    , vsurface(nullptr)
{

}

void VideoPreview::paintEvent(QPaintEvent *event)
{
    qDebug() << "paint";
    QImage *image = vsurface->getImage();
    if (image == nullptr) {
        qDebug() << "image is null";
    }
    QWidget::paintEvent(event);
}

void VideoPreview::setVideoSurface(VideoSurface *vs)
{
    vsurface = vs;
}
