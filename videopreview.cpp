#include "videopreview.h"
#include <QDebug>
#include <QPainter>

VideoPreview::VideoPreview(QWidget *parent)
    : QWidget(parent)
    , vsurface(nullptr)
{

}

void VideoPreview::paintEvent(QPaintEvent *event)
{
    QImage image = vsurface->getImage();
    if (image.isNull()) {
        qDebug() << "image is null";
        return;
    }
    QPainter painter(this);
    painter.drawImage(0, 0, image);
}

void VideoPreview::setVideoSurface(VideoSurface *vs)
{
    vsurface = vs;
}
