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

ImageSequence::ImageSequence(QString targetDir)
{
    QDir directory(targetDir);
    QStringList dirFiles = directory.entryList();
    this->targetDir = targetDir;
    this->maxIndex = 0;
    this->minIndex = 0;
    this->maxPackedChars = 0;
    this->maxPrefix = "";
    this->maxExt = "";

    //Get highest value image
    for (int i = 0; i < dirFiles.size(); ++i)
    {
        QString fina = dirFiles.at(i);
        QFileInfo finai = fina;

        QString baseName = finai.baseName(); //filename without extension
        QString extName = finai.completeSuffix();
        //cout << fina.toLocal8Bit().constData() <<"," << baseName.toLocal8Bit().constData() << endl;
        QString stripName = StripLeftAlphaChars(baseName);
        int packed = (stripName.left(1).toLocal8Bit().constData())[0] == '0';
        int packedChars = 0;
        if(packed) packedChars = stripName.length();
        long long unsigned ind = stripName.toInt();
        QString prefix = GetLeftAlphaChars(baseName);
        if(ind > maxIndex)
        {
            this->maxIndex = ind;
            this->maxPrefix = prefix;
            this->maxExt = extName;
            this->maxPackedChars = packedChars;
        }
    }

    //Get lowest value file
    //TODO this is a much better place to determine if the file name is packed
    for(long long unsigned i=0;i<=this->maxIndex;i++)
    {
        QString fina;
        fina.sprintf("%s/%s%03lld.%s", this->targetDir.toLocal8Bit().constData(),
                 this->maxPrefix.toLocal8Bit().constData(), i,
                 this->maxExt.toLocal8Bit().constData());
        QFile file(fina);
        if(file.exists())
        {
            this->minIndex = i;
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
    ti += this->minIndex;
    QString fina;
    fina.sprintf("%s/%s%03lld.%s", this->targetDir.toLocal8Bit().constData(),
                 this->maxPrefix.toLocal8Bit().constData(), ti,
                 this->maxExt.toLocal8Bit().constData());
    //cout << fina.toLocal8Bit().constData() << endl;
    QImage *image = new QImage(fina);
    assert(!image->isNull());
    QSharedPointer<QImage> out(image);
    return out;
}

long long unsigned ImageSequence::Length() //Get length
{
    return this->maxIndex + 1 - this->minIndex;
}

//********************************************************************

VideoWidget::VideoWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::VideoWidget)
{
    ui->setupUi(this);

    QObject::connect(this->ui->horizontalScrollBar, SIGNAL(sliderMoved(int)), this, SLOT(SliderMoved(int)));

    this->seq = QSharedPointer<ImageSequence>(new ImageSequence("/home/tim/dev/QtMedia/testseq"));

    this->SetVisibleAtTime(0);
    this->ui->horizontalScrollBar->setRange(0, seq->Length()-1);
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
    this->scene = QSharedPointer<QGraphicsScene>(new QGraphicsScene(this));
    this->scene->addItem(&*item); //I love pointers
    this->ui->graphicsView->setScene(&*this->scene);
}

void VideoWidget::SliderMoved(int newValue)
{
    this->SetVisibleAtTime(newValue);
}

