#pragma once

#include "echo_control_mobile.h"
#include "Tools.h"
#include "Size.h"

#define AECM_SAMPLES_IN_FRAME 80

class CEcho
{
	void* AECM_instance;
	bool bAecmCreated;
	bool bAecmInited;
	Tools m_Tools;
	long long m_llLastFarendTime;
	short m_sZeroBuf[AECM_SAMPLES_IN_FRAME];
	short m_sTempBuf[AUDIO_CLIENT_SAMPLES_IN_FRAME];
public:
	CEcho();
	~CEcho();
	int CancelEcho(short *sInBuf, int sBufferSize);
	int AddFarEnd(short *sInBuf, int sBufferSize);
};

