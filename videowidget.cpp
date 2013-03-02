#include "videowidget.h"
#include "ui_videowidget.h"
#include "avbinmedia.h"
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QObject>
#include <QtGui/QPushButton>
#include <iostream>
#include <assert.h>
#include <stdexcept>
#include <cstdlib>
#include <math.h>
#include "eventloop.h"
using namespace std;
#define ROUND_TIMESTAMP(x) (unsigned long long)(x+0.5)

//Custom graphics view to catch mouse wheel

void RawImgToQImage(QByteArray &pix, unsigned width, unsigned height, QImage &img)
{
    assert(width > 0);
    assert(height > 0);
    assert(pix.size() > 0);

    uint8_t *raw = (uint8_t *)pix.constBegin();
    unsigned int cursor = 0;
    for(unsigned int j=0;j<height;j++)
        for(unsigned int i=0;i<width;i++)
        {
            cursor = i * 3 + (j * width * 3);
            assert(cursor + 2 < pix.size());

            QRgb value = qRgb(raw[cursor], raw[cursor+1], raw[cursor+2]);
            img.setPixel(i, j, value);
        }
}

MediaResponseFrame::MediaResponseFrame(std::tr1::shared_ptr<class Event> ev)
{
    assert(ev->type=="MEDIA_FRAME_RESPONSE");
    assert(ev->data!="FAILED");

    std::string tmp(ev->data.toLocal8Bit().constData());
    std::vector<std::string> args = split(tmp,',');
    this->start = atof(args[0].c_str());
    this->end = atof(args[1].c_str());
    this->req = atof(args[2].c_str());
    unsigned width = atoi(args[3].c_str());
    unsigned height = atoi(args[4].c_str());

    //Convert to a QImage object
    QImage img2(width, height,QImage::Format_RGB888);
    RawImgToQImage(ev->buffer, width, height, img2);
    this->img = img2;
}

//***************************************

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
    this->ui->graphicsView->setMouseTracking(true);
    this->playVidStartPos = 0;
    this->currentTime = 0;
    this->playActive = false;
    this->mediaLength = 0;
    this->seq = QUuid();
    this->sceneControl = NULL;
    this->fitWindowToNextFrame = 0;
    this->lastRequestedTime = 1;
    this->eventLoop = NULL;
    this->eventReceiver = NULL;

    this->SetVisibleAtTime(0);

    //Start idle timer to update video when playing
    this->timer = QSharedPointer<QTimer>(new QTimer(this));
    QObject::connect(&(*this->timer),SIGNAL(timeout()), this, SLOT(TimerUpdate()));
    this->timer->start(10);
    this->ui->timeEdit->setDisplayFormat("hh:mm:ss:zzz");

}

VideoWidget::~VideoWidget()
{
    if(this->eventReceiver!=NULL)
        delete this->eventReceiver;
    this->eventReceiver = NULL;
    delete ui;
}

void VideoWidget::SetSource(QUuid src, QString finaIn)
{
    this->seq = src;
    this->mediaLength = 0;
    this->fina = finaIn;
    int lengthError = 0;

    if(!this->seq.isNull() && this->eventLoop!=NULL) try
    {
        this->mediaLength = AvBinMedia::GetMediaDuration(this->fina,
                                            this->seq,
                                            this->eventLoop,
                                            this->eventReceiver);
    }
    catch (std::runtime_error &e)
    {
        cout << "Warning: could not determine length of video" << endl;
        lengthError = 1;
    }

    if(this->mediaLength > 0)
        this->ui->horizontalScrollBar->setRange(0, this->mediaLength);
    else
        this->ui->horizontalScrollBar->setRange(0, 1000);
    this->ui->horizontalScrollBar->setValue(0);
    this->SetVisibleAtTime(0);

    this->fitWindowToNextFrame = 1;

    if(lengthError)
    {
        throw runtime_error("Error setting source");
    }
}

void VideoWidget::SetVisibleAtTime(long long unsigned ti)
{
    assert(this!=NULL);

    //Check the sequence is valid
    if(this->seq.isNull()) return;

    //Check the requested time has not already been set
    if(this->lastRequestedTime == ti)
        return;
    this->lastRequestedTime = ti;

    //Get image from sequence
    if(this->eventLoop) try
    {
        std::tr1::shared_ptr<class Event> reqEv(new Event("GET_MEDIA_FRAME"));
        reqEv->toUuid = this->seq;
        reqEv->data = this->fina;
        QString tiStr = QString("%1").arg(ti);
        reqEv->buffer = tiStr.toLocal8Bit().constData();
        reqEv->id = this->eventLoop->GetId();
        this->eventLoop->SendEvent(reqEv);
    }
    catch(std::runtime_error &err)
    {
        cout << "Warning: failed to update view to requested frame" << endl;
    }

}

void VideoWidget::SliderMoved(int newValue)
{
    //Update time display
    QTime time;
    int ms = newValue % 1000;
    int sec = ((newValue - ms) / 1000) % 60;
    int remainSec = (newValue - sec * 1000 - ms) / 1000;
    int min = (remainSec / 60) % 60;
    int remainMin = (remainSec - min * 60) / 60;
    time.setHMS(remainMin / 60, min, sec, ms);
    this->ui->timeEdit->setTime(time);


    this->SetVisibleAtTime(newValue);

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

void VideoWidget::SeekBack()
{
    if(this->sceneControl == NULL) return;
    try
    {
        //This throws an exception if no seek point exists
        unsigned long long ti = this->sceneControl->GetSeekBackTime();
        assert(ti < this->mediaLength);
        cout << "Requesting frame at " << ti << endl;
        this->ui->horizontalScrollBar->setValue(ti);
    }
    catch(exception &err) {}
}

void VideoWidget::SeekForward()
{
    if(this->sceneControl == NULL) return;
    try
    {
        //This throws an exception if no seek point exists
        unsigned long long ti = this->sceneControl->GetSeekFowardTime();
        assert(ti < this->mediaLength);
        cout << "Requesting frame at " << ti << endl;
        this->ui->horizontalScrollBar->setValue(ti);
    }
    catch(exception &err) {}
}

void VideoWidget::HandleEvent(std::tr1::shared_ptr<class Event> ev)
{
    if(ev->type=="MEDIA_FRAME_RESPONSE")
    {
        //Ignore frames not directed at widget
        if(ev->fromUuid != this->seq) return;

        MediaResponseFrame processedImg(ev);
        this->AsyncFrameReceived(processedImg.img, processedImg.start,
                                 processedImg.end, processedImg.req);
    }

}

void VideoWidget::TimerUpdate()
{
    if(this->seq.isNull() && this->playActive)
    {
        this->Pause();
    }

    try
    {
        assert(this->eventReceiver);
        std::tr1::shared_ptr<class Event> ev = this->eventReceiver->PopEvent();
        cout << "Event type " << qPrintable(ev->type) << endl;
        this->HandleEvent(ev);
    }
    catch(std::runtime_error e)
    {

    }

    //Check if any async messages are waiting from the source media
    /*if(!this->seq.isNull())
    {
        this->seq->Update(FrameCallbackTest, (void *)this);
    }
    else
    {
        int debug = 0;
    }*/

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

    //Monitor if the mouse is in the video view area
    int mouseOverVideoView = this->ui->graphicsView->underMouse();
    if(this->sceneControl!=NULL)
    {
        if(mouseOverVideoView && !this->sceneControl->GetMouseOver())
            this->sceneControl->MouseEnterEvent();
        if(!mouseOverVideoView && this->sceneControl->GetMouseOver())
            this->sceneControl->MouseLeaveEvent();
    }
}

void VideoWidget::AsyncFrameReceived(QImage& fr, unsigned long long startTimestamp,
                                     unsigned long long endTimestamp,
                                     unsigned long long requestTimestamp)
{
    cout << "Showing frame from " << startTimestamp << " to " << endTimestamp <<
            " ("<< requestTimestamp << ")" << endl;

    //Add to scene
    if(this->sceneControl!=NULL)
    {
        this->sceneControl->VideoImageChanged(fr, startTimestamp, endTimestamp, requestTimestamp);
        this->ui->graphicsView->setScene(&*this->sceneControl->GetScene());
    }

    //Update current time
    this->currentTime = startTimestamp;

    //Fit to window, if this is selected
    if(this->fitWindowToNextFrame)
        this->FitToWindow();
    this->fitWindowToNextFrame = 0;

    //Change sider to move one frame length in a single step
    if(endTimestamp > 0)
        this->ui->horizontalScrollBar->setSingleStep(endTimestamp - startTimestamp);

}

void VideoWidget::SetSceneControl(TrackingSceneController *sceneControlIn)
{
    //Remove previous scene button controls
    while(this->ui->annotationTools->count()>0)
    {
        //This item usually corresponds to the widget generated by the control factory
        QLayoutItem *item = this->ui->annotationTools->itemAt(0);
        QWidget *custom = item->widget();
        assert(custom!=NULL);
        cout << custom->metaObject()->className() << endl;

        //Also iterate through to get child widgets and directly close and remove them
        QLayout *wlayout = custom->layout();
        assert(wlayout!=NULL);
        while(wlayout->count())
        {
            int test = wlayout->count();
            QLayoutItem *citem = wlayout->itemAt(0);
            QWidget *childw = citem->widget();
            assert(childw!=NULL);
            childw->close();
            wlayout->removeItem(citem);
            delete childw;
        }

        custom->close();
        this->ui->annotationTools->removeItem(item);
        delete custom;
    }

    //Remove previous menu controls

    //Clear previous scene
    QGraphicsScene *oldScene = this->ui->graphicsView->scene();
    if(oldScene!=NULL) oldScene->clear();

    //Activate new scene button controls
    this->sceneControl = sceneControlIn;
    if(this->sceneControl!=NULL)
    {
        QSharedPointer<MouseGraphicsScene> scene = this->sceneControl->GetScene();
        if(!scene.isNull())
            this->ui->graphicsView->setScene(&*scene);
        else
            this->ui->graphicsView->setScene(NULL);
        this->ui->annotationTools->addWidget(this->sceneControl->ControlsFactory(this));
    }
    else
    {
        this->ui->graphicsView->setScene(NULL);
    }

}

void VideoWidget::SetAnnotationTrack(QUuid srcUuid)
{
    assert(this->sceneControl!=NULL);
    this->sceneControl->SetAnnotationTrack(srcUuid);
}

void VideoWidget::FitToWindow()
{
    if(this->sceneControl == NULL) return;
    QSharedPointer<MouseGraphicsScene> scene = this->sceneControl->GetScene();
    if(scene.isNull()) return;
    this->ui->graphicsView->fitInView(scene->itemsBoundingRect());
    QMatrix mat = this->ui->graphicsView->matrix();

    qreal scale = mat.m11();
    if(mat.m22() < scale) scale = mat.m22();
    mat.setMatrix(scale,mat.m12(),mat.m21(),scale,mat.dx(),mat.dy());
    this->ui->graphicsView->setMatrix(mat);
}

void VideoWidget::TimeChanged(QTime time)
{
    unsigned long long t = time.msec();
    t += time.second() * 1000;
    t += time.minute() * 60000;
    t += time.hour() * 3600000;
    this->ui->horizontalScrollBar->setValue(t);
    this->SetVisibleAtTime(t);
}

void VideoWidget::SetEventLoop(class EventLoop *eventLoopIn)
{
    this->eventLoop = eventLoopIn;
    this->eventReceiver = new EventReceiver(this->eventLoop);
    this->eventLoop->AddListener("MEDIA_FRAME_RESPONSE", *this->eventReceiver);
    this->eventLoop->AddListener("MEDIA_DURATION_RESPONSE", *this->eventReceiver);
}
