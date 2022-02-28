#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QMessageBox>
#include <QDateTime>
#include <QProcessEnvironment>
#include <QDir>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , mycam(nullptr)
{
    ui->setupUi(this);
    setupContextmenu();

    dialog = new DialogSelectCamera(this);
    QObject::connect(dialog, SIGNAL(accepted()), this, SLOT(setCamera()));

    dialogSetBBT = new DialogSetBBThreshold(this);
    dialogSetBBT->setCurrentThreshold(ui->viewfinder->getBBThreshold());
    QObject::connect(dialogSetBBT, SIGNAL(valueChanged(int)),
                     this, SLOT(setBBThreshold(int)));


    timer.setInterval(40);
    QObject::connect(&timer, SIGNAL(timeout()), this, SLOT(updateFrame()));
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

void MainWindow::setupContextmenu()
{
    QAction *a;
    // action: capture image
    a = contextmenu.addAction("Capture image",
                          this, &MainWindow::saveImage,
                          QKeySequence(Qt::CTRL + Qt::Key_Space));
    addAction(a);

    QAction *a1 = new QAction();
    a1->setSeparator(true);
    contextmenu.addAction(a1);

    // action: toggle invert color
    a = contextmenu.addAction("Toggle invert color mode",
                          ui->viewfinder, &VideoPreview::toggleInvertColor,
                          QKeySequence(Qt::CTRL + Qt::Key_I));
    addAction(a);

    // action: toggle scale previewing image
    a = contextmenu.addAction("Toggle resize frame",
                          ui->viewfinder, &VideoPreview::toggleResize,
                          QKeySequence(Qt::CTRL + Qt::Key_S));
    addAction(a);

    // add blackboard submenu
    submenuBB.setTitle("Blackboard mode");
    contextmenu.addMenu(&submenuBB);

    // action: set blackboard threshold
    a = contextmenu.addAction("Adjust BB Threshold",
                          this, &MainWindow::adjustBBThreshold,
                          QKeySequence(Qt::CTRL + Qt::Key_T));
    addAction(a);


    QAction *a2 = new QAction();
    a2->setSeparator(true);
    contextmenu.addAction(a2);

    // action: select camera
    a = contextmenu.addAction("Change camera",
                          this, &MainWindow::selectCamera,
                          QKeySequence(Qt::CTRL + Qt::Key_N));
    addAction(a);

    QAction *a3 = new QAction();
    a3->setSeparator(true);
    contextmenu.addAction(a3);

    a = contextmenu.addAction("Toggle fullscreen",
                          this, &MainWindow::toggleFullScreen,
                          QKeySequence(Qt::CTRL + Qt::Key_F));
    addAction(a);

    QAction *a4 = new QAction();
    a4->setSeparator(true);
    contextmenu.addAction(a4);

    // action: exit
    a = contextmenu.addAction("Exit",
                          this, &MainWindow::onExit,
                          QKeySequence(Qt::CTRL + Qt::Key_Q));
    addAction(a);

    // blackboard submenu
    // action: toggle blackboard mode
    a = submenuBB.addAction("Toggle blackboard mode",
                        ui->viewfinder, &VideoPreview::toggleBB,
                        QKeySequence(Qt::CTRL + Qt::Key_B));
    addAction(a);

    // action: toggle curve blackboard
    a = submenuBB.addAction("Toggle curve blackboard",
                        ui->viewfinder, &VideoPreview::toggleCurveBB,
                        QKeySequence(Qt::CTRL + Qt::Key_C));
    addAction(a);

}

void MainWindow::contextMenuEvent(QContextMenuEvent *event)
{
    contextmenu.exec(QCursor::pos());
    event->accept();
}

void MainWindow::selectCamera()
{
    // Each time this dialog is launched,
    // we should query the latest camera list on system.
    cameraInfoList = QCameraInfo::availableCameras();
    for (QCameraInfo ci : cameraInfoList) {
        qDebug() << "found device:" << ci.deviceName();
    }

    dialog->setCameraDevices(cameraInfoList);
    dialog->open();
}

void MainWindow::adjustBBThreshold()
{
    dialogSetBBT->setCurrentThreshold(ui->viewfinder->getBBThreshold());
    dialogSetBBT->open();
}

void MainWindow::setCamera()
{
    int index = dialog->getIndex();

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
    qDebug() << "use device:" << devname;
    mycam = new V4L2Camera(devname);
    QObject::connect(mycam, SIGNAL(errored(const QString &)),
                     this, SLOT(onError(const QString&)));
    mycam->open();
    if (mycam->currentState() != V4L2Camera::CAM_STATE::OPENED) {
        qDebug() << "failed to open camera device";
        return;
    }

    QList<FrameFSI> ffsis = mycam->supportedResolutions();
    FrameFSI maxframe = ffsis.at(0);
    for (FrameFSI ffsi : ffsis) {
        if (ffsi.width > maxframe.width) {
            maxframe = ffsi;
        }
    }
    qDebug() << "max frame size:" << maxframe.width
             << maxframe.height << maxframe.formatName;
    mycam->setFrameFSI(maxframe);
    mycam->turnOn();
    int frameinterval = maxframe.numerator * 1000 / maxframe.denominator;
    qDebug() << "frame rate:" << (1000 / frameinterval);
    timer.setInterval(frameinterval);
    timer.start();
}

void MainWindow::onExit()
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

void MainWindow::saveImage()
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

void MainWindow::updateFrame()
{
    if (mycam == nullptr) {
        qDebug() << "No camera device configured";
        return;
    }

    QPixmap pixmap = mycam->capture();
    source_image = pixmap.toImage();
    ui->viewfinder->setFrame(source_image);
    ui->viewfinder->update();
}

void MainWindow::toggleFullScreen()
{
    setWindowState(windowState() ^ Qt::WindowFullScreen);

}
