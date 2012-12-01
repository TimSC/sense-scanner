#ifndef VIDEOWIDGET_H
#define VIDEOWIDGET_H

#include <QtGui/QWidget>
#include <QtCore/QtCore>
#include <QtGui/QtGui>
#include <QtGui/QGraphicsScene>
#include <vector>
#include "mediabuffer.h"

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

class MouseGraphicsScene : public QGraphicsScene
{
public:
    MouseGraphicsScene(QWidget *parent);
    virtual ~MouseGraphicsScene();

    void mouseMoveEvent(QGraphicsSceneMouseEvent *mouseEvent);
    void mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent);
    void mouseReleaseEvent (QGraphicsSceneMouseEvent *mouseEvent);

    void SetSceneControl(class SimpleScene *sceneControlIn);

protected:
    class SimpleScene *sceneControl;
};

class SimpleScene : public QObject
{
    Q_OBJECT

public:
    SimpleScene(QWidget *parent);
    virtual ~SimpleScene();

    void VideoImageChanged(QImage &fr);

    void mouseMoveEvent(QGraphicsSceneMouseEvent *mouseEvent);
    void mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent);
    void mouseReleaseEvent (QGraphicsSceneMouseEvent *mouseEvent);
    int NearestPoint(float x, float y);
    void Redraw();

    QWidget *ControlsFactory(QWidget *parent);

    QSharedPointer<QGraphicsPixmapItem> item;
    QSharedPointer<MouseGraphicsScene> scene;
    std::vector<std::vector<float> > pos;
    QImage img;

public slots:
    void MovePressed();


protected:
    int activePoint;
    unsigned int imgHeight, imgWidth;
    float markerSize;
    int leftDrag;
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
    void TimerUpdate();
    void AsyncFrameReceived(QImage& fr, unsigned long long timestamp);
    void SetSceneControl(QSharedPointer<SimpleScene> sceneIn);

protected:
    void SetVisibleAtTime(long long unsigned ti);

    QSharedPointer<SimpleScene> sceneControl;
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
