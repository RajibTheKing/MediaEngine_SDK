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
	aecConfig.cngMode = AecmFalse;
	aecConfig.echoMode = 4;
	if (WebRtcAecm_set_config(AECM_instance, aecConfig) == -1)
	{
		ALOG("WebRtcAecm_set_config unsuccessful");
	}
	else
	{
		ALOG("WebRtcAecm_set_config successful");
	}
	m_sZeroBuf = new short[100];
	memset(m_sZeroBuf, 0, 100 * sizeof(short));
	m_sTempBuf = new short[AUDIO_CLIENT_SAMPLE_SIZE];
	memset(m_sZeroBuf, 0, AUDIO_CLIENT_SAMPLE_SIZE * sizeof(short));
	m_llLastFarendTime = 0;
}


CEcho::~CEcho()
{
	delete[] m_sZeroBuf;
	delete[] m_sTempBuf;
	WebRtcAecm_Free(AECM_instance);
}


int CEcho::CancelEcho(short *sInBuf, int sBufferSize)
{
#if 1
	long long llNow = m_Tools.CurrentTimestamp();
#endif
	memcpy(m_sTempBuf, sInBuf, sBufferSize * sizeof(short));
	for (int i = 0; i < AUDIO_CLIENT_SAMPLE_SIZE; i += AECM_SAMPLE_SIZE)
	{
		bool bFailed = false, bZeroed = false;
		int delay = m_Tools.CurrentTimestamp() - m_llLastFarendTime;
		if (0 != WebRtcAecm_Process(AECM_instance, sInBuf + i, NULL, sInBuf + i, AECM_SAMPLE_SIZE, delay))
		{
			ALOG("WebRtcAec_Process failed bAecmCreated = " + m_Tools.IntegertoStringConvert((int)bAecmCreated) + " delay = " + m_Tools.IntegertoStringConvert((int)delay));
			bFailed = true;
		}
		else
		{
			ALOG("WebRtcAec_Process Delay = " + m_Tools.IntegertoStringConvert((int)delay));
		}
		if (memcmp(m_sZeroBuf, sInBuf + i, AECM_SAMPLE_SIZE * sizeof(short)) == 0)
		{
			ALOG("WebRtcAec_Process zeroed the buffer :-( :-(" + m_Tools.IntegertoStringConvert((int)this));
			bZeroed = true;
		}
		if (bFailed || bZeroed)
		{
			memcpy(sInBuf + i, m_sTempBuf + i, AECM_SAMPLE_SIZE * sizeof(short));
		}
	}
	return true;
#if 1
	if (memcmp(sInBuf, m_sTempBuf, sBufferSize * sizeof(short)) == 0)
	{
		ALOG("WebRtcAec_Process did nothing or failed or zeroed but took " + m_Tools.LongLongtoStringConvert(m_Tools.CurrentTimestamp() - llNow));
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
		else
		{
			m_llLastFarendTime = m_Tools.CurrentTimestamp();
			ALOG("WebRtcAec_BufferFarend successful");
		}
	}
	return true;
}