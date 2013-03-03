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
#include "localmutex.h"

class AvBinBackend
{
    /*!
    * AvBinBackend uses the avbin interface to decode video files. The
    * stream of images and audio blocks are decoded and are returned in
    * RGB format. Seeking in files is also supported.
    */

public:
    AvBinBackend();
    AvBinBackend(const AvBinBackend &other);
    ~AvBinBackend();

    int GetBackendVersion();
    int OpenFile(const char *filename, int requestId = 0);
    void CloseFile();
    int64_t Length();

    //Frame based retrieval
    int GetFrame(uint64_t time, class DecodedFrame &out);

    //Replay retrieval
    int PlayUpdate(); //Returns true if playing is active
    void HandleEvent(std::tr1::shared_ptr<class Event> ev);

    void OpenStreams();
    void OpenStream(int in);
    void CloseStreams();
    void DoOpenFile(int requestId = 0);
    void PrintAVbinFileInfo(AVbinFileInfo &info);
    void PrintAVbinStreamInfo(AVbinStreamInfo &info);
    void SetEventLoop(class EventLoop *eventLoopIn);
    void SetUuid(QUuid idIn);

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
    QUuid uuid;
};

//************************************************

class AvBinThread : public MessagableThread
{
    /*!
    * AvBinThread decodes video while not iterrupting GUI behaviour.
    * This repeatedly calls into AvBinBackend to do the actual work.
    * AvBinMedia provides a higher level interface to retrieve frames.
    */

public:
    AvBinThread();
    virtual ~AvBinThread();
    void SetEventLoop(class EventLoop *eventLoopIn);
    void SetUuid(QUuid idIn);

    void Update();
    void Finished();
protected:
    class AvBinBackend avBinBackend;
};


#endif // AVBINBACKEND_H
