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
    if(this->backend) delete this->backend;
    this->backend = NULL;
}

QSharedPointer<QImage> AvBinMedia::Get(long long unsigned ti) //in milliseconds
{

    class DecodedFrame &frame = this->singleFrame;
    this->backend->GetFrame(ti * 1000, frame);

    QSharedPointer<QImage> img(new QImage(frame.width, frame.height, QImage::Format_RGB888));

    cout << "frame time " << ti << endl;
    assert(frame.buffSize > 0);
    cout << frame.width <<","<<  frame.height << endl;
    uint8_t *raw = &*frame.buff;
    cout << (unsigned long long)raw << endl;
    int cursor = 0;
    for(int j=0;j<frame.height;j++)
        for(int i=0;i<frame.width;i++)
        {
            cursor = i * 3 + (j * frame.width * 3);
            uint8_t *raw = &*frame.buff;
            assert(cursor >= 0);
            assert(cursor + 2 < frame.buffSize);

            QRgb value = qRgb(raw[cursor], raw[cursor+1], raw[cursor+2]);
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


//************************************

void MyThread::run()
{
    while(1)
    {
        cout << "x" << endl;
        msleep(200);
    }
}
