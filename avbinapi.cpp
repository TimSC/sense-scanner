#include "avbinapi.h"
#include <string.h>

#ifndef _MSC_VER

AVbinResult mod_avbin_init()
{
	return avbin_init();
}

AVbinFile* mod_avbin_open_filename(const char *filename)
{
	return avbin_open_filename(filename);
}

AVbinResult mod_avbin_file_info(AVbinFile *file, AVbinFileInfo *info)
{
	return avbin_file_info(file, info);
}

AVbinResult mod_avbin_stream_info(AVbinFile *file, int stream_index, AVbinStreamInfo *info)
{
	return avbin_stream_info(filem stream_index, info);
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
	return avbin_open_stream(file, stream_index);
}
void mod_avbin_close_stream(AVbinStream *stream)
{
	avbin_close_stream(stream);
}

void mod_avbin_close_file(AVbinFile *file)
{
	avbin_close_file(file);
}

#endif //_MSC_VER
//******************************************************

#ifdef _MSC_VER
AVbinResult mod_avbin_init()
{
	return AVBIN_RESULT_ERROR;
}

AVbinFile* mod_avbin_open_filename(const char *filename)
{
	return NULL;
}

AVbinResult mod_avbin_file_info(AVbinFile *file, AVbinFileInfo *info)
{
	return AVBIN_RESULT_ERROR;
}

AVbinResult mod_avbin_stream_info(AVbinFile *file, int stream_index, AVbinStreamInfo *info)
{
	return AVBIN_RESULT_ERROR;
}

AVbinResult mod_avbin_seek_file(AVbinFile *file, AVbinTimestamp timestamp)
{
	return AVBIN_RESULT_ERROR;
}

AVbinResult mod_avbin_read(AVbinFile *file, AVbinPacket *packet)
{
	return AVBIN_RESULT_ERROR;
}

int mod_avbin_decode_video(AVbinStream *stream, uint8_t *data_in, size_t size_in, uint8_t *data_out)
{
	return -1;
}

int mod_avbin_decode_audio(AVbinStream *stream, uint8_t *data_in, size_t size_in, uint8_t *data_out, int *size_out)
{
	return -1;
}
AVbinStream* mod_avbin_open_stream(AVbinFile *file, int stream_index)
{
	return NULL;
}
void mod_avbin_close_stream(AVbinStream *stream)
{
	
}

void mod_avbin_close_file(AVbinFile *file)
{
	
}
#endif