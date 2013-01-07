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

AVbinFile* mod_avbin_open_filename(const char *filename)
{
    avbinOpenMutex.lock();
    AVbinFile *fi = avbin_open_filename(filename);
    avbinOpenMutex.unlock();
    return fi;
}

AVbinResult mod_avbin_file_info(AVbinFile *file, AVbinFileInfo *info)
{
	return avbin_file_info(file, info);
}

AVbinResult mod_avbin_stream_info(AVbinFile *file, int stream_index, AVbinStreamInfo *info)
{
    return avbin_stream_info(file, stream_index, info);
}

AVbinResult mod_avbin_seek_file(AVbinFile *file, AVbinTimestamp timestamp)
{
	return avbin_seek_file(file, timestamp);
}

AVbinResult mod_avbin_read(AVbinFile *file, AVbinPacket *packet)
{
	return avbin_read(file, packet);
}

int mod_avbin_decode_video(AVbinStream *stream, uint8_t *data_in, size_t size_in, uint8_t *data_out)
{
	return avbin_decode_video(stream, data_in, size_in, data_out);
}

int mod_avbin_decode_audio(AVbinStream *stream, uint8_t *data_in, size_t size_in, uint8_t *data_out, int *size_out)
{
	return avbin_decode_audio(stream, data_in, size_in, data_out, size_out);
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
    avbinOpenMutex.unlock();
	return (*func)(filename);
}

AVbinResult mod_avbin_file_info(AVbinFile *file, AVbinFileInfo *info)
{
	assert(ghinst != NULL);
	FARPROC rawfunc = GetProcAddress ( ghinst , "avbin_file_info" );
	assert(rawfunc != (FARPROC)NULL);

	AVbinResult (*func)(AVbinFile *, AVbinFileInfo *)=0;
	func = (AVbinResult (*)(AVbinFile *, AVbinFileInfo *)) rawfunc;
	return (*func)(file, info);
}

AVbinResult mod_avbin_stream_info(AVbinFile *file, int stream_index, AVbinStreamInfo *info)
{
	assert(ghinst != NULL);
	FARPROC rawfunc = GetProcAddress ( ghinst , "avbin_stream_info" );
	assert(rawfunc != (FARPROC)NULL);

	AVbinResult (*func)(AVbinFile *, int, AVbinStreamInfo *)=0;
	func = (AVbinResult (*)(AVbinFile *, int, AVbinStreamInfo *)) rawfunc;
	return (*func)(file, stream_index, info);
}

AVbinResult mod_avbin_seek_file(AVbinFile *file, AVbinTimestamp timestamp)
{
	assert(ghinst != NULL);
	FARPROC rawfunc = GetProcAddress ( ghinst , "avbin_seek_file" );
	assert(rawfunc != (FARPROC)NULL);

	AVbinResult (*func)(AVbinFile *, AVbinTimestamp)=0;
	func = (AVbinResult (*)(AVbinFile *, AVbinTimestamp)) rawfunc;
	return (*func)(file, timestamp);
}

AVbinResult mod_avbin_read(AVbinFile *file, AVbinPacket *packet)
{
	assert(ghinst != NULL);
	FARPROC rawfunc = GetProcAddress ( ghinst , "avbin_read" );
	assert(rawfunc != (FARPROC)NULL);

	AVbinResult (*func)(AVbinFile *, AVbinPacket *)=0;
	func = (AVbinResult (*)(AVbinFile *, AVbinPacket *)) rawfunc;
	return (*func)(file, packet);
}

int mod_avbin_decode_video(AVbinStream *stream, uint8_t *data_in, size_t size_in, uint8_t *data_out)
{
	assert(ghinst != NULL);
	FARPROC rawfunc = GetProcAddress ( ghinst , "avbin_decode_video" );
	assert(rawfunc != (FARPROC)NULL);

	int (*func)(AVbinStream *, uint8_t *, size_t, uint8_t *)=0;
	func = (int (*)(AVbinStream *, uint8_t *, size_t, uint8_t *)) rawfunc;
	return (*func)(stream, data_in, size_in, data_out);
}

int mod_avbin_decode_audio(AVbinStream *stream, uint8_t *data_in, size_t size_in, uint8_t *data_out, int *size_out)
{
	assert(ghinst != NULL);
	FARPROC rawfunc = GetProcAddress ( ghinst , "avbin_decode_audio" );
	assert(rawfunc != (FARPROC)NULL);

	int (*func)(AVbinStream *, uint8_t *, size_t, uint8_t *, int *)=0;
	func = (int (*)(AVbinStream *, uint8_t *, size_t, uint8_t *, int *)) rawfunc;
	return (*func)(stream, data_in, size_in, data_out, size_out);
}
AVbinStream* mod_avbin_open_stream(AVbinFile *file, int stream_index)
{
    avbinOpenMutex.lock();
	assert(ghinst != NULL);
	FARPROC rawfunc = GetProcAddress ( ghinst , "avbin_open_stream" );
	assert(rawfunc != (FARPROC)NULL);

	AVbinStream* (*func)(AVbinFile *, int)=0;
	func = (AVbinStream* (*)(AVbinFile *, int)) rawfunc;
    avbinOpenMutex.unlock();
	return (*func)(file, stream_index);
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
