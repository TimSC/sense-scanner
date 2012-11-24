#include "avbinmedia.h"
#include "avbinbackend.h"
#include <assert.h>

AvBinMedia::AvBinMedia(QObject *parent) : QObject(parent)
{
    this->backend = new AvBinBackend;
}

AvBinMedia::~AvBinMedia()
{
    delete this->backend;
    this->backend = NULL;
}

QSharedPointer<QImage> AvBinMedia::Get(long long unsigned ti) //in milliseconds
{
    std::tr1::shared_ptr<class FrameGroup> frames =
        this->backend->GetFrameRange(ti * 1000, (ti + 1000) * 1000);

    std::tr1::shared_ptr<class DecodedFrame> pix = frames->frames[0];

    QSharedPointer<QImage> img(new QImage(pix->height, pix->width, QImage::Format_RGB888));
    return img;
}

long long unsigned AvBinMedia::GetNumFrames()
{
    assert(0); //Not implemented
}

long long unsigned AvBinMedia::Length() //Get length (ms)
{
    return this->backend->Length() / 1000.;
}

long long unsigned AvBinMedia::GetFrameStartTime(long long unsigned ti) //in milliseconds
{
    return ti;
}
