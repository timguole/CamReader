#include "videopreview.h"
#include <QDebug>
#include <QPainter>

VideoPreview::VideoPreview(QWidget *parent)
    : QWidget(parent)
    , vsurface(nullptr)
    , isInvertColor(false)
    , isScaleImage(true)
{

}

void VideoPreview::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    QImage image = vsurface->getImage();

    if (image.isNull()) {
        qDebug() << "image is null";
        return;
    }
    if (isInvertColor) {
        image.invertPixels();
    }

    // Scale image to widget size
    QImage scaledImage;
    int ww = width();
    int wh = height();
    int x = 0;
    int y = 0;

    if (isScaleImage && ((image.width() != ww)
            || (image.height() != wh))) {
        scaledImage = image.scaled(size(), Qt::KeepAspectRatio);
        int iw = scaledImage.width();
        int ih = scaledImage.height();

        if (iw < ww) {
            x = (ww - iw) / 2;
        } else if (ih < wh) {
            y = (wh - ih) / 2;
        }
    } else {
        scaledImage = image;
    }
    painter.drawImage(x, y, scaledImage);
}

void VideoPreview::setVideoSurface(VideoSurface *vs)
{
    vsurface = vs;
}

void VideoPreview::toggleInvertColor(bool checked)
{
    qDebug() << "action checked: " << checked;
    isInvertColor = !isInvertColor;
}

void VideoPreview::toggleScale(bool checked)
{
    qDebug() << "action checked: " << checked;
    isScaleImage = !isScaleImage;
}
