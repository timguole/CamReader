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
    , videocapture(nullptr)
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

    timer.setInterval(200);
    QObject::connect(&timer, SIGNAL(timeout()), this, SLOT(grabFrame()));
}

MainWindow::~MainWindow()
{
    delete dialog;
    delete dialogSetBBT;
    delete ui;

    if (videocapture != nullptr) {
        videocapture->release();
        delete videocapture;
        videocapture = nullptr;
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

    if (videocapture != nullptr) {
        qDebug() << "delete old camera";
        videocapture->release();
        delete videocapture;
        videocapture = nullptr;
    }
    QString devname = cameraInfoList.at(index - 1).deviceName();
    int cameraid = devname.at(devname.size() - 1).digitValue();
    qDebug() << "camera id:" << cameraid;
    videocapture = new cv::VideoCapture();
    videocapture->open(cameraid);
    if (!videocapture->isOpened()) {
        qDebug() << "failed to open videocapture";
        return;
    }
    double h = videocapture->get(cv::CAP_PROP_FRAME_HEIGHT);
    double w = videocapture->get(cv::CAP_PROP_FRAME_WIDTH);
    qDebug() << "default resolution: " << w << " " << h;

    //videocapture->set(cv::CAP_PROP_FRAME_WIDTH, 1920);
    //videocapture->set(cv::CAP_PROP_FRAME_HEIGHT, 1080);
    //videocapture->set(cv::CAP_PROP_FPS, 25);
    //videocapture->set(cv::CAP_PROP_FOURCC,
    //                  cv::VideoWriter::fourcc('M', 'J', 'P', 'G'));
    h = videocapture->get(cv::CAP_PROP_FRAME_HEIGHT);
    w = videocapture->get(cv::CAP_PROP_FRAME_WIDTH);
    qDebug() << "max resolution: " << w << " " << h;
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
    if (videocapture == nullptr) {
        return;
    }
    videocapture->read(source_frame);
    if (source_frame.empty()) {
        qDebug() << "source frame is empty";
        return;
    }
    if (source_frame.type() != CV_8UC3) {
        qDebug() << "mat type is not CV_8UC3";
        return;
    }
    cv::cvtColor(source_frame, rgb_frame, cv::COLOR_BGR2RGB);
    QImage i(rgb_frame.data,
             rgb_frame.cols,
             rgb_frame.rows,
             QImage::Format_RGB888);
    source_image = i;
    ui->viewfinder->setFrame(source_image);
    ui->viewfinder->update();
}
