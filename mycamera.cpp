#include "mycamera.h"
#include <unistd.h>
#include <QDebug>

MyCamera::MyCamera(QString device, QObject *parent) : QObject(parent)
  , dev_name(device)
  , fd(-1)
  , cam_state(CAM_STATE::INVALID)
  , buf_type(V4L2_BUF_TYPE_VIDEO_CAPTURE)
  , framefsi()
  , mybuffers()
{

}

MyCamera::~MyCamera()
{
    qDebug() << "~MyCamera";
    for (MyBuffer mb : mybuffers) {
        munmap(mb.start, mb.length);
    }

    switch (cam_state) {
    case CAM_STATE::STREAMON:
        if (::ioctl(fd, VIDIOC_STREAMOFF, &buf_type) == -1) {
            qDebug() << "turn off stream failed";
        }
        cam_state = CAM_STATE::INITED;
    case CAM_STATE::INITED:
    case CAM_STATE::OPENED:
        ::close(fd);
        cam_state = CAM_STATE::CLOSED;
    case CAM_STATE::INVALID:
    case CAM_STATE::CLOSED:
        fd = -1;
        cam_state = CAM_STATE::INVALID;

    }
}

int MyCamera::open()
{
    if (cam_state != CAM_STATE::INVALID) {
        emit errored("Already opened");
        cam_state = CAM_STATE::ERRORED;
        return 1;
    }

    fd = ::open(dev_name.toUtf8().constData(), O_RDWR);
    if (fd < 0) {
        emit errored("Failed to open device");
        cam_state = CAM_STATE::ERRORED;
        return 1;
    }

    if (::ioctl(fd, VIDIOC_QUERYCAP, &cam_caps) == -1) {
        emit errored("Failed to query cam cap");
        cam_state = CAM_STATE::ERRORED;
        return 1;
    }

    // Get frame info: format, resolution, rate
    struct v4l2_fmtdesc fmtdesc;
    fmtdesc.index = 0;
    fmtdesc.type = buf_type;
    while (::ioctl(fd, VIDIOC_ENUM_FMT, &fmtdesc) != -1) {
        fmtdesc.index++;
        struct v4l2_frmsizeenum fse;
        fse.index = 0;
        fse.type = V4L2_FRMSIZE_TYPE_DISCRETE;
        fse.pixel_format = fmtdesc.pixelformat;
        while (::ioctl(fd, VIDIOC_ENUM_FRAMESIZES, &fse) != -1) {
            fse.index++;
            struct v4l2_frmivalenum fie;
            fie.index = 0;
            fie.pixel_format = fmtdesc.pixelformat;
            fie.type = V4L2_FRMSIZE_TYPE_DISCRETE;
            fie.width = fse.discrete.width;
            fie.height = fse.discrete.height;
            while (::ioctl(fd, VIDIOC_ENUM_FRAMEINTERVALS, &fie) != -1) {
                fie.index++;
                FrameFSI ffsi;
                ffsi.formatName = QString((char *)fmtdesc.description);
                ffsi.formatInt = fse.pixel_format;
                ffsi.width = fse.discrete.width;
                ffsi.height = fse.discrete.height;
                ffsi.denominator = fie.discrete.denominator;
                ffsi.numerator = fie.discrete.numerator;
                framefsi << ffsi;
            }
        }
    }

    // get default format
    defaultformat.type = buf_type;
    if (::ioctl(fd, VIDIOC_G_FMT, &defaultformat) == -1) {
        emit errored("Failed to get default format");
        cam_state = CAM_STATE::ERRORED;
        return 1;
    }
    cam_state = CAM_STATE::OPENED;
    return 0;
}

QString MyCamera::driverName()
{
    return QString((char *)cam_caps.driver);
}

int MyCamera::driverVersion()
{
    return cam_caps.version;
}

QString MyCamera::bufInfo()
{
    return QString((char *)cam_caps.bus_info);
}

QString MyCamera::cardInfo()
{
    return QString((char *)cam_caps.card);
}

int MyCamera::capability()
{
    return cam_caps.capabilities;
}

bool MyCamera::isCaptureSupported()
{
    return (cam_caps.capabilities & V4L2_CAP_VIDEO_CAPTURE) == V4L2_CAP_VIDEO_CAPTURE;
}

bool MyCamera::isStreamSupported()
{
    return (cam_caps.capabilities & V4L2_CAP_STREAMING) == V4L2_CAP_STREAMING;
}

QList<FrameFSI> MyCamera::supportedResolutions()
{
    return framefsi;
}

MyCamera::CAM_STATE MyCamera::currentState()
{
    return cam_state;
}

void MyCamera::setFrameFSI(FrameFSI &ffsi)
{
    if (cam_state != CAM_STATE::OPENED) {
        emit errored("Config a not opened device");
        cam_state = CAM_STATE::ERRORED;
        return;
    }

    // set frame settings
    defaultformat.fmt.pix.width = ffsi.width;
    defaultformat.fmt.pix.height = ffsi.height;
    defaultformat.fmt.pix.pixelformat = ffsi.formatInt;
    if (::ioctl(fd, VIDIOC_S_FMT, &defaultformat) == -1) {
        emit errored("Failed to set format or resolution");
        cam_state = CAM_STATE::ERRORED;
        return;
    }

    // prepare buffers
    struct v4l2_requestbuffers reqbuf;

    ::memset(&reqbuf, 0, sizeof(reqbuf));
    reqbuf.type = buf_type;
    reqbuf.memory = V4L2_MEMORY_MMAP;
    reqbuf.count = 4;

    if (-1 == ::ioctl (fd, VIDIOC_REQBUFS, &reqbuf)) {
        if (errno == EINVAL) {
            emit errored("capturing or mmap is not supported");
        } else {
            emit errored("VIDIOC_REQBUFS");
        }
        cam_state = CAM_STATE::ERRORED;
        return;
    }

    if (reqbuf.count == 0) {
        emit errored("Not enough buffer memory");
        cam_state = CAM_STATE::ERRORED;
        return;
    }

    for (int i = 0; i < reqbuf.count; i++) {
        struct v4l2_buffer buffer;

        memset(&buffer, 0, sizeof(buffer));
        buffer.type = reqbuf.type;
        buffer.memory = V4L2_MEMORY_MMAP;
        buffer.index = i;

        if (-1 == ioctl (fd, VIDIOC_QUERYBUF, &buffer)) {
            emit errored("VIDIOC_QUERYBUF");
            cam_state = CAM_STATE::ERRORED;
            return;
        }

        MyBuffer mb;
        mb.length = buffer.length;
        mb.start = mmap(NULL, buffer.length,
                    PROT_READ | PROT_WRITE, /* recommended */
                    MAP_SHARED,             /* recommended */
                    fd, buffer.m.offset);

        if (MAP_FAILED == mb.start) {
            emit errored("mmap failed");
            cam_state = CAM_STATE::ERRORED;
            return;
        }

        // queue buffer
        if (::ioctl(fd, VIDIOC_QBUF, &buffer) == -1) {
            //emit errored("Failed to queue buffer");
            qDebug() << "Failed to queue buffer";
        }

        // save nmapped memory info
        mybuffers << mb;
    }

    cam_state = CAM_STATE::INITED;
}

void MyCamera::turnOn()
{
    if (::ioctl(fd, VIDIOC_STREAMON, &buf_type) == -1) {
        emit errored("Failed to turn on streaming");
        cam_state = CAM_STATE::ERRORED;
        return;
    }

    cam_state = CAM_STATE::STREAMON;
}

// Return a QByteArray containing the frame data
// On error, an empty QByteArray is returned.
QPixmap MyCamera::capture()
{
    QPixmap pixmap;

    if (cam_state != CAM_STATE::STREAMON) {
        qDebug() << "Capture on a not stream-on device";
        return pixmap; // return the empty pixmap
    }

    struct v4l2_buffer buf;
    ::memset(&buf, 0, sizeof(buf));
    buf.type = buf_type;
    buf.memory = V4L2_MEMORY_MMAP;

    if (::ioctl(fd, VIDIOC_DQBUF, &buf) == -1) {
        qDebug() << "Failed to capture data";
        cam_state = CAM_STATE::ERRORED;
        return pixmap;
    }

    MyBuffer mb = mybuffers.at(buf.index);
    pixmap.loadFromData((const unsigned char *)mb.start, mb.length);

    if (::ioctl(fd, VIDIOC_QBUF, &buf) == -1) {
        qDebug() << "Failed to queue buffer";
        cam_state = CAM_STATE::ERRORED;
    }

    return pixmap;
}
