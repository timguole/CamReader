#include "videosurface.h"

#include <QDebug>

VideoSurface::VideoSurface()
    : QAbstractVideoSurface()
    , image(nullptr)
{

}

VideoSurface::~VideoSurface()
{
    if (image != nullptr) {
        delete image;
    }
}

QList<QVideoFrame::PixelFormat> VideoSurface::supportedPixelFormats(
        QAbstractVideoBuffer::HandleType type = QAbstractVideoBuffer::NoHandle) const
{
    if (type == QAbstractVideoBuffer::NoHandle) {
        return QList<QVideoFrame::PixelFormat>()
                << QVideoFrame::Format_RGB24
                << QVideoFrame::Format_RGB32
                << QVideoFrame::Format_RGB555
                << QVideoFrame::Format_RGB565
                << QVideoFrame::Format_ARGB32
                << QVideoFrame::Format_ARGB32
                << QVideoFrame::Format_BGR24
                << QVideoFrame::Format_BGR32
                << QVideoFrame::Format_BGR555
                << QVideoFrame::Format_BGR565;
    } else {
        return QList<QVideoFrame::PixelFormat>();
    }
}

bool VideoSurface::present(const QVideoFrame &frame)
{
    if (frame.isValid()) {
        qDebug() << frame.pixelFormat();
        emit newFrame();
        return true;
    } else {
        qDebug() << "invalid frame";
        return false;
    }
}

QImage * VideoSurface::getImage()
{
    return image;
}
