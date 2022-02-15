#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "dialogselectcamera.h"
#include "videosurface.h"

#include <QMainWindow>
#include <QCamera>
#include <QCameraInfo>
#include <QCameraViewfinder>
#include <QList>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void selectCamera();

public slots:
    void setCamera();

private:
    Ui::MainWindow *ui;
    QList<QCameraInfo> cameraInfoList;
    QCamera *camera;
    VideoSurface *vs;
    DialogSelectCamera *dialog;
};
#endif // MAINWINDOW_H
