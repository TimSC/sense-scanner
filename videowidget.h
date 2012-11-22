#ifndef VIDEOWIDGET_H
#define VIDEOWIDGET_H

#include <QWidget>
#include <QtCore>
#include <QtGui>
#include <QGraphicsScene>
#include "imagesequence.h"

namespace Ui {
class VideoWidget;
}

class ZoomGraphicsView : public QGraphicsView
{
    Q_OBJECT
public:
    explicit ZoomGraphicsView(QWidget *parent = 0);
    virtual ~ZoomGraphicsView();

public slots:
    void wheelEvent(QWheelEvent* event);

protected:
    float scaleFactor;
};

class VideoWidget : public QWidget
{
    Q_OBJECT
    
public:
    explicit VideoWidget(QWidget *parent = 0);
    ~VideoWidget();
public slots:
    void SliderMoved(int newValue);
    void Stop();
    void Pause();
    void Play();
    void TimerUpdate();

protected:
    void SetVisibleAtTime(long long unsigned ti);

    QSharedPointer<QGraphicsScene> scene;
    QSharedPointer<QGraphicsPixmapItem> item;
    QSharedPointer<ImageSequence> seq;
    QSharedPointer<QTimer> timer;

    QTime playPressedTime;
    int playActive;
    long long unsigned currentTime, playVidStartPos;
private:
    Ui::VideoWidget *ui;
};

#endif // VIDEOWIDGET_H
