#include "videowidget.h"
#include "ui_videowidget.h"
#include <QDir>
#include <QFile>
#include <QObject>
#include <iostream>
#include <assert.h>
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

    QObject::connect(this->ui->horizontalScrollBar, SIGNAL(valueChanged(int)), this, SLOT(SliderMoved(int)));
    QObject::connect(this->ui->pauseButton,SIGNAL(clicked()), this, SLOT(Pause()));
    QObject::connect(this->ui->playButton,SIGNAL(clicked()), this, SLOT(Play()));

    this->seq = QSharedPointer<ImageSequence>(new ImageSequence("/home/tim/dev/QtMedia/testseq"));
    this->scene = QSharedPointer<QGraphicsScene>(new QGraphicsScene(this));

    this->SetVisibleAtTime(0);
    this->ui->horizontalScrollBar->setRange(0, seq->Length());

    //Start idle timer to update video when playing
    this->timer = QSharedPointer<QTimer>(new QTimer(this));
    QObject::connect(&(*this->timer),SIGNAL(timeout()), this, SLOT(TimerUpdate()));
    this->timer->start(10);
}

VideoWidget::~VideoWidget()
{
    delete ui;
}

void VideoWidget::SetVisibleAtTime(long long unsigned ti)
{

    //Get image from sequence
    QSharedPointer<QImage> image = this->seq->Get(ti);
    assert(!image->isNull());

    //Add to scene
    this->item = QSharedPointer<QGraphicsPixmapItem>(new QGraphicsPixmapItem(QPixmap::fromImage(*image)));
    this->scene->clear();
    this->scene->addItem(&*item); //I love pointers
    this->ui->graphicsView->setScene(&*this->scene);

    //Update current time
    this->currentTime = ti;
}

void VideoWidget::SliderMoved(int newValue)
{
    this->SetVisibleAtTime(newValue);
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
    if(this->currentTime < this->seq->GetFrameStartTime(this->seq->Length()))
        this->playVidStartPos = this->currentTime;
    else
        this->playVidStartPos = 0;
    playActive = true;
}

void VideoWidget::TimerUpdate()
{
    if(this->playActive)
    {
        int elapsedMs = this->playPressedTime.elapsed();
        long long unsigned calcCurrentTime = this->playVidStartPos + elapsedMs;

        //Check if we have reached the end of the video
        if(calcCurrentTime >= this->seq->Length())
        {
            calcCurrentTime = this->seq->Length();
            this->Pause();
        }

        //Round time down to the start of the frame
        long long unsigned frameStartTime = this->seq->GetFrameStartTime(calcCurrentTime);

        //This automatically triggers the video frame refresh

        if (frameStartTime != this->currentTime)
            this->ui->horizontalScrollBar->setValue(frameStartTime);
    }
}
