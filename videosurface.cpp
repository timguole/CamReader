#include "videosurface.h"

#include <QDebug>

VideoSurface::VideoSurface()
    : QAbstractVideoSurface()
{

}

VideoSurface::~VideoSurface()
{

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
        QVideoFrame copyFrame(frame);
        bool isMapped = copyFrame.map(QAbstractVideoBuffer::ReadOnly);
        if (!isMapped) {
            qDebug() << "failed to map frame";
            stop();
            return false;
        }
        image = QImage(copyFrame.bits(),
                       copyFrame.width(),
                       copyFrame.height(),
                       QVideoFrame::imageFormatFromPixelFormat(copyFrame.pixelFormat()));
        copyFrame.unmap();
        emit newFrame();
        return true;
    } else {
        qDebug() << "invalid frame";
        stop();
        return false;
    }
}

QImage VideoSurface::getImage()
{
    return image;
}
