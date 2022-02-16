#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "ui_dialogSelectCamera.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , camera(nullptr)
    , vs(new VideoSurface)
    , dialog(nullptr)
    , camerafocus(nullptr)
{
    ui->setupUi(this);
    ui->viewfinder->setVideoSurface(vs);
    QObject::connect(vs, SIGNAL(newFrame()), ui->viewfinder, SLOT(update()));
}

MainWindow::~MainWindow()
{
    delete ui;

    if (dialog != nullptr) {
        delete dialog;
    }
    if (camera != nullptr) {
        camera->stop();
        camera->unload();
        delete camera;
    }
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
    camera->setViewfinder(vs);
    camera->load();
    camera->start();
}

void MainWindow::onCameraStateChanged(QCamera::State state)
{
    if (state == QCamera::LoadedState) {
        camerafocus = camera->focus();
        qDebug() << camerafocus->maximumDigitalZoom();
        qDebug() << camerafocus->maximumOpticalZoom();
    }
}

void MainWindow::wheelEvent(QWheelEvent *we)
{
    QPoint rotation = we->angleDelta();
    qreal zoom;
    qreal currentZoom = camerafocus->digitalZoom();

    zoom = rotation.y() < 0 ? currentZoom - 1.0 : currentZoom + 1.0;
    if ((zoom >= 1.0)
            && (zoom <= camerafocus->maximumDigitalZoom())) {
        camerafocus->zoomTo(1.0, zoom);
    }
    qDebug() << "zoom: " << camerafocus->digitalZoom();
}
