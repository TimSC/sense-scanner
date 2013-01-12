#ifndef IMAGESEQUENCE_H
#define IMAGESEQUENCE_H

#include <QtGui/QWidget>
#include <QtCore/QtCore>
#include <QtGui/QtGui>
#include "mediabuffer.h"

class ImageSequence : public AbstractMedia
{
    /*!
    * ImageSequences uses a folder of images to emulate frames from
    * a video sequence.
    */

public:
    ImageSequence(QString targetDir, float frameRate = 25.);
    virtual ~ImageSequence();
    QSharedPointer<QImage> Get(long long unsigned ti,
                               long long unsigned &outFrameTi); //in milliseconds
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
