#include "Echo.h"
#include "AudioCallSession.h"

#define AECM_SAMPLE_SIZE 80

CEcho::CEcho()
{
	bAecmCreated = false;
	bAecmInited = false;
	
	int iAECERR = WebRtcAecm_Create(&AECM_instance);
	if (iAECERR)
	{
		ALOG("WebRtcAecm_Create failed");
	}
	else
	{
		ALOG("WebRtcAecm_Create successful");
		bAecmCreated = true;
	}

	iAECERR = WebRtcAecm_Init(AECM_instance, AUDIO_SAMPLE_RATE);
	if (iAECERR)
	{
		ALOG("WebRtcAecm_Init failed");
	}
	else
	{
		ALOG("WebRtcAecm_Init successful");
		bAecmInited = true;
	}

	AecmConfig aecConfig;
	aecConfig.cngMode = AecmTrue;
	aecConfig.echoMode = 4;
	if (WebRtcAecm_set_config(AECM_instance, aecConfig) == -1)
	{
		ALOG("WebRtcAecm_set_config unsuccessful");
	}
	else
	{
		ALOG("WebRtcAecm_set_config successful");
	}
}


CEcho::~CEcho()
{
	WebRtcAecm_Free(AECM_instance);
}


int CEcho::CancelEcho(short *sInBuf, int sBufferSize, short * sOutBuf)
{
#if 0
	long long llNow = m_Tools.CurrentTimestamp();
#endif
	for (int i = 0; i < AUDIO_CLIENT_SAMPLE_SIZE; i += AECM_SAMPLE_SIZE)
	{

		if (0 != WebRtcAecm_Process(AECM_instance, sInBuf + i, NULL, sOutBuf + i, AECM_SAMPLE_SIZE, 10))
		{
			ALOG("WebRtcAec_Process failed bAecmCreated = " + m_Tools.IntegertoStringConvert((int)bAecmCreated) + " bAecmInited = " + m_Tools.IntegertoStringConvert((int)bAecmInited));
		}
	}
	return true;
#if 0
	if (memcmp(sInBuf, sOutBuf, sBufferSize * sizeof(short)) == 0)
	{
		ALOG("WebRtcAec_Process did nothing but took " + m_Tools.LongLongtoStringConvert(m_Tools.CurrentTimestamp() - llNow));
		return false;
	}
	else
	{
		ALOG("WebRtcAec_Process tried to do something, believe me :-( . It took " + m_Tools.LongLongtoStringConvert(m_Tools.CurrentTimestamp() - llNow));
		return true;
	}
#endif
	
}

int CEcho::AddFarEnd(short *sBuffer, int sBufferSize)
{
	for (int i = 0; i < sBufferSize; i += AECM_SAMPLE_SIZE)
	{
		if (0 != WebRtcAecm_BufferFarend(AECM_instance, sBuffer + i, AECM_SAMPLE_SIZE))
		{
			ALOG("WebRtcAec_BufferFarend failed");
		}
	}
	return true;
}