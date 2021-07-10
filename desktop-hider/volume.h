#pragma once

#include <mmdeviceapi.h>
#include <endpointvolume.h>

#if !defined(DLLExport)
#define DLLExport __declspec(dllexport)
#endif

IAudioEndpointVolume* getEndpointVolume();
DLLExport float getSystemVolume();
DLLExport void setSystemVolume(double volume);