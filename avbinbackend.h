#ifndef AVBINBACKEND_H
#define AVBINBACKEND_H

#include <vector>
#include <string.h>
extern "C"
{
#include <avbin.h>
}

class AvBinBackend
{
public:
    AvBinBackend();
    ~AvBinBackend();

    int OpenFile(const char *filename);
    void CloseFile();

    void PrintAVbinFileInfo(AVbinFileInfo &info);
    void PrintAVbinStreamInfo(AVbinStreamInfo &info);
protected:
    AVbinFile *fi;
    int32_t numStreams;
    std::vector<AVbinStreamInfo *> streamInfos;
    std::vector<AVbinStream *> streams;

};

#endif // AVBINBACKEND_H
