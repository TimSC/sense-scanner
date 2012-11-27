#include "imagesequence.h"
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

ImageSequence::ImageSequence(QString targetDir, float frameRate) : AbstractMedia()
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

QSharedPointer<QImage> ImageSequence::Get(long long unsigned ti,
                                          long long unsigned &outFrameTi) //in milliseconds
{
    long long unsigned frameNum = float(ti) * this->frameRate / 1000.;
    outFrameTi = frameNum * 1000 / this->frameRate;
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

long long unsigned ImageSequence::GetFrameStartTime(long long unsigned ti)
{
    long long unsigned frameNum = float(ti) * this->frameRate / 1000.;
    long long unsigned startTime = frameNum * 1000. / this->frameRate;
    return startTime;
}
