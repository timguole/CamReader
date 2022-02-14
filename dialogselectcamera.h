#ifndef DIALOGSELECTCAMERA_H
#define DIALOGSELECTCAMERA_H

#include <QDialog>
#include <QList>
#include <QCameraInfo>

QT_BEGIN_NAMESPACE
namespace Ui { class DialogSelectCamera; }
QT_END_NAMESPACE

class DialogSelectCamera : public QDialog
{
    Q_OBJECT

public:
    DialogSelectCamera(QWidget *parent = nullptr);
    ~DialogSelectCamera();
    void setCameraDevices(QList<QCameraInfo> &cameraInfo);
    int getIndex();

 public slots:
    void onSelected(int i);

private:
    Ui::DialogSelectCamera *ui;
    int index;
};
#endif // DIALOGSELECTCAMERA_H
