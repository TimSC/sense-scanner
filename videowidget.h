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
    /*!
    * A QGraphicsView with overridden methods to detect mouse
    * wheel events.
    */

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
    /*
    * VideoWidget handles the video controls and display based on
    * an AbstractMedia data source. The TrackingAnnotation provides
    * the GUI code for inside the video area.
    */

    Q_OBJECT   
public:
    explicit VideoWidget(QWidget *parent = 0);
    virtual ~VideoWidget();

public slots:
    void SetSource(AbstractMedia *src, QString finaIn);
    void SliderMoved(int newValue);
    void Pause();
    void Play();
    void SeekBack();
    void SeekForward();
    void TimerUpdate();
    void AsyncFrameReceived(QImage& fr, unsigned long long startTimestamp,
                            unsigned long long endTimestamp,
                            unsigned long long requestTimestamp);
    void SetSceneControl(TrackingSceneController *sceneControlIn);
    void SetAnnotationTrack(QUuid srcUuid);
    void FitToWindow();
    void TimeChanged(QTime time);

protected:
    void SetVisibleAtTime(long long unsigned ti);

    TrackingSceneController *sceneControl;
    AbstractMedia *seq;
    QSharedPointer<QTimer> timer;

    QTime playPressedTime;
    int playActive;
    long long unsigned currentTime, playVidStartPos;
    long long unsigned mediaLength;
    int fitWindowToNextFrame;
    long long unsigned lastRequestedTime;
    QString fina;

private:
    Ui::VideoWidget *ui;
};

#endif // VIDEOWIDGET_H
