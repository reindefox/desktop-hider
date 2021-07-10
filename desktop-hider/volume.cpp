#include "volume.h"

IAudioEndpointVolume* getEndpointVolume() {
	(void)CoInitialize(NULL);

	IMMDeviceEnumerator* iMMDeviceEnumerator = nullptr;
	IMMDevice* iMMDevice = nullptr;
	IAudioEndpointVolume* endpointVolume = nullptr;

	(void)CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_INPROC_SERVER, __uuidof(IMMDeviceEnumerator), (LPVOID*)&iMMDeviceEnumerator);
	iMMDeviceEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &iMMDevice);
	iMMDeviceEnumerator->Release();
	iMMDeviceEnumerator = nullptr;

	(void)iMMDevice->Activate(__uuidof(IAudioEndpointVolume), CLSCTX_INPROC_SERVER, nullptr, (LPVOID*)&endpointVolume);
	iMMDevice->Release();
	iMMDevice = nullptr;

	return endpointVolume;
}

DLLExport float getSystemVolume() {
	(void)CoInitialize(NULL);

	IAudioEndpointVolume* endpointVolume = getEndpointVolume();

	float volume = 0;

	endpointVolume->GetMasterVolumeLevelScalar(&volume);

	endpointVolume->Release();

	(void)CoUninitialize();

	return volume;
}

DLLExport void setSystemVolume(double volume) {
	(void)CoInitialize(NULL);

	IAudioEndpointVolume* endpointVolume = getEndpointVolume();

	endpointVolume->SetMasterVolumeLevelScalar((float) volume, nullptr);

	endpointVolume->Release();

	(void)CoUninitialize();
}