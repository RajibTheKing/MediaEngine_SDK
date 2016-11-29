#pragma once

#include "echo_control_mobile.h"
#include "Tools.h"
class CEcho
{
	void* AECM_instance;
	bool bAecmCreated;
	bool bAecmInited;
	Tools m_Tools;
	long long m_llLastFarendTime;
	short *m_sZeroBuf;
	short *m_sTempBuf;
public:
	CEcho();
	~CEcho();
	int CancelEcho(short *sInBuf, int sBufferSize);
	int AddFarEnd(short *sInBuf, int sBufferSize);
};

