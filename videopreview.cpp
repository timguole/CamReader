#include "videopreview.h"
#include <QDebug>
#include <QPainter>

VideoPreview::VideoPreview(QWidget *parent)
    : QWidget(parent)
    , isInvertColor(false)
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
    scaleWidth = width();
    scaleHeight = height();
}

void VideoPreview::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);

    if (image.isNull()) {
        qDebug() << "image is null";
        return;
    }

    // if original frame size changed ( e.g.
    // changing to a different device),
    // reset offset of x and y
    if ((image.width() != imageWidth)
            || (image.height() != imageHeight)) {
        xRect = 0; // reset
        yRect = 0; // reset
        imageWidth = image.width();
        imageHeight = image.height();
        scaleWidth = width();
        scaleHeight = height();
    }

    // scale image and display visible rectangle of image.
    int ww = width();
    int wh = height();

    // if image size is different from scale size,
    // we scale the image
    if (((image.width() != scaleWidth)
            || (image.height() != scaleHeight))) {
        image = image.scaled(QSize(scaleWidth, scaleHeight),
                                   Qt::KeepAspectRatio);
    }

    int wRect = image.width() > ww ? ww : image.width();
    int hRect = image.height() > wh ? wh : image.height();
    QImage croppedImage = image.copy(QRect(xRect, yRect, wRect, hRect));

    // if iamge is smaller than the widget,
    // display it at the center of widget
    int x = 0;
    int y = 0;
    if (scaleWidth < ww) {
        x = (ww - scaleWidth) / 2;
    }
    if (scaleHeight < wh) {
        y = (wh - scaleHeight) / 2;
    }

    // if color inversion is set, do it
    if (isInvertColor) {
        croppedImage.invertPixels();
    }

    // blackboard mode makes dark pixels pure black
    if (isBloackBoard) {
        if (croppedImage.format() == QImage::Format_RGB888) {
            blackboardRgb888(croppedImage);
        } else if (croppedImage.format() == QImage::Format_ARGB32
                   || croppedImage.format() == QImage::Format_RGB32) {
            blackboardRgba(croppedImage);
        } else {
            qDebug() << "not supported image format:" << image.format();
        }
    }
    painter.drawImage(x, y, croppedImage);
}

int VideoPreview::getBBThreshold()
{
    return blackboardThreshold;
}

void VideoPreview::toggleInvertColor(bool checked)
{
    isInvertColor = !isInvertColor;
}

void VideoPreview::toggleResize(bool checked)
{
    scaleWidth = (scaleWidth != imageWidth) ? imageWidth : width();
    scaleHeight = (scaleHeight != imageHeight) ? imageHeight : height();
    updateVewportSize();
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
            && ((xRect - xoffset) <= (scaleWidth - width()))) {
        xRect -= xoffset;
    }
    if (((yRect - yoffset) >= 0)
            && ((yRect - yoffset) <= (scaleHeight - height()))) {
        yRect -= yoffset;
    }
    xPressed = x;
    yPressed = y;
    event->accept();
}

void VideoPreview::resizeEvent(QResizeEvent *event)
{
    scaleWidth = event->size().width();
    scaleHeight = event->size().height();
    updateVewportSize();
}

void VideoPreview::wheelEvent(QWheelEvent *we)
{
    double factor = (we->angleDelta().y() > 0) ? 1.25 : 0.80;
    scaleWidth *= factor;
    scaleHeight *= factor;
    updateVewportSize();
}

void VideoPreview::updateVewportSize()
{
    scaleWidth = (scaleWidth < 320) ? 320 : scaleWidth;
    scaleWidth = (scaleWidth > 8000) ? 8000 : scaleWidth;
    scaleHeight = (scaleHeight < 180) ? 180 : scaleHeight;
    scaleHeight = (scaleHeight > 4500) ? 4500 : scaleHeight;

    int xnewmax = scaleWidth - width();
    int ynewmax = scaleHeight - height();
    xRect = ((xRect > xnewmax) && (xnewmax > 0)) ? xnewmax : xRect;
    yRect = ((yRect > ynewmax) && (ynewmax > 0)) ? ynewmax : yRect;

    xRect = (xnewmax <= 0) ? 0 : xRect;
    yRect = (ynewmax <= 0) ? 0 : yRect;
}
