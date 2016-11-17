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

class CGain
{
#ifdef USE_WEBRTC_AGC
	void* AGC_instance;
	short *m_sTempBuf;
#endif
	Tools m_Tools;
	int m_iVolume;
	bool m_bGainEnabled;
public:
	CGain();
	~CGain();
	int AddGain(short *sInBuf, int sBufferSize);
	int AddFarEnd(short *sInBuf, int sBufferSize);
	int SetGain(int iGain);
};

