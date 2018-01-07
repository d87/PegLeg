#include "soundplayer.h"

SoundPlayer::SoundPlayer() {
	// Initialize the COM library.
	HRESULT hr = CoInitialize(NULL);
	if (FAILED(hr))
	{
		printf("ERROR - Could not initialize COM library");
		return;
	}

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
}

void SoundPlayer::Play(LPCWSTR filename) {
	
	HRESULT hr = pControl->Stop();

	// Build the graph.
	hr = pGraph->RenderFile((LPCWSTR)filename, NULL);
	if (SUCCEEDED(hr))
	{
		// Run the graph.
		hr = pControl->Run();
		//if (SUCCEEDED(hr))
		//{
			// Wait for completion.
			//long evCode;
			//pEvent->WaitForCompletion(INFINITE, &evCode);

			// Note: Do not use INFINITE in a real application, because it
			// can block indefinitely.
		//}
	}
}

void SoundPlayer::Release() {
	pControl->Release();
	pEvent->Release();
	pGraph->Release();
	CoUninitialize();
}