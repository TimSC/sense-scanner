#ifndef MEDIABUFFER_H
#define MEDIABUFFER_H

#include <QtCore/QObject>
#include <QtCore/QSharedPointer>
#include <QtGui/QImage>
#include <QtCore/QMap>
#include "localints.h"
#ifdef _MSC_VER
	#include <memory>
	#include <functional>
#else
	#include <tr1/memory>
	#include <tr1/functional>
#endif
#include "eventloop.h"

class DecodedFrame : public Deletable
{
public:
    uint8_t *buff;
    unsigned int buffSize;

    unsigned int height, width;
    unsigned int sample_aspect_num, sample_aspect_den;
    unsigned int frame_rate_num, frame_rate_den;
    uint64_t timestamp; //start time
    uint64_t endTimestamp; //end time
    uint64_t requestedTimestamp;

    DecodedFrame();
    DecodedFrame(const DecodedFrame &other);
    DecodedFrame& operator=(const DecodedFrame& other);

    virtual ~DecodedFrame();
    void AllocateSize(unsigned int size);
    void FastSwap(class DecodedFrame &other);
};

class ProcessingRequest : public Deletable
{
public:
    QSharedPointer<QImage> img;

    std::vector<std::vector<std::vector<float> > > pos;

    ProcessingRequest();
    ProcessingRequest(const ProcessingRequest &other);
    ProcessingRequest& operator=(const ProcessingRequest& other);
    virtual ~ProcessingRequest();
};

typedef std::tr1::function<void (const class DecodedFrame&)> FrameCallback;

class AbstractMedia
{
public:
    explicit AbstractMedia() {}
    virtual ~AbstractMedia() {}

    virtual QSharedPointer<QImage> Get(QString source,
                                       long long unsigned ti,
                                       long long unsigned &outFrameStart,
                                       long long unsigned &outFrameEnd,
                                       long long unsigned timeout = 50000)=0; //in milliseconds

    virtual long long unsigned Length(QString source)=0; //Get length (ms)
    virtual long long unsigned GetFrameStartTime(QString source, long long unsigned ti)=0; //in milliseconds

    virtual int RequestFrame(QString source, long long unsigned ti)=0;
    virtual void Update(void (*frameCallback)(QImage& fr, unsigned long long startTimestamp,
                                              unsigned long long endTimestamp,
                                              unsigned long long requestedTimestamp,
                                              void *raw), void *raw)=0;
};

#endif // MEDIABUFFER_H
