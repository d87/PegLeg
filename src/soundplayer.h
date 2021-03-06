#pragma once

#include <dshow.h>

#pragma comment(lib, "Strmiids.lib")

class SoundPlayer {
	public:
		IGraphBuilder *pGraph = NULL;
		IMediaControl *pControl = NULL;
		IMediaEvent   *pEvent = NULL;
		IBasicAudio *pBasicAudio = NULL;
	public:
		SoundPlayer();
		~SoundPlayer();
		void Play(LPCWSTR filename, float volume);
};