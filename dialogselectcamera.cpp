#include "dialogselectcamera.h"
#include "ui_dialogSelectCamera.h"

DialogSelectCamera::DialogSelectCamera(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::DialogSelectCamera)
    , index(0)
{
    ui->setupUi(this);

    // the first item is invalid value,
    // so the default value is always invalid.
    setResult(0);
    QObject::connect(ui->comboboxCameras, SIGNAL(currentIndexChanged(int)),
                     this, SLOT(onSelected(int)));
}

DialogSelectCamera::~DialogSelectCamera()
{
    delete ui;
}

void DialogSelectCamera::setCameraDevices(QList<QCameraInfo> &cameraInfo)
{
    ui->comboboxCameras->clear();
    ui->comboboxCameras->addItem("(none)");
    for (QCameraInfo ci : cameraInfo) {
        QString deviceName = ci.deviceName()
                + " (" + ci.description() + ")";
        ui->comboboxCameras->addItem(deviceName);
    }
}

void DialogSelectCamera::onSelected(int i)
{
    index = i;
}

int DialogSelectCamera::getIndex()
{
    return index;
}
