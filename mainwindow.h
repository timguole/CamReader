#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "dialogselectcamera.h"
#include "dialogsetbbthreshold.h"
#include "videosurface.h"

#include <QMainWindow>
#include <QCamera>
#include <QCameraInfo>
#include <QCameraViewfinder>
#include <QCameraImageCapture>
#include <QList>
#include <QAction>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    void selectCamera(bool checked = false);
    void setBBThreshold(bool checked = false);
    void setCamera();
    void onCameraStateChanged(QCamera::State state);
    void wheelEvent(QWheelEvent *we);
    void onExit(bool checked);
    void saveImage(bool checked);
    void setBBThreshold(int t);

private:
    Ui::MainWindow *ui;
    QList<QCameraInfo> cameraInfoList;
    QCamera *camera;
    QCameraImageCapture *imageCapture;
    VideoSurface *vs;
    DialogSelectCamera *dialog;
    QCameraFocus *camerafocus;
    DialogSetBBThreshold *dialogSetBBT;

    QAction actCaptureImage;
    QAction actToggleInvertColor;
    QAction actToggleScale;
    QAction actToggleBB;
    QAction actSetBBThreshold;
    QAction actSelectCamera;
    QAction actExit;
};
#endif // MAINWINDOW_H
