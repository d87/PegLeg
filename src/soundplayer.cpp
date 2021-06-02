#include "soundplayer.h"
#include <algorithm>

SoundPlayer::SoundPlayer() {
	// Initialize the COM library.
	HRESULT hr;
	//HRESULT hr = CoInitialize(NULL);
	//if (FAILED(hr))
	//{
	//	printf("ERROR - Could not initialize COM library");
	//	return;
	//}

	// Create the filter graph manager and query for interfaces.
	hr = CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER,
		IID_IGraphBuilder, (void **)&pGraph);
	if (FAILED(hr))
	{
		printf("ERROR - Could not create the Filter Graph Manager.");
		return;
	}

	hr = pGraph->QueryInterface(IID_IMediaControl, (void **)&pControl);
	hr = pGraph->QueryInterface(IID_IMediaEvent, (void **)&pEvent);
	hr = pGraph->QueryInterface(IID_IBasicAudio, (void**)&pBasicAudio);
}

SoundPlayer::~SoundPlayer() {
	pControl->Release();
	pEvent->Release();
	pGraph->Release();
	pBasicAudio->Release();
	//CoUninitialize();
}

void SoundPlayer::Play(LPCWSTR filename, float volume) {
	long lVolume = std::clamp((int)(volume * 10000), 0, 10000) - 10000;
	
	HRESULT hr = pControl->Stop();

	// Build the graph.
	hr = pGraph->RenderFile((LPCWSTR)filename, NULL);
	if (SUCCEEDED(hr))
	{
		hr = pBasicAudio->put_Volume(lVolume);
		// Run the graph.
		hr = pControl->Run();

		//if (SUCCEEDED(hr))
		//{
			//  completion.
			//long evCode;
			//pEvent->WaitForCompletion(INFINITE, &evCode);

			// Note: Do not use INFINITE in a real application, because it
			// can block indefinitely.
		//}
	}
}