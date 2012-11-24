#ifndef AVBINBACKEND_H
#define AVBINBACKEND_H

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

    void PrintAVbinFileInfo(AVbinFileInfo &info);
    void PrintAVbinStreamInfo(AVbinStreamInfo &info);
protected:
    AVbinFile *fi;
    int32_t numStreams;

};

#endif // AVBINBACKEND_H
