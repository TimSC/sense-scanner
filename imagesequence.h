#ifndef IMAGESEQUENCE_H
#define IMAGESEQUENCE_H

#include <QWidget>
#include <QtCore>
#include <QtGui>
#include "mediabuffer.h"

class ImageSequence : public AbstractMedia
{
public:
    ImageSequence(QObject *parent, QString targetDir, float frameRate = 25.);
    virtual ~ImageSequence();
    QSharedPointer<QImage> Get(long long unsigned ti); //in milliseconds
    long long unsigned GetNumFrames();
    long long unsigned Length(); //Get length (ms)
    long long unsigned GetFrameStartTime(long long unsigned ti); //in milliseconds

protected:
    long long unsigned minIndex, maxIndex;
    int numPackedChars;
    QString maxPrefix, maxExt, targetDir;
    float frameRate; //Hz
};

#endif // IMAGESEQUENCE_H