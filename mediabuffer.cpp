#include "mediabuffer.h"
#include <iostream>
#include <assert.h>
using namespace std;

DecodedFrame::DecodedFrame() : Deletable()
{
    //cout << "DecodedFrame::DecodedFrame()" << endl;
    height = 0;
    width = 0;
    sample_aspect_num = 0;
    sample_aspect_den = 0;
    frame_rate_num = 0;
    frame_rate_den = 0;
    timestamp = 0;
    endTimestamp = 0;
    requestedTimestamp = 0;

    buff = NULL;
    buffSize = 0;
}

DecodedFrame::DecodedFrame(const DecodedFrame &other)
{
    //cout << "DecodedFrame::DecodedFrame(other)" << endl;
    this->buff = NULL;
    this->buffSize = 0;
    DecodedFrame::operator=(other);
}

DecodedFrame& DecodedFrame::operator=(const DecodedFrame& other)
{
    height = other.height;
    width = other.width;
    sample_aspect_num = other.sample_aspect_num;
    sample_aspect_den = other.sample_aspect_den;
    frame_rate_num = other.frame_rate_num;
    frame_rate_den = other.frame_rate_den;
    timestamp = other.timestamp;
    endTimestamp = other.endTimestamp;
    requestedTimestamp = other.requestedTimestamp;

    if(this->buffSize != other.buffSize)
        this->AllocateSize(other.buffSize);
    if(this->buff == NULL)
    {
        throw runtime_error("Bad alloc detected when allocating buffer");
    }
    memcpy(this->buff, other.buff, other.buffSize);
    return *this;
}

DecodedFrame::~DecodedFrame()
{
    //cout << "DecodedFrame::~DecodedFrame()" << endl;
    if(this->buff != NULL) delete [] this->buff;
    this->buffSize = 0;
}

void DecodedFrame::AllocateSize(unsigned int size)
{
    if(this->buff) delete [] this->buff;
    this->buff = NULL;
    try
    {
        this->buff = new uint8_t[size];
        this->buffSize = size;
    }
    catch(bad_alloc &err)
    {
        cout << "Error: Bad alloc" << endl;
    }
}

template <class T>void SwapVals(T &a, T &b)
{
    T tmp;
    tmp = a;
    a = b;
    b = tmp;
}

void DecodedFrame::FastSwap(class DecodedFrame &other)
{
    SwapVals<unsigned int>(height, other.height);
    SwapVals<unsigned int>(width, other.width);
    SwapVals<unsigned int>(sample_aspect_num, other.sample_aspect_num);
    SwapVals<unsigned int>(sample_aspect_den, other.sample_aspect_den);
    SwapVals<unsigned int>(frame_rate_num, other.frame_rate_num);
    SwapVals<unsigned int>(frame_rate_den, other.frame_rate_den);
    SwapVals<uint64_t>(timestamp, other.timestamp);
    SwapVals<uint64_t>(endTimestamp, other.endTimestamp);
    SwapVals<uint64_t>(requestedTimestamp, other.requestedTimestamp);
    SwapVals<uint8_t *>(this->buff, other.buff);
    SwapVals<unsigned int>(this->buffSize, other.buffSize);
}

//**************************************

ProcessingRequest::ProcessingRequest()
{
    this->buff = NULL;
    this->buffSize = 0;
}

ProcessingRequest::ProcessingRequest(const ProcessingRequest &other)
{
    operator=(other);
}

ProcessingRequest& ProcessingRequest::operator=(const ProcessingRequest& other)
{
    this->AllocateSize(other.buffSize);
    if(this->buff == NULL)
    {
        throw runtime_error("Bad alloc detected when allocating buffer");
    }
    memcpy(this->buff, other.buff, other.buffSize);
    this->pos = other.pos;
    return *this;
}

ProcessingRequest::~ProcessingRequest()
{
    if(this->buff != NULL) delete [] this->buff;
    this->buffSize = 0;
}

void ProcessingRequest::AllocateSize(unsigned int size)
{
    if(this->buff) delete [] this->buff;
    this->buff = NULL;
    try
    {
        this->buff = new uint8_t[size];
        this->buffSize = size;
    }
    catch(bad_alloc &err)
    {
        cout << "Error: Bad alloc" << endl;
    }
}
