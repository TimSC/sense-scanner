#include "mediabuffer.h"
#include <iostream>
#include <assert.h>
using namespace std;

RawDataContainer::RawDataContainer()
{
    this->buff = NULL;
    this->buffSize = 0;
}

RawDataContainer::RawDataContainer(const RawDataContainer &other)
{
    this->buff = other.buff;
    this->buffSize = other.buffSize;
}

RawDataContainer::~RawDataContainer()
{
    if(this->buff) delete [] this->buff;
    this->buff = NULL;
}

//*****************************************************

DecodedFrame::DecodedFrame()
{
    height = 0;
    width = 0;
    sample_aspect_num = 0;
    sample_aspect_den = 0;
    frame_rate_num = 0;
    frame_rate_den = 0;
    timestamp = 0;
}

DecodedFrame::DecodedFrame(const DecodedFrame &other)
{
    raw = other.raw;
    height = other.height;
    width = other.width;
    sample_aspect_num = other.sample_aspect_num;
    sample_aspect_den = other.sample_aspect_den;
    frame_rate_num = other.frame_rate_num;
    frame_rate_den = other.frame_rate_den;
    timestamp = other.timestamp;
}

DecodedFrame::~DecodedFrame()
{

}

void DecodedFrame::AllocateSize(unsigned int size)
{
    this->raw = std::tr1::shared_ptr<class RawDataContainer>(new RawDataContainer);
    this->raw->buffSize = size;
    this->raw->buff = new uint8_t[size];
}

//**************************************

MediaBuffer::MediaBuffer(QSharedPointer<AbstractMedia> src) : AbstractMedia()
{
    this->seq = src;
}

MediaBuffer::~MediaBuffer()
{

}

void MediaBuffer::SetSource(QSharedPointer<AbstractMedia> src)
{
    this->seq = src;
}

QSharedPointer<QImage> MediaBuffer::Get(long long unsigned ti) //in milliseconds
{
    //Check if frame is in buffer
    if(this->buffer.contains(ti))
        return this->buffer[ti];

    //Get frame from underlying store
    QSharedPointer<QImage> img = this->seq->Get(ti);
    if(!img->isNull())
        this->buffer[ti] = img;

    //Prevent buffer bloating
    unsigned int totalPix = 0;
    QMapIterator<unsigned long long, QSharedPointer<QImage> > i(this->buffer);
    while(i.hasNext())
    {
        i.next();
        QSharedPointer<QImage> img = i.value();
        assert(!img->isNull());
        unsigned int numPix = img->width() * img->height();
        totalPix += numPix;
    }
    cout << "Buffer size: " << totalPix<<endl;

    return img;
}

long long unsigned MediaBuffer::GetNumFrames()
{
    return this->seq->GetNumFrames();
}

long long unsigned MediaBuffer::Length() //Get length (ms)
{
    return this->seq->Length();
}

long long unsigned MediaBuffer::GetFrameStartTime(long long unsigned ti)
{
    return this->seq->GetFrameStartTime(ti);
}
