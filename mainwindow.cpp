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
    , camerafocus(nullptr)
    , actCaptureImage("Capture image")
    , actToggleInvertColor("Toggle invert color")
    , actToggleScale("Toggle scale frame")
    , actSelectCamera("Change camera")
    , actExit("Exit")
{
    ui->setupUi(this);
    ui->viewfinder->setVideoSurface(vs);
    QObject::connect(vs, SIGNAL(newFrame()), ui->viewfinder, SLOT(update()));

    dialog = new DialogSelectCamera(this);
    QObject::connect(dialog, SIGNAL(accepted()), this, SLOT(setCamera()));

    // action: capture image
    actCaptureImage.setShortcut(QKeySequence(Qt::CTRL + Qt::Key_Space));
    addAction(&actCaptureImage);
    QObject::connect(&actCaptureImage, SIGNAL(triggered(bool)),
                     this, SLOT(saveImage(bool)));

    QAction *a1 = new QAction();
    a1->setSeparator(true);
    addAction(a1);

    // action: toggle invert color
    actToggleInvertColor.setShortcut(QKeySequence(Qt::CTRL + Qt::Key_I));
    addAction(&actToggleInvertColor);
    QObject::connect(&actToggleInvertColor, SIGNAL(triggered(bool)),
                     ui->viewfinder, SLOT(toggleInvertColor(bool)));

    // action: toggle scale previewing image
    actToggleScale.setShortcut(QKeySequence(Qt::CTRL + Qt::Key_S));
    addAction(&actToggleScale);
    QObject::connect(&actToggleScale, SIGNAL(triggered(bool)),
                     ui->viewfinder, SLOT(toggleScale(bool)));

    QAction *a2 = new QAction();
    a2->setSeparator(true);
    addAction(a2);

    // action: select camera
    actSelectCamera.setShortcut(QKeySequence(Qt::CTRL + Qt::Key_N));
    addAction(&actSelectCamera);
    QObject::connect(&actSelectCamera, SIGNAL(triggered(bool)),
                     this, SLOT(selectCamera(bool)));

    QAction *a3 = new QAction();
    a3->setSeparator(true);
    addAction(a3);

    // action: exit
    actExit.setShortcut(QKeySequence(Qt::CTRL + Qt::Key_Q));
    addAction(&actExit);
    QObject::connect(&actExit, SIGNAL(triggered(bool)),
                     this, SLOT(onExit(bool)));

    setContextMenuPolicy(Qt::ActionsContextMenu);
}

MainWindow::~MainWindow()
{
    delete dialog;
    delete ui;
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

void MainWindow::selectCamera(bool checked)
{
    qDebug() << "action checked: " << checked;

    // Each time this dialog is launched,
    // we should query the latest camera list on system.
    cameraInfoList = QCameraInfo::availableCameras();
    for (QCameraInfo ci : cameraInfoList) {
        qDebug() << "Found device: " << ci.deviceName();
    }

    dialog->setCameraDevices(cameraInfoList);
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
        qDebug() << "delete old camera";
        camera->stop();
        camera->unload();
        delete camera;
    }
    camera = new QCamera(cameraInfoList.at(index - 1));
    QObject::connect(camera, SIGNAL(stateChanged(QCamera::State)),
                     this, SLOT(onCameraStateChanged(QCamera::State)));

    if (imageCapture != nullptr) {
        qDebug() << "delete old imagecapture";
        delete imageCapture;
    }
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

    QMessageBox::information(this, "Capture & save image",
        "Image file saved to:\n" + filename,
        QMessageBox::Ok);
}
