#include "videowidget.h"
#include "ui_videowidget.h"
#include <QDir>
#include <QFile>
#include <QObject>
#include <iostream>
#include <assert.h>
#include <stdexcept>
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

    //QObject::connect(this->ui->horizontalScrollBar, SIGNAL(valueChanged(int)), this, SLOT(SliderMoved(int)));
    //QObject::connect(this->ui->pauseButton,SIGNAL(clicked()), this, SLOT(Pause()));
    //QObject::connect(this->ui->playButton,SIGNAL(clicked()), this, SLOT(Play()));

    this->scene = QSharedPointer<QGraphicsScene>(new QGraphicsScene(this));

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

void VideoWidget::SetSource(QSharedPointer<AbstractMedia> src)
{
    this->seq = src;
    this->mediaLength = 0;

    if(!this->seq.isNull())try
    {
        this->mediaLength = src->Length();
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
    if(this->seq.isNull()) return;

    //Get image from sequence
    try
    {
        unsigned long long actualTi = 0;
        //QSharedPointer<QImage> image = this->seq->Get(ti, actualTi);
        this->seq->RequestFrame(ti);

/*
        //cout << "Requested:"<<ti << " Got:"<< actualTi << endl;

        assert(!image->isNull());

        //Add to scene
        this->item = QSharedPointer<QGraphicsPixmapItem>(new QGraphicsPixmapItem(QPixmap::fromImage(*image)));
        this->scene->clear();
        this->scene->addItem(&*item); //I love pointers
        this->ui->graphicsView->setScene(&*this->scene);

        //Update current time
        this->currentTime = actualTi;*/
    }
    catch(std::runtime_error &err)
    {
        cout << "Warning: failed to update to requested frame" << endl;
    }

}

void VideoWidget::SliderMoved(int newValue)
{
    if(this->waitingForNumFrames < 2)
    {
        this->SetVisibleAtTime(newValue);
        this->waitingForNumFrames ++;
    }
}

void VideoWidget::Pause()
{
    cout << "pause" << endl;
    playActive = false;
}

void VideoWidget::Play()
{
    cout << "play" << endl;
    this->playPressedTime.start();

    //If the video is at the end, start playing from the beginning
    cout << this->currentTime <<","<< this->seq->GetFrameStartTime(this->mediaLength) << endl;
    if(this->currentTime < this->seq->GetFrameStartTime(this->mediaLength))
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
    if(this->seq.isNull())
    {
        this->Pause();
    }

    //Check if any async messages are waiting from the source media
    if(!this->seq.isNull()) this->seq->Update(FrameCallbackTest, (void *)this);

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
      //cout << "Got:"<< timestamp << endl;

      //Add to scene
      this->item = QSharedPointer<QGraphicsPixmapItem>(new QGraphicsPixmapItem(QPixmap::fromImage(fr)));
      this->scene->clear();
      this->scene->addItem(&*item); //I love pointers
      this->ui->graphicsView->setScene(&*this->scene);

      //Update current time
      this->currentTime = timestamp;

      if(this->waitingForNumFrames > 0)
          this->waitingForNumFrames -- ;

}
