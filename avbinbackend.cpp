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

    //this->timestampOfChannel.clear();
    //for(unsigned int chanNum=0;chanNum<this->numStreams;chanNum++)
    //    this->timestampOfChannel.push_back(-1);
}

std::tr1::shared_ptr<class FrameGroup> AvBinBackend::GetFrameRange(int64_t startTime, int64_t endTime)
{

}


int AvBinBackend::GetFrame(int64_t time, class DecodedFrame &out)
{
    assert(this->fi != NULL);
    //Remove start offset
    assert(time >= 0);
    time += this->info.start_time;


    //cout << time << "," << firstVideoStream << "," << this->timestampOfChannel.size() << endl;
    //Seek in file
    AVbinResult res = avbin_seek_file(this->fi, time);
    assert(res == AVBIN_RESULT_OK);

    //Decode the packets
    AVbinPacket packet;
    packet.structure_size = sizeof(packet);
    int done = false;


    int debug = 0;
    int foundAFrame = 0;

    while (!avbin_read(this->fi, &packet) && (!done))
    {
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

        //this->timestampOfChannel[packet.stream_index] = timestamp;

        //Allocate video buffer
        unsigned requiredBuffSize = sinfo->video.width*sinfo->video.height*3;
        if(sinfo->type == AVBIN_STREAM_TYPE_VIDEO && ((out.buff)==NULL || requiredBuffSize > out.buffSize))
        {
            out.AllocateSize(requiredBuffSize);
            //cout << "Creating buffer at " << (unsigned long long) &*out.buff <<
            //        " of size " << out.buffSize<<endl;
            //cout << sinfo->video.width <<","<<sinfo->video.height << endl;
        }

        //Decode video packet
        if(sinfo->type == AVBIN_STREAM_TYPE_VIDEO)
        {
            assert(out.buff);
            assert(out.buffSize > 0);

            int32_t ret = avbin_decode_video(stream, packet.data, packet.size, out.buff);
            int error = (ret == -1);

            if(timestamp > time && foundAFrame)
            {
                done = 1;
                continue;
            }
            else
            {
                out.timestamp = timestamp - this->info.start_time;
            }

            if(!error)
            {
                assert(sinfo->video.height > 0);
                assert(sinfo->video.width > 0);
                out.height = sinfo->video.height;
                out.width = sinfo->video.width;
                out.sample_aspect_num = sinfo->video.sample_aspect_num;
                out.sample_aspect_den = sinfo->video.sample_aspect_den;
                out.frame_rate_num = sinfo->video.frame_rate_num;
                out.frame_rate_den = sinfo->video.frame_rate_den;
                this->height = out.height;
                this->width = out.width;
                foundAFrame = 1;
            }

        }

        //Decode audio packet
        if(sinfo->type == AVBIN_STREAM_TYPE_AUDIO)
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

    return foundAFrame;
}

//***************************************************************

//Replay retrieval
int AvBinBackend::Play(int64_t time, FrameCallback &frameFunc)
{
    assert(this->fi != NULL);

    //Remove start offset
    assert(time >= 0);
    time += this->info.start_time;

    //Seek in file
    AVbinResult res = avbin_seek_file(this->fi, time);
    assert(res == AVBIN_RESULT_OK);

    //Decode the packets
    class DecodedFrame out;
    AVbinPacket packet;
    packet.structure_size = sizeof(packet);

    while (!avbin_read(this->fi, &packet))
    {
        if(this->streams.empty())
            this->OpenStreams();

        AVbinTimestamp &timestamp = packet.timestamp;
        AVbinStreamInfo *sinfo = this->streamInfos[packet.stream_index];
        AVbinStream *stream = this->streams[packet.stream_index];

        //Allocate video buffer
        unsigned requiredBuffSize = sinfo->video.width*sinfo->video.height*3;
        if(sinfo->type == AVBIN_STREAM_TYPE_VIDEO && ((out.buff)==NULL || requiredBuffSize > out.buffSize))
        {
            out.AllocateSize(requiredBuffSize);
            cout << "Creating buffer at " << (unsigned long long) &*out.buff <<
                    " of size " << out.buffSize<<endl;
            //cout << sinfo->video.width <<","<<sinfo->video.height << endl;
        }

        //Decode video packet
        if(sinfo->type == AVBIN_STREAM_TYPE_VIDEO)
        {
            assert(out.buff);
            assert(out.buffSize > 0);

            int32_t ret = avbin_decode_video(stream, packet.data, packet.size, out.buff);
            int error = (ret == -1);

            if(!error)
            {
                out.height = sinfo->video.height;
                out.width = sinfo->video.width;
                out.sample_aspect_num = sinfo->video.sample_aspect_num;
                out.sample_aspect_den = sinfo->video.sample_aspect_den;
                out.frame_rate_num = sinfo->video.frame_rate_num;
                out.frame_rate_den = sinfo->video.frame_rate_den;
                out.timestamp = timestamp - this->info.start_time;

                frameFunc(out);
            }
        }

        //Decode audio packet
        if(sinfo->type == AVBIN_STREAM_TYPE_AUDIO)
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

    }
    return 1;
}

int AvBinBackend::Pause()
{

}

int AvBinBackend::Stop()
{

}

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
