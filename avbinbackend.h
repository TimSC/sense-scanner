#ifndef AVBINBACKEND_H
#define AVBINBACKEND_H

#include <vector>
#include <string>
#include <tr1/memory>

extern "C"
{
#include <avbin.h>
}

class DecodedFrame
{
public:
    std::tr1::shared_ptr<uint8_t> buff;
    unsigned int height, width;
    unsigned int sample_aspect_num, sample_aspect_den;
    unsigned int frame_rate_num, frame_rate_den;
    int64_t timestamp;
};

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
    ~AvBinBackend();

    int OpenFile(const char *filename);
    void CloseFile();
    std::tr1::shared_ptr<class FrameGroup> GetFrameRange(int64_t startTime, int64_t endTime);
    int64_t Length();

    void OpenStreams();
    void CloseStreams();
    void DoOpenFile();
    void PrintAVbinFileInfo(AVbinFileInfo &info);
    void PrintAVbinStreamInfo(AVbinStreamInfo &info);
protected:
    AVbinFile *fi;
    int32_t numStreams;
    std::vector<AVbinStreamInfo *> streamInfos;
    std::vector<AVbinStream *> streams;
    AVbinFileInfo info;
    std::string filename;

};

#endif // AVBINBACKEND_H
