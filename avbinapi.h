#ifndef _AVBIN_API_H_
#define _AVBIN_API_H_

#include <string.h>
extern "C"
{
#include <avbin.h>
}

/*!
* These functions wrap the underlying avbin interface but add
* resource locking and windows specific fuctionality to load DLLs.
*/

AVbinResult mod_avbin_init();
AVbinFile* mod_avbin_open_filename(const char *filename);
AVbinResult mod_avbin_file_info(AVbinFile *file, AVbinFileInfo *info);
AVbinResult mod_avbin_stream_info(AVbinFile *file, int stream_index, AVbinStreamInfo *info);
AVbinResult mod_avbin_seek_file(AVbinFile *file, AVbinTimestamp timestamp);
AVbinResult mod_avbin_read(AVbinFile *file, AVbinPacket *packet);
int mod_avbin_decode_video(AVbinStream *stream, uint8_t *data_in, size_t size_in, uint8_t *data_out);
int mod_avbin_decode_audio(AVbinStream *stream, uint8_t *data_in, size_t size_in, uint8_t *data_out, int *size_out);
AVbinStream* mod_avbin_open_stream(AVbinFile *file, int stream_index);
void mod_avbin_close_stream(AVbinStream *stream);
void mod_avbin_close_file(AVbinFile *file);

#endif //_AVBIN_API_H_
