#include "avbinmedia.h"
#include "avbinbackend.h"
#include <assert.h>
#include <iostream>
using namespace std;

AvBinMedia::AvBinMedia(QObject *parent, QString fina) : AbstractMedia(parent)
{
    this->backend = new AvBinBackend;
    this->backend->OpenFile(fina.toLocal8Bit().constData());
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

    std::tr1::shared_ptr<class DecodedFrame> frame = frames->frames[0];

    cout << frame->width <<","<<  frame->height << endl;
    QSharedPointer<QImage> img(new QImage(frame->width, frame->height, QImage::Format_RGB888));

    int cursor = 0;
    for(int j=0;j<frame->height;j++)
        for(int i=0;i<frame->width;i++)
        {
            uint8_t *raw = &*frame->buff;
            QRgb value = qRgb(raw[cursor], raw[cursor+1], raw[cursor+2]);
            cursor += 3;
            img->setPixel(i, j, value);
        }

    //QSharedPointer<QImage> img(new QImage(&*frame->buff, frame->width, frame->height, QImage::Format_RGB888));
    //img->save("test.png");

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
