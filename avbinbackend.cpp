#include "avbinbackend.h"
#include <iostream>
#include <assert.h>
#include <sstream>
#include "localints.h"
using namespace std;
#define DEFAULT_AUDIO_BUFF_SIZE 1024*1024
#define SEEK_BEFORE_MARGIN 100000 //Seek this time before desired frame, microsec
#define DECODE_INITIAL_DURATION 0 //Duration to decode when file is loaded
#define SEEK_TOLERANCE 1000000 //Don't seek but use playback for frames less than this time

//**********************************************************

AvBinBackend::AvBinBackend()
{
    AVbinResult res = mod_avbin_init();
    assert(res == AVBIN_RESULT_OK);
    this->numStreams = -1;
    this->fi = NULL;
    this->height = 0;
    this->width = 0;
    this->firstVideoStream = -1;
    this->firstAudioStream = -1;
    this->eventReceiver = NULL;
    this->eventLoop = NULL;
    this->audioBuffer = NULL;
    this->audioBufferSize = 0;
    this->id = 0;
}

AvBinBackend::AvBinBackend(const AvBinBackend &other)
{
    assert(0); //This should never happen
}

AvBinBackend::~AvBinBackend()
{
    cout << "AvBinBackend::~AvBinBackend() " << (unsigned long long)this << endl;
    this->CloseFile();
    if(this->eventReceiver) delete this->eventReceiver;
    this->eventReceiver = NULL;

    if(this->audioBuffer) delete [] this->audioBuffer;
    this->audioBuffer = NULL;

    for(unsigned int i=0;i<this->firstFrames.size();i++)
    {
        if(this->firstFrames[i]) delete this->firstFrames[i];
        this->firstFrames[i] = NULL;
    }
    this->firstFrames.clear();
}

int AvBinBackend::OpenFile(const char *filenameIn, int requestId)
{
    assert(this->fi == NULL);

    this->filename = filenameIn;
    if(strlen(filenameIn) > 0)
        this->DoOpenFile();
    return 1;
}

void AvBinBackend::DoOpenFile(int requestId)
{
    assert(this->fi == NULL);
    assert(this->streams.size() == 0);

    //Open file
    //This function is not thread safe, so lock a protection mutex
    this->fi = mod_avbin_open_filename(this->filename.c_str());

    //Create an event with the result
    QString eventName = QString("AVBIN_OPEN_RESULT%1").arg(this->id);
    std::tr1::shared_ptr<class Event> resultEvent(new Event(eventName.toLocal8Bit().constData(), requestId));
    assert(this->eventLoop);
    std::ostringstream tmp;
    tmp << (this->fi != NULL);
    resultEvent->data = tmp.str();
    this->eventLoop->SendEvent(resultEvent);

    if(this->fi == NULL)
    {
        //File open failed
        this->filename = "";
        return;
    }

    //Get file info from avbin
    this->info.structure_size = sizeof(AVbinFileInfo);
    mod_avbin_file_info(this->fi, &this->info);
    //this->PrintAVbinFileInfo(this->info);
    this->numStreams = this->info.n_streams;
    this->firstVideoStream = -1;
    this->firstAudioStream = -1;

    //Initialise first frame storage
    assert(this->firstFrames.size()==0);
    for(int32_t i = 0; i<numStreams; i++)
        this->firstFrames.push_back(NULL);

    //Get info for each stream
    for(int32_t i = 0; i<numStreams; i++)
    {
        AVbinStreamInfo *sinfo = new AVbinStreamInfo;
        sinfo->structure_size = sizeof(AVbinStreamInfo);
        mod_avbin_stream_info(this->fi, i, sinfo);
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
        if(sinfo->type == AVBIN_STREAM_TYPE_VIDEO)
        {
            assert(!this->firstFrames[i]);
            this->firstFrames[i] = new DecodedFrame;
        }
    }

    //Prepare timestamp storage
    this->timestampOfChannel.clear();
    for(int chanNum=0;chanNum<this->numStreams;chanNum++)
        this->timestampOfChannel.push_back(0);

    //Open stream decoders
    if(this->streams.empty())
        this->OpenStreams();

    //Load some frames to get the video decoders to behave correctly
    AVbinPacket packet;
    packet.structure_size = sizeof(packet);
    int done = false;
    int processing = 1;
    while (processing && (!done))
    {
        //Read frame from file
        int readRet = mod_avbin_read(this->fi, &packet);
        if(readRet == -1)
        {
            processing = 0;
            continue;
        }

        AVbinTimestamp &timestamp = packet.timestamp;
        assert(timestamp >= 0);
        AVbinStreamInfo *sinfo = this->streamInfos[packet.stream_index];
        AVbinStream *stream = this->streams[packet.stream_index];

        //Check if all streams have received some data
        done = true;
        for(unsigned int chanNum=0;chanNum<this->timestampOfChannel.size();chanNum++)
        {
            if(this->timestampOfChannel[chanNum] == 0) done = false;
            if(this->timestampOfChannel[chanNum] < DECODE_INITIAL_DURATION) done = false;
            if(this->firstFrames[chanNum] &&
                    this->firstFrames[chanNum]->endTimestamp == 0) done = false;
        }

        //Decode video packet
        if(sinfo->type == AVBIN_STREAM_TYPE_VIDEO)
        {
            unsigned requiredBuffSize = sinfo->video.width*sinfo->video.height*3;
            if ((this->currentFrame.buff)==NULL || requiredBuffSize != this->currentFrame.buffSize)
                this->currentFrame.AllocateSize(requiredBuffSize);

            assert(this->currentFrame.buff);
            int32_t ret = mod_avbin_decode_video(stream, packet.data, packet.size, this->currentFrame.buff);
            int error = (ret == -1);
            if(!error)
            {
                //If first frame is set, but not ending timestamp, do so now
                if(this->firstFrames[packet.stream_index]->buffSize != 0
                        && this->firstFrames[packet.stream_index]->endTimestamp == 0)
                {
                    this->firstFrames[packet.stream_index]->endTimestamp = timestamp - this->info.start_time;
                }

                //Check if this is the first frame of stream
                if(this->firstFrames[packet.stream_index]->buffSize == 0)
                {
                    cout << "First frame found at " << timestamp << endl;
                    //this->currentFrame
                    this->currentFrame.height = sinfo->video.height;
                    this->currentFrame.width = sinfo->video.width;
                    this->currentFrame.sample_aspect_num = sinfo->video.sample_aspect_num;
                    this->currentFrame.sample_aspect_den = sinfo->video.sample_aspect_den;
                    this->currentFrame.frame_rate_num = sinfo->video.frame_rate_num;
                    this->currentFrame.frame_rate_den = sinfo->video.frame_rate_den;
                    this->currentFrame.timestamp = timestamp - this->info.start_time;
                    *this->firstFrames[packet.stream_index] = this->currentFrame;
                }

                this->timestampOfChannel[packet.stream_index] = timestamp;
            }
        }

        //Decode audio packet
        if(sinfo->type == AVBIN_STREAM_TYPE_AUDIO)
        {
            //Allocate audio buffer, if not already done
            if(this->audioBuffer==NULL || this->audioBufferSize==0)
            {
                this->audioBufferSize = DEFAULT_AUDIO_BUFF_SIZE;
                this->audioBuffer = new uint8_t[this->audioBufferSize];
            }
            assert(this->audioBuffer);

            int bytesleft = this->audioBufferSize;
            int bytesout = bytesleft;
            int bytesread = 0;
            uint8_t *cursor = this->audioBuffer;
            while ((bytesread = mod_avbin_decode_audio(stream, packet.data, packet.size, cursor, &bytesout)) > 0)
            {
                packet.data += bytesread;
                packet.size -= bytesread;
                cursor += bytesout;
                bytesleft -= bytesout;
                bytesout = bytesleft;
            }

            this->timestampOfChannel[packet.stream_index] = timestamp;
        }
    }


    //Return to the start
    AVbinResult res = mod_avbin_seek_file(this->fi, 0);
    assert(res == AVBIN_RESULT_OK);
}

int AvBinBackend::GetFrame(uint64_t time, class DecodedFrame &out)
{
    if(this->fi == NULL)
        return 0;

    assert(this->fi != NULL);
    assert(this->firstVideoStream >= 0);
    assert(this->firstVideoStream < (int)this->timestampOfChannel.size());

    //Apply start offset
    time += this->info.start_time;

    //Check if first frame is suitable
    class DecodedFrame *test = this->firstFrames[this->firstVideoStream];
    if(test && test->buff && time < test->timestamp)
    {
        out = *test;
        return 1;
    }

    //Check if currently buffered frame is suitable for the response
    if(this->currentFrame.timestamp > 0 && this->prevFrame.timestamp > 0)
    {
        if(time >= this->prevFrame.timestamp
                && time < this->currentFrame.timestamp)
        {
            //Used cached frame
            out = this->prevFrame;
            return 1;
        }
    }

    //Check if the requested from is close to requested frame
    //so that seeking is unnecessary
    //If current currentVidTime is zero, that is an unknown position so we always seek
    uint64_t currentVidTime = this->timestampOfChannel[this->firstVideoStream];
    int doSeek = 1;

    if(currentVidTime > 0 && time >= currentVidTime)
    {
        uint64_t diff = time - currentVidTime;
        if(diff < SEEK_TOLERANCE + SEEK_BEFORE_MARGIN) doSeek = 0;
    }

    if(doSeek)
    {
        //Seek in file
        uint64_t seekToTime = time;
        if(seekToTime >= SEEK_BEFORE_MARGIN)
            seekToTime -= SEEK_BEFORE_MARGIN; //Seek to a point well before target frame
        else
            seekToTime = 0;

        cout << "Seek to time" << seekToTime << endl;
        AVbinResult res = mod_avbin_seek_file(this->fi, seekToTime);
        assert(res == AVBIN_RESULT_OK);
        for(int chanNum=0;chanNum<this->numStreams;chanNum++)
            this->timestampOfChannel[chanNum] = 0;
        this->currentFrame.width = 0;
        this->currentFrame.height = 0;
        this->currentFrame.timestamp = 0;
        this->prevFrame.endTimestamp = 0;
        this->prevFrame.width = 0;
        this->prevFrame.height = 0;
        this->prevFrame.timestamp = 0;
        this->prevFrame.endTimestamp = 0;
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
        int readRet = mod_avbin_read(this->fi, &packet);
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
        assert(timestamp >= 0);
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
                (int)packet.stream_index == this->firstVideoStream)
        {
            //cout << "Decoded: " << this->timestampOfChannel[packet.stream_index] << endl;
            unsigned requiredBuffSize = sinfo->video.width*sinfo->video.height*3;

            assert(this->currentFrame.buff);
            int32_t ret = mod_avbin_decode_video(stream, packet.data, packet.size, this->currentFrame.buff);
            int error = (ret == -1);
            //cout << "z" << (timestamp > time && this->currentFrame.width > 0) << error<<endl;

            //Check if this frame is after the requested time and stop
            //processing frames if that is the case
            //if(!error)
            //    cout << "found:" <<timestamp << " req:"<< time << endl;
            if((uint64_t)timestamp > time && this->currentFrame.width > 0 && !error)
            {
                if ((out.buff)==NULL || requiredBuffSize != out.buffSize)
                    out.AllocateSize(requiredBuffSize);
                assert(out.buff);
                assert(out.buffSize > 0);

                //Set end time of current frame
                this->currentFrame.endTimestamp = timestamp;

                out = this->currentFrame;
                if(time < out.timestamp)
                    cout << "Warning: found frame after requested time" << endl;
                assert(out.width > 0);
                assert(out.height > 0);
                //cout << "stopping search. current timestamp" << this->currentFrame.timestamp << endl;
                //cout << "prev timestamp" << this->prevFrame.timestamp << endl;

                done = 1;
            }

            if(!error)
            {
                //Swap forward and back render buffers
                //The current frame now becomes the "previous frame"
                this->prevFrame.FastSwap(this->currentFrame);

                //Allocate video buffer if necessary
                if ((this->currentFrame.buff)==NULL || requiredBuffSize != this->currentFrame.buffSize)
                {
                    this->currentFrame.AllocateSize(requiredBuffSize);
                }
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
                this->currentFrame.endTimestamp = 0;
                this->currentFrame.requestedTimestamp = time;
                if(out.height>0) this->height = out.height;
                if(out.width>0) this->width = out.width;
            }
        }

        //Decode audio packet
        if(sinfo->type == AVBIN_STREAM_TYPE_AUDIO &&
                (int)packet.stream_index == this->firstAudioStream)
        {
            //Allocate audio buffer, if not already done
            if(this->audioBuffer==NULL || this->audioBufferSize==0)
            {
                this->audioBufferSize = DEFAULT_AUDIO_BUFF_SIZE;
                this->audioBuffer = new uint8_t[this->audioBufferSize];
            }
            assert(this->audioBuffer);

            int bytesleft = this->audioBufferSize;
            int bytesout = bytesleft;
            int bytesread = 0;
            uint8_t *cursor = this->audioBuffer;
            while ((bytesread = mod_avbin_decode_audio(stream, packet.data, packet.size, cursor, &bytesout)) > 0)
            {
                packet.data += bytesread;
                packet.size -= bytesread;
                cursor += bytesout;
                bytesleft -= bytesout;
                bytesout = bytesleft;
            }

            //int totalBytes = cursor-buff;
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

    if(this->eventReceiver==NULL)
    {
        this->eventReceiver = new class EventReceiver(eventLoopIn);
        this->eventLoop = eventLoopIn;
        QString eventName = QString("AVBIN_OPEN_FILE%1").arg(this->id);
        this->eventLoop->AddListener(eventName.toLocal8Bit().constData(), *this->eventReceiver);
        QString eventName2 = QString("AVBIN_GET_DURATION%1").arg(this->id);
        this->eventLoop->AddListener(eventName2.toLocal8Bit().constData(), *this->eventReceiver);
        QString eventName3 = QString("AVBIN_GET_FRAME%1").arg(this->id);
        this->eventLoop->AddListener(eventName3.toLocal8Bit().constData(), *this->eventReceiver);
    }
}

void AvBinBackend::SetId(int idIn)
{
    this->id = idIn;
}

int AvBinBackend::PlayUpdate()
{
    int foundEvent = 0;

    //Read incoming events into a local list
    try
    {
        assert(this->eventReceiver);
        //cout << "avbin thread queue " << this->eventReceiver->BufferSize() << endl;
        while(1)
        {
        std::tr1::shared_ptr<class Event> ev = this->eventReceiver->PopEvent();
        //cout << "Event type " << ev->type << endl;
        foundEvent++;
        QString evType = ev->type.c_str();
        if(evType.left(15) == "AVBIN_GET_FRAME")
            this->incomingFrameRequests.push_back(ev);
        else
            this->HandleEvent(ev);
        }
    }
    catch(std::runtime_error e) {}

    //Process all frames in queue
    //std::list<std::tr1::shared_ptr<class Event> >::iterator it;
    //cout << foundEvent << endl;
    //for(it=this->incomingFrameRequests.begin();it!=this->incomingFrameRequests.end();it++)
    //{
    //    this->HandleEvent(*it);
    //}

    if(!this->incomingFrameRequests.empty())
    {
        //Only process the most recent request, discard the others
        std::list<std::tr1::shared_ptr<class Event> >::iterator it;
        it = this->incomingFrameRequests.end();
        it --;
        this->HandleEvent(*it);
    }

    this->incomingFrameRequests.clear();

    return foundEvent > 0;
}

void AvBinBackend::HandleEvent(std::tr1::shared_ptr<class Event> ev)
{
    QString evType(ev->type.c_str());
    if(evType.left(15)=="AVBIN_OPEN_FILE")
    {
        this->OpenFile(ev->data.c_str(), ev->id);
    }
    if(evType.left(18)=="AVBIN_GET_DURATION")
    {
        assert(this->eventLoop);
        try
        {
            QString eventName = QString("AVBIN_DURATION_RESPONSE%1").arg(this->id);
            std::tr1::shared_ptr<class Event> response(
                        new Event(eventName.toLocal8Bit().constData(), ev->id));
            std::ostringstream tmp;
            tmp << this->Length();
            response->data = tmp.str();
            this->eventLoop->SendEvent(response);
        }
        catch (runtime_error &err)
        {
            QString eventName = QString("AVBIN_REQUEST_FAILED%1").arg(this->id);
            std::tr1::shared_ptr<class Event> fail(new Event(eventName.toLocal8Bit().constData(), ev->id));
            fail->data = err.what();
            this->eventLoop->SendEvent(fail);
        }
    }
    if(evType.left(15)=="AVBIN_GET_FRAME")
    {
        unsigned long long ti = STR_TO_ULL(ev->data.c_str(),NULL,10);
        QString eventName = QString("AVBIN_FRAME_RESPONSE%1").arg(this->id);
        std::tr1::shared_ptr<class Event> response(new Event(eventName.toLocal8Bit().constData(), ev->id));
        class DecodedFrame* decodedFrame = new DecodedFrame();
        response->raw = decodedFrame;
        int found = this->GetFrame(ti, *decodedFrame);
        if(found)
        {
            //Send decoded image as event
            assert(decodedFrame->buff != NULL);
            assert(decodedFrame->buffSize > 0);
            assert(decodedFrame->width > 0);
            assert(decodedFrame->height > 0);
            this->eventLoop->SendEvent(response);
        }
        else
        {
            //Something went wrong, so a failure event is generated
            QString eventName = QString("AVBIN_FRAME_FAILED%1").arg(this->id);
            std::tr1::shared_ptr<class Event> fail(new Event(eventName.toLocal8Bit().constData(), ev->id));
            this->eventLoop->SendEvent(fail);
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
        AVbinStream *stream = mod_avbin_open_stream(this->fi, i);
        this->streams.push_back(stream);
    }
}

void AvBinBackend::CloseStreams()
{
    for(unsigned int i =0; i < this->streams.size(); i++)
        mod_avbin_close_stream(this->streams[i]);
    this->streams.clear();
}

void AvBinBackend::CloseFile()
{
    this->CloseStreams();

    for(unsigned int i =0; i < this->streamInfos.size(); i++)
        delete this->streamInfos[i];
    this->streamInfos.clear();

    if(this->fi)
        mod_avbin_close_file(this->fi);
    this->fi = NULL;
}

int64_t AvBinBackend::Length()
{
    if(this->fi == NULL)
        throw runtime_error("File not open");

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

//*************************************************

AvBinThread::AvBinThread() : MessagableThread()
{

}

AvBinThread::~AvBinThread()
{

}

void AvBinThread::Update()
{

    //cout << "AvBinThread::Update()" << this->id <<"\t"<< (unsigned long)this << endl;

    int foundEvent = 0;

    //Update the backend to actually do something useful
    foundEvent = this->avBinBackend.PlayUpdate();

    if(!foundEvent)
        msleep(10);
    else
        msleep(0);
}

void AvBinThread::SetEventLoop(class EventLoop *eventLoopIn)
{
    this->avBinBackend.SetEventLoop(eventLoopIn);
    MessagableThread::SetEventLoop(eventLoopIn);
}

void AvBinThread::SetId(int idIn)
{
    this->avBinBackend.SetId(idIn);
    MessagableThread::SetId(idIn);
}

void AvBinThread::Finished()
{
    cout << "AvBinThread::Finished" << endl;
}
