#include "avbinbackend.h"
#include <iostream>
#include <assert.h>
using namespace std;

AvBinBackend::AvBinBackend()
{
    AVbinResult res = avbin_init();
    assert(res == AVBIN_RESULT_OK);
    this->numStreams = -1;
    this->fi = NULL;
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

    for(int32_t i = 0; i<numStreams; i++)
    {
        AVbinStreamInfo *sinfo = new AVbinStreamInfo;
        sinfo->structure_size = sizeof(AVbinStreamInfo);
        avbin_stream_info(this->fi, i, sinfo);
        this->PrintAVbinStreamInfo(*sinfo);
        this->streamInfos.push_back(sinfo);
    }
}

std::tr1::shared_ptr<class FrameGroup> AvBinBackend::GetFrameRange(int64_t startTime, int64_t endTime)
{
    std::tr1::shared_ptr<class FrameGroup> out(new class FrameGroup);
    std::vector<std::tr1::shared_ptr<class DecodedFrame> > videoOut;
    out->start = startTime;
    out->end = endTime;

    //this->CloseFile();
    //if(this->fi == NULL) this->DoOpenFile();
    //AVbinResult res = avbin_seek_file(this->fi, 0);

    //Remove start offset
    startTime -= this->info.start_time;
    endTime -= this->info.start_time;

    //Seek in file
    AVbinResult res = avbin_seek_file(this->fi, startTime);
    assert(res == AVBIN_RESULT_OK);

    //Decode the packets
    AVbinPacket packet;
    packet.structure_size = sizeof(packet);
    int done = false;

    vector<int64_t> timestampOfChannel;
    for(unsigned int chanNum=0;chanNum<this->numStreams;chanNum++)
        timestampOfChannel.push_back(-1);

	int debug = 0;

    while (!avbin_read(this->fi, &packet) && (!done))
    {
        if(this->fi == NULL) this->DoOpenFile();
        if(this->streams.empty())
        {
            this->OpenStreams();
        }

        AVbinTimestamp &timestamp = packet.timestamp;
        AVbinStreamInfo *sinfo = this->streamInfos[packet.stream_index];
        AVbinStream *stream = this->streams[packet.stream_index];

        cout << "Packet of stream " << packet.stream_index << " at " << timestamp
             << " type=" << sinfo->type << endl;

        int inRange = (timestamp >= startTime and timestamp < endTime);
        //cout <<inRange<< endl;

        //Check that every channel is beyond the end of the stream
        timestampOfChannel[packet.stream_index] = timestamp;
        int checkDone = 1;
        for(unsigned int chanNum = 0;chanNum < this->numStreams;chanNum++)
        {
            if(timestampOfChannel[chanNum] < endTime) checkDone = 0;
        }
        done = checkDone;

        //Decode video packet
        if(sinfo->type == AVBIN_STREAM_TYPE_VIDEO)
        {
            uint8_t* buff = new uint8_t[sinfo->video.width*sinfo->video.height*3];
            int32_t loop = 0;
            if (loop != -1)
            {
                loop = avbin_decode_video(stream, packet.data, packet.size, &*buff);
            }

            if(inRange)
            {
                std::tr1::shared_ptr<class DecodedFrame> frame(new class DecodedFrame);
                frame->buff = buff;
                frame->height = sinfo->video.height;
                frame->width = sinfo->video.width;
                frame->sample_aspect_num = sinfo->video.sample_aspect_num;
                frame->sample_aspect_den = sinfo->video.sample_aspect_den;
                frame->frame_rate_num = sinfo->video.frame_rate_num;
                frame->frame_rate_den = sinfo->video.frame_rate_den;
                frame->timestamp = timestamp - this->info.start_time;
                videoOut.push_back(frame);
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
    cout << "videoOut" << videoOut.size() << endl;
    out->frames = videoOut;
    return out;
}


int AvBinBackend::GetFrame(int64_t time, class DecodedFrame &out)
{
    //Remove start offset
    time -= this->info.start_time;

    //Seek in file
    AVbinResult res = avbin_seek_file(this->fi, time);
    assert(res == AVBIN_RESULT_OK);

    //Decode the packets
    AVbinPacket packet;
    packet.structure_size = sizeof(packet);
    int done = false;

    vector<int64_t> timestampOfChannel;
    for(unsigned int chanNum=0;chanNum<this->numStreams;chanNum++)
        timestampOfChannel.push_back(-1);

    int debug = 0;

    while (!avbin_read(this->fi, &packet) && (!done))
    {
        if(this->fi == NULL) this->DoOpenFile();
        if(this->streams.empty())
        {
            this->OpenStreams();
        }

        AVbinTimestamp &timestamp = packet.timestamp;
        AVbinStreamInfo *sinfo = this->streamInfos[packet.stream_index];
        AVbinStream *stream = this->streams[packet.stream_index];

        cout << "Packet of stream " << packet.stream_index << " at " << timestamp
             << " type=" << sinfo->type;
        if(sinfo->type == AVBIN_STREAM_TYPE_VIDEO) cout << " video";
        if(sinfo->type == AVBIN_STREAM_TYPE_AUDIO) cout << " audio";
        cout << endl;

        //Allocate video buffer
        if(out.buff == NULL && sinfo->type == AVBIN_STREAM_TYPE_VIDEO)
        {
            out.buffSize = sinfo->video.width*sinfo->video.height*3;
            uint8_t *buff = new uint8_t[out.buffSize];
            cout << "Creating buffer at " << (unsigned long long) &*buff <<
                    " of size " << out.buffSize<<endl;
            cout << sinfo->video.width <<","<<sinfo->video.height << endl;
            out.buff = buff;
        }

        //Decode video packet
        if(sinfo->type == AVBIN_STREAM_TYPE_VIDEO)
        {
            assert(out.buffSize>0);
            int32_t loop = 0;
            if (loop != -1)
                loop = avbin_decode_video(stream, packet.data, packet.size, &*out.buff);

            out.height = sinfo->video.height;
            out.width = sinfo->video.width;
            out.sample_aspect_num = sinfo->video.sample_aspect_num;
            out.sample_aspect_den = sinfo->video.sample_aspect_den;
            out.frame_rate_num = sinfo->video.frame_rate_num;
            out.frame_rate_den = sinfo->video.frame_rate_den;
            out.timestamp = timestamp - this->info.start_time;

            done = 1;

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
    return 1;
}

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
