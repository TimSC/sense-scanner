#include "videowidget.h"
#include "ui_videowidget.h"
#include <QDir>
#include <QFile>
#include <QObject>
#include <iostream>
#include <assert.h>
using namespace std;


QString StripLeftAlphaChars(QString in)
{
    //Determine end of alphanumberic chars
    int i;
    for(i=0;i<in.length();i++)
    {
        QString c = in.mid(i,1);
        const char *cd = c.toLocal8Bit().constData();
        int check = (cd[0] >= '0' && cd[0] <= '9');
        if(check) break;
    }
    return in.mid(i);
}

QString GetLeftAlphaChars(QString in)
{
    //Determine end of alphanumberic chars
    int i;
    for(i=0;i<in.length();i++)
    {
        QString c = in.mid(i,1);
        const char *cd = c.toLocal8Bit().constData();
        int check = (cd[0] >= '0' && cd[0] <= '9');
        if(check) break;
    }
    return in.left(i);
}

ImageSequence::ImageSequence(QString targetDir, float frameRate)
{
    QDir directory(targetDir);
    QStringList dirFiles = directory.entryList();
    this->targetDir = targetDir;
    this->maxIndex = 0;
    this->minIndex = 0;
    this->numPackedChars = 0;
    this->maxPrefix = "";
    this->maxExt = "";
    this->frameRate = frameRate;
    int packedChars = 0;

    //Get highest value image
    for (int i = 0; i < dirFiles.size(); ++i)
    {
        QString fina = dirFiles.at(i);
        QFileInfo finai = fina;

        QString baseName = finai.baseName(); //filename without extension
        QString extName = finai.completeSuffix();
        //cout << fina.toLocal8Bit().constData() <<"," << baseName.toLocal8Bit().constData() << endl;
        QString stripName = StripLeftAlphaChars(baseName);
        packedChars = stripName.length();
        long long unsigned ind = stripName.toInt();
        QString prefix = GetLeftAlphaChars(baseName);
        if(ind > maxIndex)
        {
            this->maxIndex = ind;
            this->maxPrefix = prefix;
            this->maxExt = extName;
        }
    }

    //Get lowest value file
    //TODO this is a much better place to determine if the file name is packed
    for(long long unsigned i=0;i<=this->maxIndex;i++)
    {
        //Test for packed name
        QString formatStr;
        formatStr.sprintf("%%s/%%s%%0%illd.%%s", packedChars);

        QString fina;
        fina.sprintf(formatStr.toLocal8Bit().constData(),
                 this->targetDir.toLocal8Bit().constData(),
                 this->maxPrefix.toLocal8Bit().constData(), i,
                 this->maxExt.toLocal8Bit().constData());
        QFile file(fina);
        if(file.exists())
        {
            this->minIndex = i;
            this->numPackedChars = packedChars;
            break;
        }

        //Test for unpacked name
        QString fina2;
        fina2.sprintf("%s/%s%lld.%s", this->targetDir.toLocal8Bit().constData(),
                 this->maxPrefix.toLocal8Bit().constData(), i,
                 this->maxExt.toLocal8Bit().constData());
        QFile file2(fina2);
        if(file2.exists())
        {
            this->minIndex = i;
            this->numPackedChars = 0;
            break;
        }

    }

    //cout << this->maxPrefix.toLocal8Bit().constData()
    //     << "\t" << this->maxIndex << "\t" << this->maxExt.toLocal8Bit().constData()
    //     << "\t" << this->maxPackedChars << endl;
}

ImageSequence::~ImageSequence()
{

}

QSharedPointer<QImage> ImageSequence::Get(long long unsigned ti) //in milliseconds
{
    long long unsigned frameNum = float(ti) * this->frameRate / 1000.;
    frameNum += this->minIndex;

    if (frameNum < this->minIndex) frameNum = this->minIndex;
    if (frameNum > this->maxIndex) frameNum = this->maxIndex;
    cout << frameNum << endl;
    QString fina;
    fina.sprintf("%s/%s%03lld.%s", this->targetDir.toLocal8Bit().constData(),
                 this->maxPrefix.toLocal8Bit().constData(), frameNum,
                 this->maxExt.toLocal8Bit().constData());
    //cout << fina.toLocal8Bit().constData() << endl;
    QImage *image = new QImage(fina);
    assert(!image->isNull());
    QSharedPointer<QImage> out(image);
    return out;
}

long long unsigned ImageSequence::GetNumFrames()
{
    return this->maxIndex + 1 - this->minIndex;
}

long long unsigned ImageSequence::Length() //Get length
{
    int numFrames = this->GetNumFrames();
    return numFrames * 1000. / this->frameRate;
}

//********************************************************************

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
    QObject::connect(this->ui->stopButton,SIGNAL(clicked()), this, SLOT(Stop()));
    QObject::connect(this->ui->pauseButton,SIGNAL(clicked()), this, SLOT(Pause()));
    QObject::connect(this->ui->playButton,SIGNAL(clicked()), this, SLOT(Play()));

    this->seq = QSharedPointer<ImageSequence>(new ImageSequence("/home/tim/dev/QtMedia/testseq"));
    this->scene = QSharedPointer<QGraphicsScene>(new QGraphicsScene(this));

    this->SetVisibleAtTime(0);
    this->ui->horizontalScrollBar->setRange(0, seq->Length()-1);

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
    QSharedPointer<QImage> image = this->seq->Get(ti);
    assert(!image->isNull());

    this->item = QSharedPointer<QGraphicsPixmapItem>(new QGraphicsPixmapItem(QPixmap::fromImage(*image)));
    this->scene->clear();
    this->scene->addItem(&*item); //I love pointers
    this->ui->graphicsView->setScene(&*this->scene);

    this->currentTime = ti;
}

void VideoWidget::SliderMoved(int newValue)
{
    this->SetVisibleAtTime(newValue);
}

void VideoWidget::Stop()
{
    cout << "stop" << endl;
    playActive = false;
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
    if(this->currentTime < this->seq->Length()-1)
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
            calcCurrentTime = this->seq->Length()-1;
            this->Stop();
        }

        //This automatically triggers the video frame refresh
        this->ui->horizontalScrollBar->setValue(calcCurrentTime);
    }
}
