#ifndef MEDIABUFFER_H
#define MEDIABUFFER_H

#include <QObject>
#include <QSharedPointer>
#include <QImage>
#include <QMap>
#include <inttypes.h>
#include <tr1/memory>

class DecodedFrame
{
public:
    uint8_t *buff;
    int buffSize;

    unsigned int height, width;
    unsigned int sample_aspect_num, sample_aspect_den;
    unsigned int frame_rate_num, frame_rate_den;
    int64_t timestamp;

    DecodedFrame();
    DecodedFrame(const DecodedFrame &other);
    virtual ~DecodedFrame();
    void AllocateSize(unsigned int size);
};

class AbstractMedia
{
public:
    explicit AbstractMedia() {}
    virtual ~AbstractMedia() {}

    virtual QSharedPointer<QImage> Get(long long unsigned ti)=0; //in milliseconds
    int GetFrame(int64_t time, class DecodedFrame &out) {return 0;}

    virtual long long unsigned GetNumFrames()=0;
    virtual long long unsigned Length()=0; //Get length (ms)
    virtual long long unsigned GetFrameStartTime(long long unsigned ti)=0; //in milliseconds
};

class MediaBuffer: public AbstractMedia
{
public:
    explicit MediaBuffer(QSharedPointer<AbstractMedia> src);
    virtual ~MediaBuffer();

    void SetSource(QSharedPointer<AbstractMedia> src);

    QSharedPointer<QImage> Get(long long unsigned ti); //in milliseconds
    long long unsigned GetNumFrames();
    long long unsigned Length(); //Get length (ms)
    long long unsigned GetFrameStartTime(long long unsigned ti); //in milliseconds

signals:
    
protected:
    QSharedPointer<AbstractMedia> seq;
    QMap<unsigned long long, QSharedPointer<QImage> > buffer;
public slots:
    
};

#endif // MEDIABUFFER_H
