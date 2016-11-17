#pragma once

#include "echo_control_mobile.h"
#include "Tools.h"
class CEcho
{
	void* AECM_instance;
	bool bAecmCreated;
	bool bAecmInited;
	Tools m_Tools;
public:
	CEcho();
	~CEcho();
	int CancelEcho(short *sInBuf, int sBufferSize, short * sOutBuf);
	int AddFarEnd(short *sInBuf, int sBufferSize);
};

