#include "avbinbackend.h"
#include <iostream>
#include <assert.h>
#include <sstream>
using namespace std;

AvBinBackend::AvBinBackend()
{
    AVbinResult res = avbin_init();
    assert(res == AVBIN_RESULT_OK);
    this->numStreams = -1;
    this->fi = NULL;
    this->height = 0;
    this->width = 0;
    this->firstVideoStream = -1;
    this->firstAudioStream = -1;
}

AvBinBackend::AvBinBackend(const AvBinBackend &other)
{
    assert(0); //This should never happen
}

AvBinBackend::~AvBinBackend()
{
    this->CloseFile();
}

int AvBinBackend::OpenFile(const char *filenameIn)
{
    assert(this->fi == NULL);

    this->filename = filenameIn;
    this->DoOpenFile();
}

void AvBinBackend::DoOpenFile()
{
    assert(this->fi == NULL);
    this->fi = avbin_open_filename(this->filename.c_str());
    this->info.structure_size = sizeof(AVbinFileInfo);
    avbin_file_info(this->fi, &this->info);
    //this->PrintAVbinFileInfo(this->info);
    this->numStreams = this->info.n_streams;
    this->firstVideoStream = -1;
    this->firstAudioStream = -1;

    for(int32_t i = 0; i<numStreams; i++)
    {
        AVbinStreamInfo *sinfo = new AVbinStreamInfo;
        sinfo->structure_size = sizeof(AVbinStreamInfo);
        avbin_stream_info(this->fi, i, sinfo);
        this->PrintAVbinStreamInfo(*sinfo);
        this->streamInfos.push_back(sinfo);

        if(sinfo->type == AVBIN_STREAM_TYPE_VIDEO && this->firstVideoStream == -1)
        {
            this->firstVideoStream = i;
        }
        if(sinfo->type == AVBIN_STREAM_TYPE_AUDIO && this->firstAudioStream == -1)
        {
            this->firstAudioStream = i;
        }
    }

    this->timestampOfChannel.clear();
    for(unsigned int chanNum=0;chanNum<this->numStreams;chanNum++)
        this->timestampOfChannel.push_back(0);
}

std::tr1::shared_ptr<class FrameGroup> AvBinBackend::GetFrameRange(int64_t startTime, int64_t endTime)
{

}


int AvBinBackend::GetFrame(int64_t time, class DecodedFrame &out)
{
    assert(this->firstVideoStream >= 0);
    assert(this->firstVideoStream < this->timestampOfChannel.size());
    assert(this->fi != NULL);
    //Remove start offset
    assert(time >= 0);

    time += this->info.start_time;

    //Check if currently buffered frame is suitable for the response
    if(this->currentFrame.timestamp > 0 && this->prevFrame.timestamp > 0)
    {
        if(time >= this->prevFrame.timestamp
                && time < this->currentFrame.timestamp)
        {
            //Used cached frame
            out = this->prevFrame;
        }
    }

    //Check if the requested from is close to requested frame
    //so that seeking is unnecessary
    //If current currentVidTime is zero, that is an unknown position so we always seek
    uint64_t currentVidTime = this->timestampOfChannel[this->firstVideoStream];
    int doSeek = 1;

    if(currentVidTime > 0 && time > currentVidTime)
    {
        uint64_t diff = time - currentVidTime;
        if(diff < 1000000) doSeek = 0;
    }

    if(doSeek)
    {
        //Seek in file
        AVbinResult res = avbin_seek_file(this->fi, time);
        assert(res == AVBIN_RESULT_OK);
        for(unsigned int chanNum=0;chanNum<this->numStreams;chanNum++)
            this->timestampOfChannel[chanNum] = 0;
        this->currentFrame.width = 0;
        this->currentFrame.height = 0;
        this->currentFrame.timestamp = 0;
        this->prevFrame.width = 0;
        this->prevFrame.height = 0;
        this->prevFrame.timestamp = 0;
    }

    //Decode the packets
    AVbinPacket packet;
    packet.structure_size = sizeof(packet);
    int done = false;

    int debug = 0;
    int processing = 1;
    while (processing && (!done))
    {
        //Read frame from file
        int readRet = avbin_read(this->fi, &packet);
        if(readRet == -1)
        {
            processing = 0;
            continue;
        }

        if(this->streams.empty())
        {
            this->OpenStreams();
        }

        AVbinTimestamp &timestamp = packet.timestamp;
        AVbinStreamInfo *sinfo = this->streamInfos[packet.stream_index];
        AVbinStream *stream = this->streams[packet.stream_index];

        //cout << "Packet of stream " << packet.stream_index << " at " << timestamp
        //     << " type=" << sinfo->type;
        //if(sinfo->type == AVBIN_STREAM_TYPE_VIDEO) cout << " video";
        //if(sinfo->type == AVBIN_STREAM_TYPE_AUDIO) cout << " audio";
        //cout << endl;

        this->timestampOfChannel[packet.stream_index] = timestamp;

        //Decode video packet
        if(sinfo->type == AVBIN_STREAM_TYPE_VIDEO &&
                packet.stream_index == this->firstVideoStream)
        {
            //cout << "Decoded: " << this->timestampOfChannel[packet.stream_index] << endl;
            unsigned requiredBuffSize = sinfo->video.width*sinfo->video.height*3;

            int32_t ret = avbin_decode_video(stream, packet.data, packet.size, this->currentFrame.buff);
            int error = (ret == -1);
            //cout << "z" << (timestamp > time && this->currentFrame.width > 0) << error<<endl;
            if(timestamp > time && this->currentFrame.width > 0)
            {
                if ((out.buff)==NULL || requiredBuffSize != out.buffSize)
                    out.AllocateSize(requiredBuffSize);
                assert(out.buff);
                assert(out.buffSize > 0);

                out = this->currentFrame;
                if(time < out.timestamp)
                    cout << "Warning: found frame after requested time" << endl;
                assert(out.width > 0);
                assert(out.height > 0);
                done = 1;
            }

            if(!error)
            {
                this->prevFrame = this->currentFrame;

                //Allocate video buffer
                if ((this->currentFrame.buff)==NULL || requiredBuffSize != this->currentFrame.buffSize)
                    this->currentFrame.AllocateSize(requiredBuffSize);
                assert(this->currentFrame.buff);
                assert(this->currentFrame.buffSize > 0);

                assert(sinfo->video.height > 0);
                assert(sinfo->video.width > 0);
                this->currentFrame.height = sinfo->video.height;
                this->currentFrame.width = sinfo->video.width;
                this->currentFrame.sample_aspect_num = sinfo->video.sample_aspect_num;
                this->currentFrame.sample_aspect_den = sinfo->video.sample_aspect_den;
                this->currentFrame.frame_rate_num = sinfo->video.frame_rate_num;
                this->currentFrame.frame_rate_den = sinfo->video.frame_rate_den;
                this->currentFrame.timestamp = timestamp - this->info.start_time;
                this->height = out.height;
                this->width = out.width;
            }
        }

        //Decode audio packet
        if(sinfo->type == AVBIN_STREAM_TYPE_AUDIO &&
                packet.stream_index == this->firstAudioStream)
        {
            uint8_t buff[1024*1024];
            int bytesleft = 1024*1024;
            int bytesout = bytesleft;
            int bytesread = 0;
            uint8_t *cursor = buff;
            while ((bytesread = avbin_decode_audio(stream, packet.data, packet.size, cursor, &bytesout)) > 0)
            {
                packet.data += bytesread;
                packet.size -= bytesread;
                cursor += bytesout;
                bytesleft -= bytesout;
                bytesout = bytesleft;
            }

            int totalBytes = cursor-buff;
            //cout << "Read audio bytes " << totalBytes << endl;
        }

        debug ++;
        //if (debug > 100) break;

    }

    return done;
}

//***************************************************************

void AvBinBackend::SetEventLoop(class EventLoop *eventLoopIn)
{
    this->eventLoop = eventLoopIn;
    this->eventLoop->AddListener("AVBIN_OPEN_FILE", this->eventReceiver);
    this->eventLoop->AddListener("AVBIN_GET_DURATION", this->eventReceiver);
    this->eventLoop->AddListener("AVBIN_GET_FRAME", this->eventReceiver);
}

int AvBinBackend::PlayUpdate()
{
    int foundEvent = 0;
    try
    {
        std::tr1::shared_ptr<class Event> ev = this->eventReceiver.PopEvent();
        //cout << "Event type " << ev->type << endl;
        foundEvent = 1;
        this->HandleEvent(ev);
    }
    catch(std::runtime_error e) {}

}

void AvBinBackend::HandleEvent(std::tr1::shared_ptr<class Event> ev)
{
    if(ev->type=="AVBIN_OPEN_FILE")
    {
        this->OpenFile(ev->data.c_str());
    }
    if(ev->type=="AVBIN_GET_DURATION")
    {
        std::tr1::shared_ptr<class Event> response(new Event("AVBIN_DURATION_RESPONSE", ev->id));
        std::ostringstream tmp;
        tmp << this->Length();
        response->data = tmp.str();
        this->eventLoop->SendEvent(response);
    }
    if(ev->type=="AVBIN_GET_FRAME")
    {
        unsigned long long ti = std::strtoull(ev->data.c_str(),NULL,10);
        std::tr1::shared_ptr<class Event> response(new Event("AVBIN_FRAME_RESPONSE", ev->id));
        class DecodedFrame* decodedFrame = new DecodedFrame();
        response->raw = (uint8_t*) decodedFrame;
        int found = this->GetFrame(ti, *decodedFrame);
        if(found)
        {
            //Send decoded image as event
            assert(decodedFrame->buff != NULL);
            assert(decodedFrame->buffSize > 0);
            assert(decodedFrame->width > 0);
            assert(decodedFrame->height > 0);
            response->rawSize = sizeof(class DecodedFrame);
            this->eventLoop->SendEvent(response);
        }
        else
        {
            if(this->width==0 || this->height==0)
            {
            //Something went wrong, so a failure event is generated
            std::tr1::shared_ptr<class Event> fail(new Event("AVBIN_FRAME_FAILED", ev->id));
            this->eventLoop->SendEvent(fail);
            }

            //Return a placeholder image
            unsigned buffSize = this->height*this->width*3;
            decodedFrame->AllocateSize(buffSize);
            decodedFrame->height = this->height;
            decodedFrame->width = this->width;
            assert(decodedFrame->width > 0);
            assert(decodedFrame->height > 0);
            response->rawSize = sizeof(class DecodedFrame);
            this->eventLoop->SendEvent(response);
        }
    }
}

//***************************************************************

void AvBinBackend::OpenStreams()
{
    assert(this->fi != NULL);
    this->CloseStreams();

    for(int32_t i = 0; i<numStreams; i++)
    {
        AVbinStream *stream = avbin_open_stream(this->fi, i);
        this->streams.push_back(stream);
    }
}

void AvBinBackend::CloseStreams()
{
    for(unsigned int i =0; i < this->streams.size(); i++)
        avbin_close_stream(this->streams[i]);
    this->streams.clear();
}

void AvBinBackend::CloseFile()
{
    this->CloseStreams();

    for(unsigned int i =0; i < this->streamInfos.size(); i++)
        delete this->streamInfos[i];
    this->streamInfos.clear();

    if(this->fi)
        avbin_close_file(this->fi);
    this->fi = NULL;
}

int64_t AvBinBackend::Length()
{
    assert(this->fi != NULL);
    return this->info.duration - this->info.start_time;
}

void AvBinBackend::PrintAVbinFileInfo(AVbinFileInfo &info)
{
    cout << info.n_streams << endl;
    cout << info.title << endl;
    cout << info.author << endl;
    cout << info.copyright << endl;
    cout << info.comment << endl;
    cout << info.album << endl;
    cout << info.year << endl;
    cout << info.track << endl;
    cout << info.genre << endl;
    cout << info.start_time << endl;
    cout << info.duration << endl;
}

void AvBinBackend::PrintAVbinStreamInfo(AVbinStreamInfo &sinfo)
{
    if(sinfo.type == AVBIN_STREAM_TYPE_VIDEO)
    {
        cout << "Video" << endl;
        cout << "Vid size " << sinfo.video.height <<","<<sinfo.video.height<< endl;
        cout << "Vid aspect " << sinfo.video.sample_aspect_num << "/" << sinfo.video.sample_aspect_den << endl;
        cout << "Frame rate " << sinfo.video.frame_rate_num << "," << sinfo.video.frame_rate_den << endl;
    }
    if(sinfo.type == AVBIN_STREAM_TYPE_AUDIO)
    {
        cout << "Audio" << endl;

        cout << "Aud Rate "<< sinfo.audio.sample_rate << endl;
        cout << "Aud bit " << sinfo.audio.sample_bits << endl;
        cout << "Aud chan " << sinfo.audio.channels << endl;
    }
}
