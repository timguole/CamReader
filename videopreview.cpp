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
    , scaleWidth(0)
    , scaleHeight(0)
    , fitWidth(0)
    , fitHeight(0)
{

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

        // the frame aspect ratio may change,
        // so reset scale values and 'fit' values;
        scaleWidth = 0;
        scaleHeight = 0;
        fitWidth = 0;
        fitHeight = 0;
    }

    // scale image and display visible rectangle of image.
    int ww = width();
    int wh = height();

    // if image size is different from scale size,
    // we scale the image
    if (((image.width() != scaleWidth)
            || (image.height() != scaleHeight))) {
<<<<<<< HEAD
        if (fitWidth == 0) {
=======
        if (scaleWidth == 0) {
>>>>>>> e27c4b80e681b7d9e10c30cfc157b8ea8272e9d1
            image = image.scaled(QSize(width(), height()),
                                   Qt::KeepAspectRatio);
        } else {
            image = image.scaled(QSize(scaleWidth, scaleHeight),
                                   Qt::KeepAspectRatio);
        }
    }
    if (fitWidth == 0) {
        fitWidth = image.width();
        fitHeight = image.height();
        scaleWidth = image.width();
        scaleHeight = image.height();
    }

    int wRect = image.width() > ww ? ww : image.width();
    int hRect = image.height() > wh ? wh : image.height();
    QImage croppedImage = image.copy(QRect(xRect, yRect, wRect, hRect));

    // if iamge is smaller than the widget,
    // display it at the center of widget
    int x = 0;
    int y = 0;
    int cw = croppedImage.width();
    int ch = croppedImage.height();
    if (cw < ww) {
        x = (ww - cw) / 2;
    }
    if (ch < wh) {
        y = (wh - ch) / 2;
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

    if ((cw < ww)
            || (ch < wh)) {
        painter.fillRect(0, 0, ww, wh, Qt::black);
    }
    painter.drawImage(x, y, croppedImage);
    event->accept();
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
    scaleWidth = (scaleWidth != imageWidth) ? imageWidth : fitWidth;
    scaleHeight = (scaleHeight != imageHeight) ? imageHeight : fitHeight;
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

    if ((scaleWidth > width())
            && ((xRect - xoffset) >= 0)
            && ((xRect - xoffset) <= (scaleWidth - width()))) {
        xRect -= xoffset;
        qDebug() << xRect << scaleWidth << width();
    }
    if ((scaleHeight > height())
            && ((yRect - yoffset) >= 0)
            && ((yRect - yoffset) <= (scaleHeight - height()))) {
        yRect -= yoffset;
    }
    xPressed = x;
    yPressed = y;
    event->accept();
}

void VideoPreview::resizeEvent(QResizeEvent *event)
{
<<<<<<< HEAD
    fitWidth = 0;
    fitHeight = 0;
=======
>>>>>>> e27c4b80e681b7d9e10c30cfc157b8ea8272e9d1
    updateVewportSize();
    event->accept();
}

void VideoPreview::wheelEvent(QWheelEvent *we)
{
    double factor = (we->angleDelta().y() > 0) ? 1.25 : 0.80;
    int newsw = scaleWidth * factor;
    int newsh = scaleHeight * factor;

    // Avoid from scaling the image too large of too snall.
    // we just need to check the value of width,
    // because frame image is always in a reasonable aspect ratio
    if ((newsw >= 320) && (newsw <= 8000)) {
        scaleWidth = newsw;
        scaleHeight = newsh;
    }
    updateVewportSize();
}

void VideoPreview::updateVewportSize()
{
    int xnewmax = scaleWidth - width();
    int ynewmax = scaleHeight - height();
    xRect = ((xRect > xnewmax) && (xnewmax > 0)) ? xnewmax : xRect;
    yRect = ((yRect > ynewmax) && (ynewmax > 0)) ? ynewmax : yRect;

    xRect = (xnewmax <= 0) ? 0 : xRect;
    yRect = (ynewmax <= 0) ? 0 : yRect;
}
