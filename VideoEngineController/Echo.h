#pragma once

#define USE_WEBRTC_AECM
//#define USE_SPEEX_AECM


#ifdef USE_WEBRTC_AECM
#include "echo_control_mobile.h"
#endif
#if defined(USE_SPEEX_AECM)
#include "speex/speex_echo.h"
#include "speex/speex_preprocess.h"
#endif
#include "Tools.h"
#include "AudioMacros.h"


#define AECM_SAMPLES_IN_FRAME 80

class CEcho
{
#ifdef USE_WEBRTC_AECM
	void* AECM_instance;
#endif
#if defined(USE_SPEEX_AECM)
	SpeexEchoState *st;
	SpeexPreprocessState *den;
#endif
	bool bAecmCreated;
	bool bAecmInited;
	Tools m_Tools;
	long long m_llLastFarendTime;
	int m_ID;
	int iCounter, iCounter2;
	int farending, processing;
	short m_sZeroBuf[AECM_SAMPLES_IN_FRAME];
	short m_sTempBuf[MAX_AUDIO_FRAME_SAMPLE_SIZE];
	short m_sSpeexFarendBuf[MAX_AUDIO_FRAME_SAMPLE_SIZE];
	bool m_bFarendArrived;
	bool m_bReadingFarend, m_bWritingFarend, m_bWritingDump;
	
	//SmartPointer<CLockHandler> m_pEchoMutex;
	
public:
	CEcho(int id);
	~CEcho();
	int CancelEcho(short *sInBuf, int sBufferSize, bool isLoudspeaker, bool isLiveStreamRunning = false);
	int DeAmplitude(short *sInBuf, int sBufferSize);
	int LowPass(short *sInBuf, int sBufferSize);
	int AddFarEnd(short *sInBuf, int sBufferSize, bool isLiveStreamRunning, bool bLoudSpeakerEnabled = false);
};

