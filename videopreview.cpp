#include "videopreview.h"
#include <QDebug>
#include <QPainter>

VideoPreview::VideoPreview(QWidget *parent)
    : QWidget(parent)
    , isInvertColor(false)
    , isScaleImage(true)
    , isBloackBoard(false)
    , isCutoff(true)
    , blackboardThreshold(100)
    , xRect(0)
    , yRect(0)
    , xPressed(0)
    , yPressed(0)
    , imageWidth(0)
    , imageHeight(0)
{

}

void VideoPreview::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);

    if (image.isNull()) {
        qDebug() << "image is null";
        return;
    }

    if ((image.width() != imageWidth)
            || (image.height() != imageHeight)) {
        xRect = 0;
        yRect = 0;
        imageWidth = image.width();
        imageHeight = image.height();
    }

    if (isInvertColor) {
        image.invertPixels();
    }
    if (isBloackBoard) {
        if (image.format() == QImage::Format_RGB888) {
            blackboardRgb888(image);
        } else if (image.format() == QImage::Format_ARGB32
                   || image.format() == QImage::Format_RGB32) {
            blackboardRgba(image);
        } else {
            qDebug() << "not supported image format:" << image.format();
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
        }
        if (ih < wh) {
            y = (wh - ih) / 2;
        }
    } else {
        int wRect = image.width() > ww ? ww : image.width();
        int hRect = image.height() > wh ? wh : image.height();
        scaledImage = image.copy(QRect(xRect, yRect, wRect, hRect));
        if (imageWidth < ww) {
            x = (ww - imageWidth) / 2;
        }
        if (imageHeight < wh) {
            y = (wh - imageHeight) / 2;
        }
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
    isScaleImage = !isScaleImage;
}

void VideoPreview::toggleBB(bool checked)
{
    isBloackBoard = !isBloackBoard;
}

void VideoPreview::toggleCurveBB(bool checked)
{
    isCutoff = !isCutoff;
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
            int r = qRed(line[j]);
            int g = qGreen(line[j]);
            int b = qBlue(line[j]);
            if (r < blackboardThreshold
                    && g < blackboardThreshold
                    && b < blackboardThreshold) {
                if (isCutoff) {
                    line[j] = black;
                } else {
                    line[j] = qRgb(r*r/255, g*g/255, b*b/255);
                }
            }

        }
    }
}

void VideoPreview::blackboardRgb888(QImage &image)
{
    int h = image.height();
    int w = image.width();
    for (int i = 0; i < h; i++) {
        uchar *line = image.scanLine(i);
        for (int j = 0; j < w*3; j+=3) {
            if (line[j] < blackboardThreshold
                    && line[j+1] < blackboardThreshold
                    && line[j+2] < blackboardThreshold) {
                if (isCutoff) {
                    line[j] = 0;
                    line[j+1] = 0;
                    line[j+2] = 0;
                } else {
                    line[j] = line[j]*line[j]/255;
                    line[j+1] = line[j+1]*line[j+1]/255;
                    line[j+2] = line[j+2]*line[j+2]/255;
                }
            }

        }
    }
}

void VideoPreview::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::MouseButton::LeftButton) {
        event->accept();
    } else {
        return;
    }
    xPressed = QCursor::pos().x();
    yPressed = QCursor::pos().y();
}

void VideoPreview::mouseMoveEvent(QMouseEvent *event)
{
    int x = QCursor::pos().x();
    int y = QCursor::pos().y();
    int xoffset = x - xPressed;
    int yoffset = y - yPressed;

    if (((xRect - xoffset) >= 0)
            && ((xRect - xoffset) <= (imageWidth - width()))) {
        xRect -= xoffset;
    }
    if (((yRect - yoffset) >= 0)
            && ((yRect - yoffset) <= (imageHeight - height()))) {
        yRect -= yoffset;
    }
    xPressed = x;
    yPressed = y;
    event->accept();
}
