#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QMessageBox>
#include <QDateTime>
#include <QProcessEnvironment>
#include <QDir>
#include <opencv2/core/hal/interface.h>
#include <opencv2/imgproc.hpp>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , mycam(nullptr)
    , actCaptureImage("Capture image")
    , actToggleInvertColor("Toggle invert color mode")
    , actToggleScale("Toggle scale frame")
    , actToggleBB("Toggle blackboard mode")
    , actSetBBThreshold("Adjust BB Threshold")
    , actSelectCamera("Change camera")
    , actExit("Exit")
{
    ui->setupUi(this);

    dialog = new DialogSelectCamera(this);
    QObject::connect(dialog, SIGNAL(accepted()), this, SLOT(setCamera()));

    dialogSetBBT = new DialogSetBBThreshold(this);
    dialogSetBBT->setCurrentThreshold(ui->viewfinder->getBBThreshold());
    QObject::connect(dialogSetBBT, SIGNAL(valueChanged(int)),
                     this, SLOT(setBBThreshold(int)));

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

    // action: toggle blackboard
    actToggleBB.setShortcut(QKeySequence(Qt::CTRL + Qt::Key_B));
    addAction(&actToggleBB);
    QObject::connect(&actToggleBB, SIGNAL(triggered(bool)),
                     ui->viewfinder, SLOT(toggleBlackboard(bool)));

    // action: set blackboard threshold
    actSetBBThreshold.setShortcut(QKeySequence(Qt::CTRL + Qt::Key_T));
    addAction(&actSetBBThreshold);
    QObject::connect(&actSetBBThreshold, SIGNAL(triggered(bool)),
                     this, SLOT(setBBThreshold(bool)));


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

    timer.setInterval(5000);
    QObject::connect(&timer, SIGNAL(timeout()), this, SLOT(grabFrame()));
}

MainWindow::~MainWindow()
{
    delete dialog;
    delete dialogSetBBT;
    delete ui;

    if (mycam != nullptr) {
        delete mycam;
        mycam = nullptr;
    }
}

void MainWindow::selectCamera(bool checked)
{
    // Each time this dialog is launched,
    // we should query the latest camera list on system.
    cameraInfoList = QCameraInfo::availableCameras();
    for (QCameraInfo ci : cameraInfoList) {
        qDebug() << "Found device: " << ci.deviceName();
    }

    dialog->setCameraDevices(cameraInfoList);
    dialog->open();
}

void MainWindow::setBBThreshold(bool checked)
{
    dialogSetBBT->setCurrentThreshold(ui->viewfinder->getBBThreshold());
    dialogSetBBT->open();
}

void MainWindow::setCamera()
{
    int index = dialog->getIndex();
    qDebug() << "Index: " << index;
    if (index < 1) {
        return;
    }
    timer.stop();

    if (mycam != nullptr) {
        qDebug() << "delete old camera";
        delete mycam;
        mycam = nullptr;
    }
    QString devname = cameraInfoList.at(index - 1).deviceName();
    mycam = new MyCamera(devname);
    QObject::connect(mycam, SIGNAL(errored(const QString &)),
                     this, SLOT(onError(const QString&)));
    mycam->open();
    qDebug() << mycam->isCaptureSupported();
    qDebug() << mycam->isStreamSupported();
    QList<FrameFSI> ffsis = mycam->supportedResolutions();
    FrameFSI maxframe = ffsis.at(0);
    for (FrameFSI ffsi : ffsis) {
        if (ffsi.width > maxframe.width) {
            maxframe = ffsi;
        }
    }
    qDebug() << "max frame:" << maxframe.width
             << maxframe.height << maxframe.formatName;
    mycam->setFrameFSI(maxframe);
    mycam->turnOn();
    timer.start();
}

void MainWindow::wheelEvent(QWheelEvent *we)
{
    we->accept();
}

void MainWindow::onExit(bool checked)
{
    if (QMessageBox::Ok == QMessageBox::information(this, "Exit",
            "Do you really want to exit?", QMessageBox::Ok, QMessageBox::Cancel)) {
        qDebug() << "quit program";
       QApplication::quit();
    }
}

void MainWindow::onError(const QString &msg)
{
    QMessageBox::warning(this, "Camera device error",
            msg,
            QMessageBox::Ok);
}

void MainWindow::saveImage(bool checked)
{
    if (source_image.isNull()) {
        QMessageBox::warning(this, "Failed to save image",
            "Image data is null",
            QMessageBox::Ok);
        return;
    }
    QProcessEnvironment pe = QProcessEnvironment::systemEnvironment();
    QString homeEnv = pe.value("HOME", "./");
    QDir homeDir(homeEnv);
    QDateTime now = QDateTime::currentDateTime();
    QString nowString = now.toString("yyyy-MM-dd_hh-mm-ss");
    QString filename = homeDir.absoluteFilePath(nowString + ".jpg");
    qDebug() << "file name:" << filename;
    source_image.save(filename);
    QMessageBox::information(this, "Capture & save image",
        "Image file saved to:\n" + filename,
        QMessageBox::Ok);
}

void MainWindow::setBBThreshold(int t)
{
    ui->viewfinder->setBBThreshold(t);
}

void MainWindow::grabFrame()
{
    if (mycam == nullptr) {
        qDebug() << "No camera device configured";
        return;
    }

    QByteArray data = mycam->capture();
    qDebug() << "data size:" << data.size();

    ui->viewfinder->setFrame(source_image);
    ui->viewfinder->update();
}
