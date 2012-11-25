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
    std::tr1::shared_ptr<class FrameGroup> frames;

    //Check if time is in cache
    int cacheHit = 0;
    for(unsigned int i=0;i<this->groupCache.size();i++)
    {
        std::tr1::shared_ptr<class FrameGroup> &cachedGroup = this->groupCache[i];
        if(ti * 1000 >= cachedGroup->start && ti * 1000 < cachedGroup->end)
        {
            cout << ti * 1000 << "within" << cachedGroup->start << "," << cachedGroup->end<< endl;
            frames = cachedGroup;
            cacheHit = 1;
        }
    }

    if(!cacheHit)
    {
        //Get frames from source
        frames = this->backend->GetFrameRange(ti * 1000, (ti + 1000) * 1000);
        groupCache.push_back(frames);
    }

    //Find frame before requested time
    /*int bestIndex = 0; //Default to first frame
    int64_t score = -1;
    for (unsigned int i=0;i<frames->frames.size();i++)
    {
        std::tr1::shared_ptr<class DecodedFrame> &testFrame = frames->frames[i];
        if(testFrame->timestamp > ti * 1000.) continue; //Frame is too late
        int64_t diff = ti * 1000. - testFrame->timestamp;
        cout << i << ","<<ti*1000<<","<< testFrame->timestamp << "," << diff << endl;
        if(score < 0 || diff < score)
        {
            score = diff;
            bestIndex = i;
        }
    }
    std::tr1::shared_ptr<class DecodedFrame> &frame = frames->frames[bestIndex];*/
    if(frames->frames.size()==0)
    {
        QSharedPointer<QImage> img(new QImage(100, 100, QImage::Format_RGB888));
        return img;
    }

    std::tr1::shared_ptr<class DecodedFrame> &frame = frames->frames[0];

    cout << frame->width <<","<<  frame->height << endl;
    QSharedPointer<QImage> img(new QImage(frame->width, frame->height, QImage::Format_RGB888));

    cout << "frame time " << ti << endl;
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
