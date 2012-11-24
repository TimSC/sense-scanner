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
    //if(this->fi)
    //    avbin_close_file(this->fi);
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
        AVbinStreamInfo sinfo;
        info.structure_size = sizeof(AVbinStreamInfo);
        avbin_stream_info(this->fi, i, &sinfo);
        this->PrintAVbinStreamInfo(sinfo);
    }

    for(int32_t i = 0; i<numStreams; i++)
        AVbinStream *stream = avbin_open_stream(this->fi, i);

    AVbinPacket packet;
    AVbinResult res = avbin_read(this->fi, &packet);


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
