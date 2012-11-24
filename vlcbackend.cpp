#include "vlcbackend.h"
#include <vlc/vlc.h>
#include <stdio.h>
#include <assert.h>
#include <iostream>
#include <tr1/memory>
using namespace std;

//******** libvlc Callbacks ************************

static void cbAudioPrerender (void* p_audio_data, uint8_t** pp_pcm_buffer , unsigned int size)
{
    VlcBackend *obj = (VlcBackend *)p_audio_data;
    assert(obj != NULL);
    obj->AudioPrerender(pp_pcm_buffer, size);
}

static void cbAudioPostrender(void* p_audio_data, uint8_t* p_pcm_buffer, unsigned int channels, unsigned int rate,
                       unsigned int nb_samples, unsigned int bits_per_sample, unsigned int size, int64_t pts )
{
    VlcBackend *obj = (VlcBackend *)p_audio_data;
    assert(obj != NULL);
    obj->AudioPostrender(p_pcm_buffer, channels, rate, nb_samples, bits_per_sample, size, pts);
}

static void cbVideoPrerender(void *p_video_data, uint8_t **pp_pixel_buffer, int size)
{
    VlcBackend *obj = (VlcBackend *)p_video_data;
    assert(obj != NULL);
    obj->VideoPrerender(pp_pixel_buffer, size);
}

static void cbVideoPostrender(void *p_video_data, uint8_t *p_pixel_buffer
      , int width, int height, int pixel_pitch, int size, int64_t pts)
{
    VlcBackend *obj = (VlcBackend *)p_video_data;
    assert(obj != NULL);
    obj->VideoPostrender(p_pixel_buffer, width, height, pixel_pitch, size, pts);
}

static void handleEvent(const libvlc_event_t* pEvt, void* pUserData)
{
    VlcBackend *obj = (VlcBackend *)pUserData;
    assert(obj != NULL);
    //printf("%s\n", libvlc_event_type_name(pEvt->type));

    switch(pEvt->type)
    {
    case libvlc_MediaPlayerTimeChanged:
        obj->TimeChanged();
        break;
    case libvlc_MediaPlayerEndReached:
        obj->EndReached();
        break;
    }
}

//***********************************************************

VlcBackend::VlcBackend()
{
    vlcDebugLog.open("vlcdebuglog.txt");
	this->media = NULL;
	this->startVlcClock = 0;

    // VLC options
    char smem_options[1000];
    sprintf(smem_options
      , "#transcode{vcodec=I444,acodec=s16l}:smem{"
         "video-prerender-callback=%lld,"
         "video-postrender-callback=%lld,"
         "audio-prerender-callback=%lld,"
         "audio-postrender-callback=%lld,"
         "audio-data=%lld,"
         "video-data=%lld},"
      , (long long int)(intptr_t)(void*)&cbVideoPrerender
      , (long long int)(intptr_t)(void*)&cbVideoPostrender
      , (long long int)(intptr_t)(void*)&cbAudioPrerender
      , (long long int)(intptr_t)(void*)&cbAudioPostrender
      , (long long int)(intptr_t)(void*)this
      , (long long int)(intptr_t)(void*)this);
    //TODO This string buffer is rather unsafe

    const char * const vlc_args[] = {
         "-I", "dummy", // Don't use any interface
         "--ignore-config", // Don't use VLC's config
         "--extraintf=logger", // Log anything
         "--verbose=1", // Be verbose
         "--sout", smem_options // Stream to memory
          };

    this->vlcInstance = libvlc_new(sizeof(vlc_args) / sizeof(vlc_args[0]), vlc_args);
    this->media_player = libvlc_media_player_new(vlcInstance);
    this->eventManager = libvlc_media_player_event_manager(media_player);
    libvlc_event_attach(eventManager, libvlc_MediaPlayerTimeChanged, handleEvent, this);
    libvlc_event_attach(eventManager, libvlc_MediaPlayerEndReached, handleEvent, this);
    libvlc_event_attach(eventManager, libvlc_MediaPlayerPositionChanged, handleEvent, this);
    
}

VlcBackend::~VlcBackend()
{
    libvlc_media_player_stop(this->media_player);
    if(this->media) libvlc_media_release(this->media);
    this->media = NULL;
}

int VlcBackend::OpenFile(const char *filename)
{
    if(this->media) libvlc_media_release(this->media);
    this->media = libvlc_media_new_path(this->vlcInstance, filename);
    libvlc_media_player_set_media(this->media_player, this->media);
    this->vlcDebugLog << "Opened " << filename << endl;
	return 1;
}

static void Test3 (void *opaque, void **planes)
{
	cout << "Test3" << endl;
}


void VlcBackend::Test()
{
	libvlc_media_player_play(this->media_player);
	cout << libvlc_get_play_start_time(this->media_player, this->media, this->vlcInstance, (libvlc_video_lock_cb)Test3) << endl;
	this->startVlcClock = libvlc_clock();
}

void VlcBackend::Test2(unsigned long long ti)
{
	this->vlcDebugLog << "Seek to " << ti << " from " << libvlc_media_player_get_time(this->media_player)<<endl;
	libvlc_media_player_stop(this->media_player);
	libvlc_media_player_set_time(this->media_player, ti);
	libvlc_media_player_play(this->media_player);
	this->startVlcClock = libvlc_clock();
}

//*************************************************

void VlcBackend::AudioPrerender (uint8_t** pp_pcm_buffer , unsigned int size)
{
    *pp_pcm_buffer = new uint8_t[size];
}

void VlcBackend::AudioPostrender(uint8_t* p_pcm_buffer, unsigned int channels, unsigned int rate, unsigned int nb_samples, unsigned int bits_per_sample, unsigned int size, int64_t pts )
{
    std::tr1::shared_ptr<uint8_t> sndBuffer(p_pcm_buffer);
    //this->vlcDebugLog << "AudioPostrender " << size << endl;
}

void VlcBackend::VideoPrerender(uint8_t **pp_pixel_buffer, int size)
{
    *pp_pixel_buffer = new uint8_t[size];
}

void VlcBackend::VideoPostrender(uint8_t *p_pixel_buffer,
    int width, int height, int pixel_pitch, int size, int64_t pts)
{
	//input_clock_ConvertTS is important?
	//struct block_t in vlc_block.h
	//cl->ref.i_stream
	//cl->ref.i_system
	//decoder_GetDisplayDate
	//struct image_handler_t

	//libvlc_media_player_get_fps
	
    std::tr1::shared_ptr<uint8_t> pixBuffer(p_pixel_buffer);
    this->vlcDebugLog << "VideoPostrender " << size << " pts="<<(pts - this->startVlcClock)<< " clock=" << libvlc_clock()<<endl;
}

void VlcBackend::TimeChanged()
{
	libvlc_time_t time = libvlc_media_player_get_time(this->media_player);
    //cout << "MediaPlayerTimeChanged "<<(long long)time << endl;
    this->vlcDebugLog << "TimeChanged " << (long long)time << endl;
}

void VlcBackend::EndReached()
{
    this->vlcDebugLog << "EndReached " << (long long)time << endl;
}
