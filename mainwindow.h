#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "dialogselectcamera.h"
#include "dialogsetbbthreshold.h"
#include "mycamera.h"

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
    void selectCamera(bool checked = false);
    void setBBThreshold(bool checked = false);
    void setCamera();
    void onExit(bool checked);
    void onError(const QString &msg);
    void saveImage(bool checked);
    void setBBThreshold(int t);
    void grabFrame();

protected:
    void contextMenuEvent(QContextMenuEvent *event);

private:
    void setupContextmenu();

    Ui::MainWindow *ui;
    QList<QCameraInfo> cameraInfoList;
    MyCamera *mycam;
    QImage source_image;
    DialogSelectCamera *dialog;
    DialogSetBBThreshold *dialogSetBBT;
    QTimer timer;

    QMenu contextmenu;
    QAction actCaptureImage;
    QAction actToggleInvertColor;
    QAction actToggleScale;
    QAction actSetBBThreshold;
    QAction actSelectCamera;
    QAction actExit;

    QMenu submenuBB; // sub menu for blackboard mode
    QAction actToggleBB;
    QAction actToggleCurveBB;
};
#endif // MAINWINDOW_H
