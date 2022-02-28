#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "dialogselectcamera.h"
#include "dialogsetbbthreshold.h"
#include "v4l2camera.h"

#include <QMainWindow>
#include <QCameraInfo>
#include <QList>
#include <QAction>
#include <QMenu>
#include <QTimer>

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
    void selectCamera();
    void adjustBBThreshold();
    void setCamera();
    void onExit();
    void onError(const QString &msg);
    void saveImage();
    void setBBThreshold(int t);
    void updateFrame();

protected:
    void contextMenuEvent(QContextMenuEvent *event);

private slots:
    void toggleFullScreen();

private:
    void setupContextmenu();

    Ui::MainWindow *ui;
    QList<QCameraInfo> cameraInfoList;
    V4L2Camera *mycam;
    QImage source_image;
    DialogSelectCamera *dialog;
    DialogSetBBThreshold *dialogSetBBT;
    QTimer timer;
    QMenu contextmenu;
    QMenu submenuBB; // sub menu for blackboard mode
};
#endif // MAINWINDOW_H
