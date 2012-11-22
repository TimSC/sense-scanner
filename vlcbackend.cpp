#include "vlcbackend.h"
#include <vlc/vlc.h>
#include <stdio.h>
#include <assert.h>
#include <iostream>
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
	this->media = NULL;

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
    if(this->media) libvlc_media_release(this->media);
    this->media = NULL;
}

int VlcBackend::OpenFile(const char *filename)
{
    if(this->media) libvlc_media_release(this->media);
    this->media = libvlc_media_new_path(this->vlcInstance, filename);
    libvlc_media_player_set_media(this->media_player, this->media);
	return 1;
}

void VlcBackend::Test()
{
	libvlc_media_player_play(this->media_player);
}

//*************************************************

void VlcBackend::AudioPrerender (uint8_t** pp_pcm_buffer , unsigned int size)
{
    *pp_pcm_buffer = new uint8_t[size];
}

void VlcBackend::AudioPostrender(uint8_t* p_pcm_buffer, unsigned int channels, unsigned int rate, unsigned int nb_samples, unsigned int bits_per_sample, unsigned int size, int64_t pts )
{
    delete p_pcm_buffer;
}

void VlcBackend::VideoPrerender(uint8_t **pp_pixel_buffer, int size)
{
    *pp_pixel_buffer = new uint8_t[size];
}

void VlcBackend::VideoPostrender(uint8_t *p_pixel_buffer,
    int width, int height, int pixel_pitch, int size, int64_t pts)
{
    delete p_pixel_buffer;
}

void VlcBackend::TimeChanged()
{
	libvlc_time_t time = libvlc_media_player_get_time(this->media_player);
    cout << "MediaPlayerTimeChanged "<<(long long)time << endl;
}

void VlcBackend::EndReached()
{

}
