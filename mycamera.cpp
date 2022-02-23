#include "mycamera.h"
#include <unistd.h>
#include <QDebug>

MyCamera::MyCamera(QString device, QObject *parent) : QObject(parent)
  , dev_name(device)
  , fd(-1)
  , cam_state(CAM_STATE::INVALID)
  , buf_type(V4L2_BUF_TYPE_VIDEO_CAPTURE)
  , framefsi()
{

}

MyCamera::~MyCamera()
{
    switch (cam_state) {
    case CAM_STATE::STREAMON:
        ::ioctl(fd, VIDIOC_STREAMOFF, &buf_type);
    case CAM_STATE::INITED:
        close(fd);
    case CAM_STATE::OPENED:
        close(fd);
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
        return 1;
    }
    fd = ::open(dev_name.toUtf8().constData(), O_RDWR);
    if (fd < 0) {
        emit errored("Failed to open device");
        return 1;
    }
    int ret = ::ioctl(fd, VIDIOC_QUERYCAP, &cam_caps);
    if (ret == -1) {
        emit errored("Failed to query cam cap");
        return 1;
    }

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
