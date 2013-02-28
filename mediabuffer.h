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
    /*!
    * DecodedFrame contains a video frame and associated meta data.
    * This is used to return requested frames in a high level manner.
    */

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

class ProcessingRequestOrResponse : public Deletable
{
    /*!
    * DecodedFrame contains a video frame and a tracking point model.
    * This is used for requesting a tracking position and returning the
    * result to the GUI.
    */

public:
    QSharedPointer<QImage> img;

    std::vector<std::vector<std::vector<float> > > pos;

    ProcessingRequestOrResponse();
    ProcessingRequestOrResponse(const ProcessingRequestOrResponse &other);
    ProcessingRequestOrResponse& operator=(const ProcessingRequestOrResponse& other);
    virtual ~ProcessingRequestOrResponse();
};

typedef std::tr1::function<void (const class DecodedFrame&)> FrameCallback;

class AbstractMedia
{
    /*!
    * AbstractMedia is a pure virtual interface for any sequential source of
    * image frames.
    */

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

    virtual long long unsigned RequestFrame(QString source, long long unsigned ti)=0;
    virtual void Update(void (*frameCallback)(QImage& fr, unsigned long long startTimestamp,
                                              unsigned long long endTimestamp,
                                              unsigned long long requestedTimestamp,
                                              void *raw), void *raw)=0;
};

#endif // MEDIABUFFER_H
