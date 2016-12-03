#include "Echo.h"
#include "AudioCallSession.h"

CEcho::CEcho(int id)
{
	bAecmCreated = false;
	bAecmInited = false;
	//m_pEchoMutex.reset(new CLockHandler);
	
	int iAECERR = WebRtcAecm_Create(&AECM_instance);
	if (iAECERR)
	{
		ALOG("WebRtcAecm_Create failed id = " + m_Tools.IntegertoStringConvert(id));
	}
	else
	{
		ALOG("WebRtcAecm_Create successful id = " + m_Tools.IntegertoStringConvert(id));
		bAecmCreated = true;
	}

	iAECERR = WebRtcAecm_Init(AECM_instance, AUDIO_SAMPLE_RATE);
	if (iAECERR)
	{
		ALOG("WebRtcAecm_Init failed id = " + m_Tools.IntegertoStringConvert(id));
	}
	else
	{
		ALOG("WebRtcAecm_Init successful id = " + m_Tools.IntegertoStringConvert(id));
		bAecmInited = true;
	}

	AecmConfig aecConfig;
	aecConfig.cngMode = AecmFalse;
	aecConfig.echoMode = 4;
	if (WebRtcAecm_set_config(AECM_instance, aecConfig) == -1)
	{
		ALOG("WebRtcAecm_set_config unsuccessful id = " + m_Tools.IntegertoStringConvert(id));
	}
	else
	{
		ALOG("WebRtcAecm_set_config successful id = " + m_Tools.IntegertoStringConvert(id));
	}
	memset(m_sZeroBuf, 0, AECM_SAMPLES_IN_FRAME * sizeof(short));
	memset(m_sZeroBuf, 0, AUDIO_CLIENT_SAMPLES_IN_FRAME * sizeof(short));
	m_llLastFarendTime = 0;
	m_ID = id;
	//m_Tools.SOSleep(100);
	ALOG("WebRtcAecm_set_id successful id = " + m_Tools.IntegertoStringConvert(m_ID));
	iCounter = 0;
	iCounter2 = 0;
	farending = 0;
	processing = 0;
}


CEcho::~CEcho()
{
	ALOG("WebRtcAec_destructor called");
	WebRtcAecm_Free(AECM_instance);
}


int CEcho::CancelEcho(short *sInBuf, int sBufferSize)
{
	if (sBufferSize != AUDIO_CLIENT_SAMPLES_IN_FRAME)
	{
	
		ALOG("aec nearend Invalid size");
		return false;
	}
	iCounter ++ ;
	/*if (iCounter ++ > 5)
		return 0;*/
	/*while (farending)
	{
		m_Tools.SOSleep(5);
	}*/
	processing = 1;
#if 1
	long long llNow = m_Tools.CurrentTimestamp();
#endif
	//ALOG("aec sBufferSize = " + m_Tools.IntegertoStringConvert((int)sBufferSize));
	memcpy(m_sTempBuf, sInBuf, sBufferSize * sizeof(short));
	for (int i = 0; i < AUDIO_CLIENT_SAMPLES_IN_FRAME; i += AECM_SAMPLES_IN_FRAME)
	{
		bool bFailed = false, bZeroed = false;
		int delay = m_Tools.CurrentTimestamp() - m_llLastFarendTime/*10*/;
		if (delay < 0)
		{
			delay = 0;
		}
		if (0 != WebRtcAecm_Process(AECM_instance, sInBuf + i, NULL, sInBuf + i, AECM_SAMPLES_IN_FRAME, delay))
		{
			ALOG("WebRtcAec_Process failed bAecmCreated = " + m_Tools.IntegertoStringConvert((int)bAecmCreated) + " delay = " + m_Tools.IntegertoStringConvert((int)delay)
				+ " err = " + m_Tools.IntegertoStringConvert(WebRtcAecm_get_error_code(AECM_instance)) + " id = " + m_Tools.IntegertoStringConvert(m_ID)
				+ " iCounter = " + m_Tools.IntegertoStringConvert(iCounter)
				+ " iCounter2 = " + m_Tools.IntegertoStringConvert(iCounter2));
			bFailed = true;
		}
		else
		{
			/*ALOG("WebRtcAec_Process successful Delay = " + m_Tools.IntegertoStringConvert((int)delay) + " id = " + m_Tools.IntegertoStringConvert(m_ID)
				+ " iCounter = " + m_Tools.IntegertoStringConvert(iCounter)
				+ " iCounter2 = " + m_Tools.IntegertoStringConvert(iCounter2));*/
		}
		if (memcmp(m_sZeroBuf, sInBuf + i, AECM_SAMPLES_IN_FRAME * sizeof(short)) == 0)
		{
			//ALOG("WebRtcAec_Process zeroed the buffer :-( :-( id = " + m_Tools.IntegertoStringConvert(m_ID));
			if (memcmp(m_sZeroBuf, m_sTempBuf + i, AECM_SAMPLES_IN_FRAME * sizeof(short)))
			{
				//ALOG("WebRtcAec_Process didnt zero the buffer it was already 0");
			}
			else
			{
				ALOG("WebRtcAec_Process did zero the buffer it was not already 0");
				bZeroed = true;
			}			
		}
		if (bFailed || bZeroed)
		{
			memcpy(sInBuf + i, m_sTempBuf + i, AECM_SAMPLES_IN_FRAME * sizeof(short));
		}
	}
	processing = 0;
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
	if (sBufferSize != AUDIO_CLIENT_SAMPLES_IN_FRAME)
	{
		ALOG("aec farend Invalid size");
		return false;
	}
	iCounter2++;
	/*if (iCounter2 ++ > 5)
		return 0;*/
	//Locker lock(*m_pEchoMutex);
	/*while (processing)
	{
		m_Tools.SOSleep(5);
	}*/
	farending = 1;
	for (int i = 0; i < sBufferSize; i += AECM_SAMPLES_IN_FRAME)
	{
		if (0 != WebRtcAecm_BufferFarend(AECM_instance, sBuffer + i, AECM_SAMPLES_IN_FRAME))
		{			
			ALOG("WebRtcAec_BufferFarend failed id = " + m_Tools.IntegertoStringConvert(m_ID) + " err = " + m_Tools.IntegertoStringConvert(WebRtcAecm_get_error_code(AECM_instance))
				+ " iCounter = " + m_Tools.IntegertoStringConvert(iCounter)
				+ " iCounter2 = " + m_Tools.IntegertoStringConvert(iCounter2));
		}
		else
		{
			m_llLastFarendTime = m_Tools.CurrentTimestamp();
			/*ALOG("WebRtcAec_BufferFarend successful id = " + m_Tools.IntegertoStringConvert(m_ID)
				+ " iCounter = " + m_Tools.IntegertoStringConvert(iCounter)
				+ " iCounter2 = " + m_Tools.IntegertoStringConvert(iCounter2));*/
		}
	}
	farending = 0;
	return true;
}