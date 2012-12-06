#ifndef AVBINBACKEND_H
#define AVBINBACKEND_H

#include <vector>
#include <string>
#ifdef _MSC_VER
	#include <memory>
#else
	#include <tr1/memory>
#endif
#include "mediabuffer.h"
#include "eventloop.h"
#include <sstream>
#include <vector>
#include <list>
#include "avbinapi.h"

class FrameGroup
{
public:
    std::vector<std::tr1::shared_ptr<class DecodedFrame> > frames;
    int64_t start, end;
};

class AvBinBackend
{
public:
    AvBinBackend();
    AvBinBackend(const AvBinBackend &other);
    ~AvBinBackend();

    int OpenFile(const char *filename, int requestId = 0);
    void CloseFile();
    int64_t Length();

    //Frame based retrieval
    int GetFrame(uint64_t time, class DecodedFrame &out);

    //Replay retrieval
    int PlayUpdate(); //Returns true if playing is active
    void HandleEvent(std::tr1::shared_ptr<class Event> ev);

    void OpenStreams();
    void CloseStreams();
    void DoOpenFile(int requestId = 0);
    void PrintAVbinFileInfo(AVbinFileInfo &info);
    void PrintAVbinStreamInfo(AVbinStreamInfo &info);
    void SetEventLoop(class EventLoop *eventLoopIn);

protected:
    AVbinFile *fi;
    int32_t numStreams;
    std::vector<AVbinStreamInfo *> streamInfos;
    std::vector<AVbinStream *> streams;
    AVbinFileInfo info;
    std::string filename;
    class EventReceiver *eventReceiver;
    class EventLoop *eventLoop;
    unsigned height, width;
    int firstVideoStream, firstAudioStream;

    std::vector<uint64_t> timestampOfChannel;
    class DecodedFrame currentFrame;
    class DecodedFrame prevFrame;
    std::list<class DecodedFrame> frames;
    std::vector<class DecodedFrame *> firstFrames;
    std::list<std::tr1::shared_ptr<class Event> > incomingFrameRequests;

    uint8_t *audioBuffer;
    unsigned audioBufferSize;
};

#endif // AVBINBACKEND_H
