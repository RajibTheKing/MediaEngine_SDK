#pragma once

#include "noise_suppression.h"
#include "Tools.h"
class CNoise
{
	NsHandle* NS_instance;
	Tools m_Tools;
public:
	CNoise();
	~CNoise();
	int Denoise(short *sInBuf, int sBufferSize, short * sOutBuf);
};

