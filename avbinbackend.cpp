#include "avbinbackend.h"
#include <iostream>
#include <assert.h>
using namespace std;

AvBinBackend::AvBinBackend()
{
    AVbinResult res = avbin_init();
    assert(!res);
    this->numStreams = -1;
}

AvBinBackend::~AvBinBackend()
{
    this->CloseFile();
}

int AvBinBackend::OpenFile(const char *filename)
{
    this->fi = avbin_open_filename(filename);
    AVbinFileInfo info;
    info.structure_size = sizeof(AVbinFileInfo);
    avbin_file_info(this->fi, &info);
    //this->PrintAVbinFileInfo(info);
    this->numStreams = info.n_streams;

    for(int32_t i = 0; i<numStreams; i++)
    {
        AVbinStreamInfo *sinfo = new AVbinStreamInfo;
        sinfo->structure_size = sizeof(AVbinStreamInfo);
        avbin_stream_info(this->fi, i, sinfo);
        this->PrintAVbinStreamInfo(*sinfo);
        this->streamInfos.push_back(sinfo);
    }

    for(int32_t i = 0; i<numStreams; i++)
    {
        AVbinStream *stream = avbin_open_stream(this->fi, i);
        this->streams.push_back(stream);
    }

    AVbinPacket packet;
    packet.structure_size = sizeof(packet);

    while (!avbin_read(this->fi, &packet))
    {

        AVbinTimestamp &timestamp = packet.timestamp;
        AVbinStreamInfo *sinfo = this->streamInfos[packet.stream_index];
        AVbinStream *stream = this->streams[packet.stream_index];

        //cout << "Packet of stream " << packet.stream_index << " at " << timestamp
        //     << " type=" << sinfo->type << endl;


        if(sinfo->type == AVBIN_STREAM_TYPE_VIDEO)
        {
            uint8_t* buff = new uint8_t[sinfo->video.width*sinfo->video.height*3];
            if (avbin_decode_video(stream, packet.data, packet.size,buff)<=0)
                cout << "Error decoding video packet" << endl;

            //TODO stuff

            delete [] buff;

        }
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
}

void AvBinBackend::CloseFile()
{
    for(unsigned int i =0; i < this->streams.size(); i++)
        avbin_close_stream(this->streams[i]);
    this->streams.clear();

    for(unsigned int i =0; i < this->streamInfos.size(); i++)
        delete this->streamInfos[i];
    this->streamInfos.clear();

    if(this->fi)
        avbin_close_file(this->fi);
    this->fi = NULL;
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
        //cout << "Frame rate " << sinfo.video.frame_rate_num << "," << sinfo.video.frame_rate_den << endl;
    }
    if(sinfo.type == AVBIN_STREAM_TYPE_AUDIO)
    {
        cout << "Audio" << endl;

        cout << "Aud Rate "<< sinfo.audio.sample_rate << endl;
        cout << "Aud bit " << sinfo.audio.sample_bits << endl;
        cout << "Aud chan " << sinfo.audio.channels << endl;
    }
}
