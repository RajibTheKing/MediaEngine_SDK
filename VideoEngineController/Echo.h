#pragma once

#include "echo_control_mobile.h"
#include "Tools.h"
#include "Size.h"
#include "SmartPointer.h"
#include "LockHandler.h"

#define AECM_SAMPLES_IN_FRAME 80

class CEcho
{
	void* AECM_instance;
	bool bAecmCreated;
	bool bAecmInited;
	Tools m_Tools;
	long long m_llLastFarendTime;
	int m_ID;
	int iCounter, iCounter2;
	int farending, processing;
	short m_sZeroBuf[AECM_SAMPLES_IN_FRAME];
	short m_sTempBuf[AUDIO_CLIENT_SAMPLES_IN_FRAME];
	
	//SmartPointer<CLockHandler> m_pEchoMutex;
	
public:
	CEcho(int id);
	~CEcho();
	int CancelEcho(short *sInBuf, int sBufferSize);
	int AddFarEnd(short *sInBuf, int sBufferSize);
};

