#include <assert.h>
#include "avbinapi.h"
#include <QtCore/QMutex>
#ifdef _MSC_VER
#include <Windows.h>
#endif

QMutex avbinOpenMutex;
int gavBinInitCompleted = 0;

#ifndef _MSC_VER

AVbinResult mod_avbin_init()
{
    avbinOpenMutex.lock();
    if(!gavBinInitCompleted)
    {
        AVbinResult ret = avbin_init();
        if(ret == AVBIN_RESULT_OK)
            gavBinInitCompleted = 1;

        avbinOpenMutex.unlock();
        return ret;
    }
    avbinOpenMutex.unlock();
    return AVBIN_RESULT_OK;
}

int32_t mod_avbin_get_version()
{
    avbinOpenMutex.lock();
    int32_t ver = avbin_get_version();
    avbinOpenMutex.unlock();
    return ver;
}

AVbinFile* mod_avbin_open_filename(const char *filename)
{
    avbinOpenMutex.lock();
    AVbinFile *fi = avbin_open_filename(filename);
    avbinOpenMutex.unlock();
    return fi;
}

AVbinResult mod_avbin_file_info(AVbinFile *file, AVbinFileInfo *info)
{
    avbinOpenMutex.lock();
    AVbinResult out = avbin_file_info(file, info);
    avbinOpenMutex.unlock();
    return out;
}

AVbinResult mod_avbin_stream_info(AVbinFile *file, int stream_index, AVbinStreamInfo *info)
{
    avbinOpenMutex.lock();
    AVbinResult out = avbin_stream_info(file, stream_index, info);
    avbinOpenMutex.unlock();
    return out;
}

AVbinResult mod_avbin_seek_file(AVbinFile *file, AVbinTimestamp timestamp)
{
    avbinOpenMutex.lock();
    AVbinResult ret = avbin_seek_file(file, timestamp);
    avbinOpenMutex.unlock();
    return ret;
}

AVbinResult mod_avbin_read(AVbinFile *file, AVbinPacket *packet)
{
    avbinOpenMutex.lock();
    AVbinResult out = avbin_read(file, packet);
    avbinOpenMutex.unlock();
    return out;
}

int mod_avbin_decode_video(AVbinStream *stream, uint8_t *data_in, size_t size_in, uint8_t *data_out)
{
    avbinOpenMutex.lock();
    int out = avbin_decode_video(stream, data_in, size_in, data_out);
    avbinOpenMutex.unlock();
    return out;
}

int mod_avbin_decode_audio(AVbinStream *stream, uint8_t *data_in, size_t size_in, uint8_t *data_out, int *size_out)
{
    avbinOpenMutex.lock();
    int out = avbin_decode_audio(stream, data_in, size_in, data_out, size_out);
    avbinOpenMutex.unlock();
    return out;
}
AVbinStream* mod_avbin_open_stream(AVbinFile *file, int stream_index)
{
    avbinOpenMutex.lock();
    AVbinStream* ret = avbin_open_stream(file, stream_index);
    avbinOpenMutex.unlock();
    return ret;
}
void mod_avbin_close_stream(AVbinStream *stream)
{
    avbinOpenMutex.lock();
	avbin_close_stream(stream);
    avbinOpenMutex.unlock();
}

void mod_avbin_close_file(AVbinFile *file)
{
    avbinOpenMutex.lock();
	avbin_close_file(file);
    avbinOpenMutex.unlock();
}

#endif //_MSC_VER
//******************************************************
#ifdef _MSC_VER
#define WIN32DLL_API __declspec(dllimport)

HINSTANCE ghinst = 0;

AVbinResult mod_avbin_init()
{
    avbinOpenMutex.lock();
	if(ghinst==0) ghinst = LoadLibrary("avbin11-32.dll");
	assert(ghinst != NULL);
	
    if(!gavBinInitCompleted)
    {
        FARPROC init = GetProcAddress ( ghinst , "avbin_init" );
        assert(init != (FARPROC)NULL);

        AVbinResult (*func)()=0;
        func = (AVbinResult (*)()) init;
        AVbinResult ret = (*func)();
        if(ret == AVBIN_RESULT_OK)
            gavBinInitCompleted = 1;
        avbinOpenMutex.unlock();
        return ret;
    }
    avbinOpenMutex.unlock();
    return AVBIN_RESULT_OK;
}

AVbinFile* mod_avbin_open_filename(const char *filename)
{
    avbinOpenMutex.lock();
	assert(ghinst != NULL);
	FARPROC rawfunc = GetProcAddress ( ghinst , "avbin_open_filename" );
	assert(rawfunc != (FARPROC)NULL);

	AVbinFile* (*func)(const char *filename)=0;
	func = (AVbinFile* (*)(const char *filename)) rawfunc;
    AVbinFile*out = (*func)(filename);
    avbinOpenMutex.unlock();
    return out;
}

AVbinResult mod_avbin_file_info(AVbinFile *file, AVbinFileInfo *info)
{
    avbinOpenMutex.lock();
	assert(ghinst != NULL);
	FARPROC rawfunc = GetProcAddress ( ghinst , "avbin_file_info" );
	assert(rawfunc != (FARPROC)NULL);

	AVbinResult (*func)(AVbinFile *, AVbinFileInfo *)=0;
	func = (AVbinResult (*)(AVbinFile *, AVbinFileInfo *)) rawfunc;
    AVbinResult out = (*func)(file, info);
    avbinOpenMutex.unlock();
    return out;
}

AVbinResult mod_avbin_stream_info(AVbinFile *file, int stream_index, AVbinStreamInfo *info)
{
    avbinOpenMutex.lock();
	assert(ghinst != NULL);
	FARPROC rawfunc = GetProcAddress ( ghinst , "avbin_stream_info" );
	assert(rawfunc != (FARPROC)NULL);

	AVbinResult (*func)(AVbinFile *, int, AVbinStreamInfo *)=0;
	func = (AVbinResult (*)(AVbinFile *, int, AVbinStreamInfo *)) rawfunc;
    AVbinResult out = (*func)(file, stream_index, info);
    avbinOpenMutex.unlock();
    return out;
}

AVbinResult mod_avbin_seek_file(AVbinFile *file, AVbinTimestamp timestamp)
{
    avbinOpenMutex.lock();
	assert(ghinst != NULL);
	FARPROC rawfunc = GetProcAddress ( ghinst , "avbin_seek_file" );
	assert(rawfunc != (FARPROC)NULL);

	AVbinResult (*func)(AVbinFile *, AVbinTimestamp)=0;
	func = (AVbinResult (*)(AVbinFile *, AVbinTimestamp)) rawfunc;
    AVbinResult out = (*func)(file, timestamp);
    avbinOpenMutex.unlock();
    return out;
}

AVbinResult mod_avbin_read(AVbinFile *file, AVbinPacket *packet)
{
    avbinOpenMutex.lock();
	assert(ghinst != NULL);
	FARPROC rawfunc = GetProcAddress ( ghinst , "avbin_read" );
	assert(rawfunc != (FARPROC)NULL);

	AVbinResult (*func)(AVbinFile *, AVbinPacket *)=0;
	func = (AVbinResult (*)(AVbinFile *, AVbinPacket *)) rawfunc;
    AVbinResult out = (*func)(file, packet);
    avbinOpenMutex.unlock();
    return out;
}

int mod_avbin_decode_video(AVbinStream *stream, uint8_t *data_in, size_t size_in, uint8_t *data_out)
{
    avbinOpenMutex.lock();
	assert(ghinst != NULL);
	FARPROC rawfunc = GetProcAddress ( ghinst , "avbin_decode_video" );
	assert(rawfunc != (FARPROC)NULL);

	int (*func)(AVbinStream *, uint8_t *, size_t, uint8_t *)=0;
	func = (int (*)(AVbinStream *, uint8_t *, size_t, uint8_t *)) rawfunc;
    int out = (*func)(stream, data_in, size_in, data_out);
    avbinOpenMutex.unlock();
    return out;
}

int mod_avbin_decode_audio(AVbinStream *stream, uint8_t *data_in, size_t size_in, uint8_t *data_out, int *size_out)
{
    avbinOpenMutex.lock();
	assert(ghinst != NULL);
	FARPROC rawfunc = GetProcAddress ( ghinst , "avbin_decode_audio" );
	assert(rawfunc != (FARPROC)NULL);

	int (*func)(AVbinStream *, uint8_t *, size_t, uint8_t *, int *)=0;
	func = (int (*)(AVbinStream *, uint8_t *, size_t, uint8_t *, int *)) rawfunc;
    int out = (*func)(stream, data_in, size_in, data_out, size_out);
    avbinOpenMutex.unlock();
    return out;
}
AVbinStream* mod_avbin_open_stream(AVbinFile *file, int stream_index)
{
    avbinOpenMutex.lock();
	assert(ghinst != NULL);
	FARPROC rawfunc = GetProcAddress ( ghinst , "avbin_open_stream" );
	assert(rawfunc != (FARPROC)NULL);

	AVbinStream* (*func)(AVbinFile *, int)=0;
	func = (AVbinStream* (*)(AVbinFile *, int)) rawfunc;
    AVbinStream* out= (*func)(file, stream_index);
    avbinOpenMutex.unlock();
    return out;
}
void mod_avbin_close_stream(AVbinStream *stream)
{
    avbinOpenMutex.lock();
	assert(ghinst != NULL);
	FARPROC rawfunc = GetProcAddress ( ghinst , "avbin_close_stream" );
	assert(rawfunc != (FARPROC)NULL);

	void (*func)(AVbinStream *)=0;
	func = (void (*)(AVbinStream *)) rawfunc;
	(*func)(stream);	
    avbinOpenMutex.unlock();
}

void mod_avbin_close_file(AVbinFile *file)
{
    avbinOpenMutex.lock();
	assert(ghinst != NULL);
	FARPROC rawfunc = GetProcAddress ( ghinst , "avbin_close_file" );
	assert(rawfunc != (FARPROC)NULL);

	void (*func)(AVbinFile *)=0;
	func = (void (*)(AVbinFile *)) rawfunc;
	(*func)(file);		
    avbinOpenMutex.unlock();
}
#endif
