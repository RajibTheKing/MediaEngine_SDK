#pragma once

#include "webrtc_vad.h"
#include "Tools.h"
class CVoice
{
	VadInst* VAD_instance;
	int nNextFrameMayHaveVoice;
	Tools m_Tools;
public:
	CVoice();
	~CVoice();
	bool HasVoice(short *sInBuf, int sBufferSize);
};

