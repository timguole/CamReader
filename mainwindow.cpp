#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "ui_dialogSelectCamera.h"

#include <QMessageBox>
#include <QDateTime>
#include <QProcessEnvironment>
#include <QDir>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , camera(nullptr)
    , imageCapture(nullptr)
    , vs(new VideoSurface)
    , dialog(nullptr)
    , camerafocus(nullptr)
    , actCaptureImage("Capture image")
    , actToggleInvertColor("Toggle invert color")
    , actToggleScale("Toggle scale frame")
    , actExit("Exit")
{
    ui->setupUi(this);
    ui->viewfinder->setVideoSurface(vs);
    QObject::connect(vs, SIGNAL(newFrame()), ui->viewfinder, SLOT(update()));

    addAction(&actCaptureImage);
    addAction(&actToggleInvertColor);
    addAction(&actToggleScale);
    addAction(&actExit);
    setContextMenuPolicy(Qt::ActionsContextMenu);

    QObject::connect(&actCaptureImage, SIGNAL(triggered(bool)),
                     this, SLOT(saveImage(bool)));
    QObject::connect(&actToggleInvertColor, SIGNAL(triggered(bool)),
                     ui->viewfinder, SLOT(toggleInvertColor(bool)));
    QObject::connect(&actToggleScale, SIGNAL(triggered(bool)),
                     ui->viewfinder, SLOT(toggleScale(bool)));
    QObject::connect(&actExit, SIGNAL(triggered(bool)),
                     this, SLOT(onExit(bool)));
}

MainWindow::~MainWindow()
{
    delete ui;

    if (dialog != nullptr) {
        delete dialog;
    }
    if (imageCapture != nullptr) {
        delete imageCapture;
    }
    if (camera != nullptr) {
        camera->stop();
        camera->unload();
        delete camera;
    }
    delete vs;
}

void MainWindow::selectCamera()
{
    cameraInfoList = QCameraInfo::availableCameras();
    for (QCameraInfo ci : cameraInfoList) {
        qDebug() << "Found device: " << ci.deviceName();
    }

    dialog = new DialogSelectCamera(this);
    dialog->setCameraDevices(cameraInfoList);
    QObject::connect(dialog, SIGNAL(accepted()), this, SLOT(setCamera()));
    dialog->open();
}

void MainWindow::setCamera()
{
    int index = dialog->getIndex();
    qDebug() << "Index: " << index;
    if (index < 1) {
        return;
    }

    if (camera != nullptr) {
        camera->stop();
        camera->unload();
        delete camera;
    }
    camera = new QCamera(cameraInfoList.at(index - 1));
    QObject::connect(camera, SIGNAL(stateChanged(QCamera::State)),
                     this, SLOT(onCameraStateChanged(QCamera::State)));

    imageCapture = new QCameraImageCapture(camera);
    camera->setCaptureMode(QCamera::CaptureStillImage);
    camera->setViewfinder(vs);
    camera->load();
    camera->start();
}

void MainWindow::onCameraStateChanged(QCamera::State state)
{
    if (state == QCamera::LoadedState) {
        camerafocus = camera->focus();
        qDebug() << "max digital zoom: " << camerafocus->maximumDigitalZoom();
        qDebug() << "max optical zoom: " << camerafocus->maximumOpticalZoom();
    }
}

void MainWindow::wheelEvent(QWheelEvent *we)
{
    QPoint rotation = we->angleDelta();
    qreal zoomDigital;
    qreal zoomOptical;
    qreal currentDigitalZoom = camerafocus->digitalZoom();
    qreal currentOpticalZoom = camerafocus->opticalZoom();


    zoomDigital = rotation.y() < 0
            ? currentDigitalZoom - 0.25 : currentDigitalZoom + 0.25;
    zoomOptical = rotation.y() < 0
            ? currentOpticalZoom - 0.25 : currentOpticalZoom + 0.25;

    // We prefer optical zoom to digital zoom
    if ((zoomOptical >= 1.0)
            && (zoomOptical <= camerafocus->maximumOpticalZoom())) {
        camerafocus->zoomTo(zoomOptical, 1.0);
    } else if ((zoomDigital >= 1.0)
            && (zoomDigital <= camerafocus->maximumDigitalZoom())) {
        camerafocus->zoomTo(1.0, zoomDigital);
    }
    qDebug() << "optical zoom: " << camerafocus->opticalZoom();
    qDebug() << "digital zoom: " << camerafocus->digitalZoom();

    we->accept();
}

void MainWindow::onExit(bool checked)
{
    qDebug() << "action checked: " << checked;
    if (QMessageBox::Ok == QMessageBox::information(this, "Exit",
            "Do you really want to exit?", QMessageBox::Ok, QMessageBox::Cancel)) {
        qDebug() << "quit program";
       QApplication::quit();
    }
}

void MainWindow::saveImage(bool checked)
{
    qDebug() << "action checked: " << checked;
    QProcessEnvironment pe = QProcessEnvironment::systemEnvironment();
    QString homeEnv = pe.value("HOME", "./");
    QDir homeDir(homeEnv);
    QDateTime now = QDateTime::currentDateTime();
    QString nowString = now.toString("yyyy-MM-dd_hh-mm-ss");
    QString filename = homeDir.absoluteFilePath(nowString + ".jpg");
    imageCapture->capture(filename);
}
