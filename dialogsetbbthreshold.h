#ifndef DIALOGSETBBTHRESHOLD_H
#define DIALOGSETBBTHRESHOLD_H

#include <QDialog>

QT_BEGIN_NAMESPACE
namespace Ui { class DialogSetBBThreshold; }
QT_END_NAMESPACE

class DialogSetBBThreshold : public QDialog
{
    Q_OBJECT
public:
    DialogSetBBThreshold(QWidget * parent = nullptr);
    ~DialogSetBBThreshold();
    void setCurrentThreshold(int t);

 signals:
    void valueChanged(int);

private slots:
    void updateText(int t);

private:
    Ui::DialogSetBBThreshold *ui;
};

#endif // DIALOGSETBBTHRESHOLD_H
