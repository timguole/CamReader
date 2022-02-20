#include "videopreview.h"
#include <QDebug>
#include <QPainter>

VideoPreview::VideoPreview(QWidget *parent)
    : QWidget(parent)
    , isInvertColor(false)
    , isScaleImage(true)
    , isBloackBoard(false)
    , blackboardThreshold(100)
{

}

void VideoPreview::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);

    if (image.isNull()) {
        qDebug() << "image is null";
        return;
    }
    if (isInvertColor) {
        image.invertPixels();
    }
    if (isBloackBoard) {
        if (image.format() == QImage::Format_RGB888) {
            blackboardRgb(image);
        } else if (image.format() == QImage::Format_ARGB32) {
            blackboardRgba(image);
        }
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

int VideoPreview::getBBThreshold()
{
    return blackboardThreshold;
}

void VideoPreview::toggleInvertColor(bool checked)
{
    isInvertColor = !isInvertColor;
}

void VideoPreview::toggleScale(bool checked)
{
    qDebug() << "action checked: " << checked;
    isScaleImage = !isScaleImage;
}

void VideoPreview::toggleBlackboard(bool checked)
{
    qDebug() << "action checked: " << checked;
    isBloackBoard = !isBloackBoard;
}

void VideoPreview::setBBThreshold(int t)
{
    blackboardThreshold = t;
}

void VideoPreview::setFrame(QImage &i)
{
    image = i;
}

void VideoPreview::blackboardRgba(QImage &image)
{
    int h = image.height();
    int w = image.width();
    QRgb black = qRgb(0, 0, 0);

    for (int i = 0; i < h; i++) {
        QRgb *line = (QRgb *)image.scanLine(i);
        for (int j = 0; j < w; j++) {
            if (qRed(line[j]) < blackboardThreshold
                    && qGreen(line[j]) < blackboardThreshold
                    && qBlue(line[j]) < blackboardThreshold) {
                line[j] = black;
            }

        }
    }
}

void VideoPreview::blackboardRgb(QImage &image)
{
    int h = image.height();
    int w = image.width();
    for (int i = 0; i < h; i++) {
        uchar *line = image.scanLine(i);
        for (int j = 0; j < w*3; j+=3) {
            if (line[j] < blackboardThreshold
                    && line[j+1] < blackboardThreshold
                    && line[j+2] < blackboardThreshold) {
                line[j] = 0;
                line[j+1] = 0;
                line[j+2] = 0;
            }

        }
    }
}
