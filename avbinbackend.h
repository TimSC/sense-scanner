#ifndef AVBINBACKEND_H
#define AVBINBACKEND_H

#include <vector>
#include <string>
#include <tr1/memory>
#include <tr1/functional>
#include "mediabuffer.h"
#include "eventloop.h"
#include <sstream>

extern "C"
{
#include <avbin.h>
}

class FrameGroup
{
public:
    std::vector<std::tr1::shared_ptr<class DecodedFrame> > frames;
    int64_t start, end;
};

typedef std::tr1::function<void (const class DecodedFrame&)> FrameCallback;

class AvBinBackend
{
public:
    AvBinBackend();
    ~AvBinBackend();

    int OpenFile(const char *filename);
    void CloseFile();
    int64_t Length();

    //Frame based retrieval
    std::tr1::shared_ptr<class FrameGroup> GetFrameRange(int64_t startTime, int64_t endTime);
    int GetFrame(int64_t time, class DecodedFrame &out);

    //Replay retrieval
    int Play(int64_t startTime, FrameCallback &frame);
    int Pause();
    int Stop();
    int PlayUpdate(); //Returns true if playing is active
    void HandleEvent(class Event &ev);

    void OpenStreams();
    void CloseStreams();
    void DoOpenFile();
    void PrintAVbinFileInfo(AVbinFileInfo &info);
    void PrintAVbinStreamInfo(AVbinStreamInfo &info);
    void SetEventLoop(QSharedPointer<class EventLoop> &eventLoopIn);

protected:
    AVbinFile *fi;
    int32_t numStreams;
    std::vector<AVbinStreamInfo *> streamInfos;
    std::vector<AVbinStream *> streams;
    AVbinFileInfo info;
    std::string filename;
    class EventReceiver eventReceiver;
    QSharedPointer<class EventLoop> eventLoop;

};

#endif // AVBINBACKEND_H
