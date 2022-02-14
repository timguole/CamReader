#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "ui_dialogSelectCamera.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , camera(nullptr)
    , dialog(nullptr)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
    if (dialog != nullptr) {
        delete dialog;
    }
    if (camera != nullptr) {
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
    QString deviceName = cameraInfoList.at(index - 1).deviceName();
    qDebug() << "Selected device: " << deviceName;
    camera = new QCamera(cameraInfoList.at(index - 1));
    QCameraViewfinderSettings viewfinderSettings;
    viewfinderSettings.setResolution(640, 480);
    viewfinderSettings.setMinimumFrameRate(15.0);
    viewfinderSettings.setMaximumFrameRate(30.0);

    camera->load();
    qDebug() << "loaded";
    QList<QSize> resolutions = camera->supportedViewfinderResolutions(viewfinderSettings);
    for (QSize r : resolutions) {
        qDebug() << r.width() << " x " << r.height();
    }
}
