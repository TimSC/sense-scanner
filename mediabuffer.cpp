#include "mediabuffer.h"

MediaBuffer::MediaBuffer(QObject *parent, QSharedPointer<AbstractMedia> src) : AbstractMedia(parent)
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
    return this->seq->Get(ti);
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
