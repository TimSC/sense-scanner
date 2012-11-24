#ifndef VLCBACKEND_H
#define VLCBACKEND_H

#include <fstream>
#include <inttypes.h>

class VlcBackend
{
public:
    VlcBackend();
    ~VlcBackend();

	int OpenFile(const char *filename);
	void Test();
	void Test2(unsigned long long ti);

    //libvlc smem callback functions
    void AudioPrerender (uint8_t** pp_pcm_buffer , unsigned int size);
    void AudioPostrender(uint8_t* p_pcm_buffer, unsigned int channels,
        unsigned int rate, unsigned int nb_samples, unsigned int bits_per_sample, unsigned int size, int64_t pts );
    void VideoPrerender(uint8_t **pp_pixel_buffer, int size);
    void VideoPostrender(uint8_t *p_pixel_buffer
          , int width, int height, int pixel_pitch, int size, int64_t pts);

    void TimeChanged();
    void EndReached();

protected:
    class libvlc_instance_t *vlcInstance;
    class libvlc_media_player_t *media_player;
    class libvlc_event_manager_t *eventManager;
    class libvlc_media_t *media;
    std::ofstream vlcDebugLog;
	int64_t startVlcClock;
};

#endif // VLCBACKEND_H
