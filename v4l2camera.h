#ifndef V4L2CAMERA_H
#define V4L2CAMERA_H

#include <QObject>
#include<QString>
#include <QList>
#include <QPixmap>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <linux/videodev2.h>

// frame Format Size Interval
class FrameFSI{
public:
    QString formatName;
    unsigned int formatInt;
    unsigned int width;
    unsigned int height;
    unsigned int numerator;
    unsigned int denominator;
};

class V4L2Camera : public QObject
{
    Q_OBJECT
public:
    enum CAM_STATE {
        INVALID, // initial state
        OPENED, // device file is opened and some capbilities may also queried
        INITED, // some settings have been set and memory may also allocated
        STREAMON, // v4l2 device stream on is set
        CLOSED, // device file is closed
        ERRORED,
    };

    struct MyBuffer {
        void *start;
        size_t length;
    };

    explicit V4L2Camera(QString device, QObject *parent = nullptr);
    ~V4L2Camera();
    int open();
    QString driverName();
    int driverVersion();
    QString bufInfo();
    QString cardInfo();
    int capability();
    bool isCaptureSupported();
    bool isStreamSupported();
    QList<FrameFSI> supportedResolutions();
    CAM_STATE currentState();
    void setFrameFSI(FrameFSI &ffsi);
    void turnOn();
    QPixmap capture();

signals:
    void errored(const QString &msg);

private:
    QString dev_name;
    int fd;
    CAM_STATE cam_state;
    v4l2_buf_type buf_type;
    struct v4l2_capability cam_caps;
    struct v4l2_format defaultformat;
    QList<FrameFSI> framefsi;
    QList<struct MyBuffer> mybuffers;
};

#endif // V4L2CAMERA_H
