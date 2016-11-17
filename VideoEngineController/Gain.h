#pragma once

#define USE_WEBRTC_AGC
#ifndef USE_WEBRTC_AGC
#define USE_NAIVE_AGC
#endif
#ifdef USE_WEBRTC_AGC
#include "gain_control.h"
#include "signal_processing_library.h"
#endif
#include "Tools.h"
#define AUDIO_CLIENT_SAMPLE_SIZE 8000
class CGain
{
#ifdef USE_WEBRTC_AGC
	void* AGC_instance;
	short m_sTempBuf[AUDIO_CLIENT_SAMPLE_SIZE];
#endif
	Tools m_Tools;
	int m_iVolume;
public:
	CGain();
	~CGain();
	int AddGain(short *sInBuf, int sBufferSize);
	int AddFarEnd(short *sInBuf, int sBufferSize);
	int SetGain(int iGain);
};

