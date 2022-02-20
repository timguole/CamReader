#ifndef VIDEOPREVIEW_H
#define VIDEOPREVIEW_H

#include <QWidget>

class VideoPreview : public QWidget
{
    Q_OBJECT
public:
    explicit VideoPreview(QWidget *parent = nullptr);
    void paintEvent(QPaintEvent *event);
    int getBBThreshold();
    void setBBThreshold(int t);
    void setFrame(QImage &i);

public slots:
    void toggleInvertColor(bool);
    void toggleScale(bool);
    void toggleBlackboard(bool);

private:
    void blackboardRgba(QImage &image);
    void blackboardRgb(QImage &image);

    QImage image;
    volatile bool isInvertColor;
    volatile bool isScaleImage;
    volatile bool isBloackBoard;
    volatile int blackboardThreshold;
};

#endif // VIDEOPREVIEW_H
