#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "dialogselectcamera.h"
#include "dialogsetbbthreshold.h"

#include <QMainWindow>
#include <QCameraInfo>
#include <QList>
#include <QAction>
#include <QTimer>
#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>
#include <opencv2/core/mat.hpp>
#include <opencv2/videoio.hpp>

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
    void wheelEvent(QWheelEvent *we);
    void onExit(bool checked);
    void saveImage(bool checked);
    void setBBThreshold(int t);
    void grabFrame();

private:
    Ui::MainWindow *ui;
    QList<QCameraInfo> cameraInfoList;
    cv::VideoCapture *videocapture;
    cv::Mat source_frame;
    cv::Mat rgb_frame;
    QImage source_image;
    DialogSelectCamera *dialog;
    DialogSetBBThreshold *dialogSetBBT;
    QTimer timer;

    QAction actCaptureImage;
    QAction actToggleInvertColor;
    QAction actToggleScale;
    QAction actToggleBB;
    QAction actSetBBThreshold;
    QAction actSelectCamera;
    QAction actExit;
};
#endif // MAINWINDOW_H
