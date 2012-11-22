#include "mediabuffer.h"

MediaBuffer::MediaBuffer(QObject *parent) : AbstractMedia(parent)
{
}

MediaBuffer::~MediaBuffer()
{

}

void MediaBuffer::SetSource(QSharedPointer<MediaBuffer> src)
{

}

QSharedPointer<QImage> MediaBuffer::Get(long long unsigned ti) //in milliseconds
{


}

long long unsigned MediaBuffer::GetNumFrames()
{


}

long long unsigned MediaBuffer::Length() //Get length (ms)
{


}

long long unsigned MediaBuffer::GetFrameStartTime(long long unsigned ti)
{


}
