#ifndef VIDEOSURFACE_H
#define VIDEOSURFACE_H

#include <QAbstractVideoSurface>
#include <QImage>

class VideoSurface : public QAbstractVideoSurface
{
    Q_OBJECT

public:
    VideoSurface();
    ~VideoSurface();

    QList<QVideoFrame::PixelFormat> supportedPixelFormats(
            QAbstractVideoBuffer::HandleType type) const;
    bool present(const QVideoFrame &frame);
    QImage * getImage();

signals:
    void newFrame();

private:
    QImage *image;
};

#endif // VIDEOSURFACE_H
