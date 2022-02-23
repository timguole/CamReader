QT += core gui multimedia multimediawidgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    dialogselectcamera.cpp \
    dialogsetbbthreshold.cpp \
    main.cpp \
    mainwindow.cpp \
    mycamera.cpp \
    videopreview.cpp

HEADERS += \
    dialogselectcamera.h \
    dialogsetbbthreshold.h \
    mainwindow.h \
    mycamera.h \
    videopreview.h

LIBS += /usr/lib/x86_64-linux-gnu/libopencv_core.so.4.2.0 \
        /usr/lib/x86_64-linux-gnu/libopencv_videoio.so.4.2.0 \
        /usr/lib/x86_64-linux-gnu/libopencv_imgproc.so.4.2.0

FORMS += \
    dialogSelectCamera.ui \
    dialogSetBBThreshold.ui \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
