#include "avbinbackend.h"
#include <iostream>
using namespace std;

extern "C"
{
#include <string.h>
#include <avbin.h>
}

int main(int argc, const char* argv[])
{
	class AvBinBackend av;
	//av.OpenFile("/media/data/main/media/music/Smashing Pumpkins/Smashing Pumpkins - Fan Tarantula Video (Full).webm");
	av.OpenFile("/home/tim/Desktop/eyevis-goldberg.avi");
	//av.OpenFile("/home/tim/Downloads/avbin-test-videos-h264/video-ntsc-sd-h264.mov");

	int i = 15;
	std::vector<std::tr1::shared_ptr<class DecodedFrame> > vid;
	av.GetFrameRange(i * 1000000, (i+1) * 1000000);
	cout << vid.size() << endl;


	av.CloseFile();
}

