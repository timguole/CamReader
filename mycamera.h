#ifndef MYCAMERA_H
#define MYCAMERA_H

#include <QObject>
#include<QString>
#include <QList>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <linux/videodev2.h>

// frame Format Size Interval
class FrameFSI{
public:
    QString formatName;
    int formatInt;
    int width;
    int height;
    int numerator;
    int denominator;
};

class MyCamera : public QObject
{
    Q_OBJECT
public:
    enum CAM_STATE {
        INVALID, // initial state
        OPENED, // device file is opened and some capbilities may also queried
        INITED, // some settings have been set and memory may also allocated
        STREAMON, // v4l2 device stream on is set
        CLOSED, // device file is closed
    };

    explicit MyCamera(QString device, QObject *parent = nullptr);
    ~MyCamera();
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

signals:
    void errored(const QString &msg);

private:
    QString dev_name;
    int fd;
    CAM_STATE cam_state;
    v4l2_buf_type buf_type;
    struct v4l2_capability cam_caps;
    QList<FrameFSI> framefsi;
};

#endif // MYCAMERA_H
