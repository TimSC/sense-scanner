#include "mediabuffer.h"
#include <iostream>
#include <assert.h>
using namespace std;

DecodedFrame::DecodedFrame()
{
    buffSize = 0;
    buff = NULL;
    height = 0;
    width = 0;
}

DecodedFrame::~DecodedFrame()
{
    if(buff!=NULL) delete [] buff;
    buff = 0;
    buffSize = 0;
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
