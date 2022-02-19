#include "dialogsetbbthreshold.h"
#include "ui_dialogSetBBThreshold.h"

DialogSetBBThreshold::DialogSetBBThreshold(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::DialogSetBBThreshold)
{
    ui->setupUi(this);
    ui->bbThreshold->setMaximum(255);
    QObject::connect(ui->bbThreshold, &QSlider::valueChanged,
                     this, &DialogSetBBThreshold::valueChanged);
    QObject::connect(ui->bbThreshold, SIGNAL(valueChanged(int)),
                     this, SLOT(updateText(int)));
}

DialogSetBBThreshold::~DialogSetBBThreshold()
{
    delete ui;
}

void DialogSetBBThreshold::setCurrentThreshold(int t)
{
    ui->bbThreshold->setValue(t);
}

void DialogSetBBThreshold::updateText(int t)
{
    ui->currentValue->setText((QString::number(t)));
}
