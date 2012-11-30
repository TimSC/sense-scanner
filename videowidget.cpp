#include "videowidget.h"
#include "ui_videowidget.h"
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QObject>
#include <iostream>
#include <assert.h>
#include <stdexcept>
#include <cstdlib>
using namespace std;

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

SimpleScene::SimpleScene(QWidget *parent)
{
    this->scene = QSharedPointer<QGraphicsScene>(new QGraphicsScene(parent));
    for(int i=0;i<50;i++)
    {
        vector<float> p;
        p.push_back(rand() % 500);
        p.push_back(rand() % 500);
        this->pos.push_back(p);
    }
}

SimpleScene::~SimpleScene()
{
    this->item = QSharedPointer<QGraphicsPixmapItem>(NULL);
    this->scene->clear();
}

void SimpleScene::VideoImageChanged(QImage &fr)
{

    this->item =
            QSharedPointer<QGraphicsPixmapItem>(new QGraphicsPixmapItem(QPixmap::fromImage(fr)));
    this->scene->clear();
    this->scene->addItem(&*this->item); //I love pointers
    this->scene->setSceneRect ( 0, 0, fr.width(), fr.height() );

    QPen pen(QColor(255,0,0));
    QBrush brush(QColor(255,0,0,0));

    for(unsigned int i=0;i<this->pos.size();i++)
    {
        this->scene->addEllipse(this->pos[i][0], this->pos[i][1], 2, 2, pen, brush);
    }
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
    this->sceneControl = QSharedPointer<SimpleScene>(new SimpleScene(this));

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
    this->sceneControl = sceneIn;
    this->ui->graphicsView->setScene(&*this->sceneControl->scene);
}
