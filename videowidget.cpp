#include "videowidget.h"
#include "ui_videowidget.h"
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QObject>
#include <QtGui/QPushButton>
#include <iostream>
#include <assert.h>
#include <stdexcept>
#include <cstdlib>
#include <math.h>
using namespace std;

//Custom graphics view to catch mouse wheel

ZoomGraphicsView::ZoomGraphicsView(QWidget *parent) : QGraphicsView(parent)
{
    this->scaleFactor = 1.;

}

ZoomGraphicsView::~ZoomGraphicsView()
{


}

void ZoomGraphicsView::wheelEvent(QWheelEvent* event)
{
    if (event->delta() > 0)
    {
        this->scaleFactor = 1.2;
    }
    else
    {
        this->scaleFactor = 1./1.2;
    }
    this->scale(this->scaleFactor,this->scaleFactor);
}

//********************************************************************

//Custom graphics scene to catch mouse move and press

MouseGraphicsScene::MouseGraphicsScene(QWidget *parent) : QGraphicsScene(parent)
{
    this->sceneControl = NULL;
}

MouseGraphicsScene::~MouseGraphicsScene()
{

}

void MouseGraphicsScene::mouseMoveEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    if(this->sceneControl != NULL)
        this->sceneControl->mouseMoveEvent(mouseEvent);
}

void MouseGraphicsScene::mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    if(this->sceneControl != NULL)
        this->sceneControl->mousePressEvent(mouseEvent);
}

void MouseGraphicsScene::mouseReleaseEvent (QGraphicsSceneMouseEvent *mouseEvent)
{
    if(this->sceneControl != NULL)
        this->sceneControl->mouseReleaseEvent(mouseEvent);
}

void MouseGraphicsScene::SetSceneControl(SimpleScene *sceneControlIn)
{
    this->sceneControl = sceneControlIn;
}

//********************************************************************

SimpleScene::SimpleScene(QWidget *parent)
{
    this->scene = QSharedPointer<MouseGraphicsScene>(new MouseGraphicsScene(parent));
    this->scene->SetSceneControl(this);
    for(int i=0;i<50;i++)
    {
        vector<float> p;
        p.push_back(rand() % 500);
        p.push_back(rand() % 500);
        this->pos.push_back(p);
    }
    activePoint = -1;
    this->imgWidth = 0;
    this->imgHeight = 0;
    this->markerSize = 2.;
    this->leftDrag = 0;
}

SimpleScene::~SimpleScene()
{
    this->item = QSharedPointer<QGraphicsPixmapItem>(NULL);
    if(!this->scene.isNull())
    {
        this->scene->clear();
        this->scene->SetSceneControl(NULL);
        this->scene = QSharedPointer<MouseGraphicsScene>(NULL);
    }
}

void SimpleScene::VideoImageChanged(QImage &fr)
{
    this->img = fr;
    //this->item =
    //        QSharedPointer<QGraphicsPixmapItem>(new QGraphicsPixmapItem(QPixmap::fromImage(fr)));
    this->imgWidth = fr.width();
    this->imgHeight = fr.height();
    this->Redraw();
}

void SimpleScene::Redraw()
{

    this->scene->clear();
    if(this->imgWidth > 0 && this->imgHeight>0 && !this->img.isNull())
    {
        QGraphicsPixmapItem *tmp = new QGraphicsPixmapItem(QPixmap::fromImage(this->img));
        this->scene->addItem(tmp); //I love pointers
        this->scene->setSceneRect ( 0, 0, this->imgWidth, this->imgHeight );
    }

    QPen penRed(QColor(255,0,0));
    QBrush brushTransparent(QColor(0,0,0,0));
    QBrush brushRed(QColor(255,0,0));

    for(unsigned int i=0;i<this->pos.size();i++)
    {
        //cout << this->activePoint << endl;
        if(i!=this->activePoint)
            this->scene->addEllipse(this->pos[i][0]-this->markerSize/2,
                                    this->pos[i][1]-this->markerSize/2,
                                    this->markerSize, this->markerSize,
                                    penRed, brushRed);
        else
            this->scene->addEllipse(this->pos[i][0]-this->markerSize/2,
                                    this->pos[i][1]-this->markerSize/2,
                                    this->markerSize, this->markerSize,
                                    penRed, brushTransparent);
    }
}

void SimpleScene::mouseMoveEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    assert(mouseEvent);
    QPointF pos = mouseEvent->scenePos();
    cout << "mouseMoveEvent, " << pos.x() << "," << pos.y () << endl;
    if(this->activePoint >= 0 && this->leftDrag)
    {
        this->pos[this->activePoint][0] = pos.x();
        this->pos[this->activePoint][1] = pos.y();
        this->Redraw();
    }

}

void SimpleScene::mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    cout << "mousePressEvent" << endl;
    assert(mouseEvent);
    QPointF pos = mouseEvent->buttonDownScenePos(mouseEvent->button());
    int nearestPoint = this->NearestPoint(pos.x(), pos.y());
    this->activePoint = nearestPoint;
    this->Redraw();

    Qt::MouseButton button = mouseEvent->button();
    if(button==Qt::LeftButton)
        this->leftDrag = 1;
}

void SimpleScene::mouseReleaseEvent (QGraphicsSceneMouseEvent *mouseEvent)
{
    cout << "mouseReleaseEvent" << endl;
    assert(mouseEvent);

    Qt::MouseButton button = mouseEvent->button();
    if(button==Qt::LeftButton)
        this->leftDrag = 0;
}

int SimpleScene::NearestPoint(float x, float y)
{
    int best = -1;
    float bestDist = -1;
    for(unsigned int i=0;i<this->pos.size();i++)
    {
        float dx = this->pos[i][0] - x;
        float dy = this->pos[i][1] - y;
        float dist = pow(dx*dx + dy*dy, 0.5f);
        if(bestDist < 0. || dist < bestDist)
        {
            bestDist = dist;
            best = i;
        }
    }
    return best;
}

void SimpleScene::AddToolButtons(QLayout *layout)
{
    assert(layout);
    class QPushButton *button = new class QPushButton("One");
    layout->addWidget(button);
    class QPushButton *button2 = new class QPushButton("Two");
    layout->addWidget(button2);
    button->show();
    button2->show();
}

void SimpleScene::RemoveToolButtons(QLayout *layout)
{

}

//********************************************************************

VideoWidget::VideoWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::VideoWidget)
{
    ui->setupUi(this);
    this->playVidStartPos = 0;
    this->currentTime = 0;
    this->playActive = false;
    this->mediaLength = 0;
    this->waitingForNumFrames = 0;
    this->seq = NULL;
    //this->sceneControl = QSharedPointer<SimpleScene>(NULL);

    this->SetVisibleAtTime(0);

    //Start idle timer to update video when playing
    this->timer = QSharedPointer<QTimer>(new QTimer(this));
    QObject::connect(&(*this->timer),SIGNAL(timeout()), this, SLOT(TimerUpdate()));
    this->timer->start(10);
}

VideoWidget::~VideoWidget()
{
    delete ui;
}

void VideoWidget::SetSource(AbstractMedia *src)
{
    this->seq = src;
    this->mediaLength = 0;

    if(this->seq!=NULL) try
    {
        this->mediaLength = this->seq->Length();
    }
    catch (std::runtime_error &e)
    {
        cout << "Warning: could not determine length of video" << endl;
    }

    if(this->mediaLength > 0)
        this->ui->horizontalScrollBar->setRange(0, this->mediaLength);
    else
        this->ui->horizontalScrollBar->setRange(0, 1000);
    this->ui->horizontalScrollBar->setValue(0);
    this->SetVisibleAtTime(0);
}

void VideoWidget::SetVisibleAtTime(long long unsigned ti)
{
    //Check the sequence is valid
    if(this->seq == NULL) return;

    //Get image from sequence
    try
    {
        this->waitingForNumFrames ++;
        this->seq->RequestFrame(ti);
    }
    catch(std::runtime_error &err)
    {
        cout << "Warning: failed to update view to requested frame" << endl;
    }

}

void VideoWidget::SliderMoved(int newValue)
{
    if(this->waitingForNumFrames < 2)
    {
        this->SetVisibleAtTime(newValue);

    }
}

void VideoWidget::Pause()
{
    cout << "pause" << endl;

    //Ensure we have an accurate frame when stopping
    if(this->playActive)
    {
        this->playActive = false;

        int elapsedMs = this->playPressedTime.elapsed();
        long long unsigned calcCurrentTime = this->playVidStartPos + elapsedMs;

        //Check if we have reached the end of the video
        if(calcCurrentTime >= this->mediaLength)
            calcCurrentTime = this->mediaLength;

        //This automatically triggers the video frame refresh
        this->SetVisibleAtTime(calcCurrentTime);
    }

}

void VideoWidget::Play()
{
    cout << "play" << endl;
    this->playPressedTime.start();

    //If the video is at the end, start playing from the beginning
    if(this->currentTime < this->mediaLength - 1000)
        this->playVidStartPos = this->currentTime;
    else
        this->playVidStartPos = 0;
    playActive = true;
}

void FrameCallbackTest(QImage& fr, unsigned long long timestamp, void *raw)
{
    VideoWidget *widget = (VideoWidget *)raw;
    widget->AsyncFrameReceived(fr, timestamp);
}

void VideoWidget::TimerUpdate()
{
    if(this->seq == NULL && this->playActive)
    {
        this->Pause();
    }

    //Check if any async messages are waiting from the source media
    if(this->seq != NULL) this->seq->Update(FrameCallbackTest, (void *)this);

    if(this->playActive)
    {
        int elapsedMs = this->playPressedTime.elapsed();
        long long unsigned calcCurrentTime = this->playVidStartPos + elapsedMs;

        //Check if we have reached the end of the video
        if(calcCurrentTime >= this->mediaLength)
        {
            calcCurrentTime = this->mediaLength;
            this->Pause();
        }

        //This automatically triggers the video frame refresh
        this->ui->horizontalScrollBar->setValue(calcCurrentTime);
    }
}

void VideoWidget::AsyncFrameReceived(QImage& fr, unsigned long long timestamp)
{
    if(this->waitingForNumFrames > 0)
        this->waitingForNumFrames -- ;

    //Add to scene
    this->sceneControl->VideoImageChanged(fr);
    this->ui->graphicsView->setScene(&*this->sceneControl->scene);

    //Update current time
    this->currentTime = timestamp;

}

void VideoWidget::SetSceneControl(QSharedPointer<SimpleScene> sceneIn)
{
    if(!this->sceneControl.isNull())
        this->sceneControl->RemoveToolButtons(this->ui->annotationTools);
    this->sceneControl = sceneIn;
    this->ui->graphicsView->setScene(&*this->sceneControl->scene);
    this->sceneControl->AddToolButtons(this->ui->annotationTools);
}
