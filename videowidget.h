#ifndef VIDEOWIDGET_H
#define VIDEOWIDGET_H

#include <QtGui/QWidget>
#include <QtCore/QtCore>
#include <QtGui/QtGui>
#include <QtGui/QGraphicsScene>
#include <vector>
#include "mediabuffer.h"
#include "scenecontroller.h"

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
    virtual ~VideoWidget();

public slots:
    void SetSource(AbstractMedia *src);
    void SliderMoved(int newValue);
    void Pause();
    void Play();
    void SeekBack();
    void SeekForward();
    void TimerUpdate();
    void AsyncFrameReceived(QImage& fr, unsigned long long timestamp);
    void SetSceneControl(QSharedPointer<SimpleSceneController> sceneIn);

protected:
    void SetVisibleAtTime(long long unsigned ti);

    QSharedPointer<SimpleSceneController> sceneControl;
    AbstractMedia *seq;
    QSharedPointer<QTimer> timer;

    QTime playPressedTime;
    int playActive;
    long long unsigned currentTime, playVidStartPos;
    long long unsigned mediaLength;
    unsigned int waitingForNumFrames;

private:
    Ui::VideoWidget *ui;
};

#endif // VIDEOWIDGET_H
