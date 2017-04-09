#include "WebRTCEchoCanceller.h"
#include "AudioCallSession.h"


WebRTCEchoCanceller::WebRTCEchoCanceller() : m_bAecmCreated(false), m_bAecmInited(false)
{
	int iAECERR = WebRtcAecm_Create(&AECM_instance);
	if (iAECERR)
	{
		ALOG("WebRtcAecm_Create failed id = " + m_Tools.IntegertoStringConvert(id));
	}
	else
	{
		ALOG("WebRtcAecm_Create successful id = " + m_Tools.IntegertoStringConvert(id));
		m_bAecmCreated = true;
	}

	iAECERR = WebRtcAecm_Init(AECM_instance, AUDIO_SAMPLE_RATE);
	if (iAECERR)
	{
		ALOG("WebRtcAecm_Init failed id = " + m_Tools.IntegertoStringConvert(id));
	}
	else
	{
		ALOG("WebRtcAecm_Init successful id = " + m_Tools.IntegertoStringConvert(id));
		m_bAecmInited = true;
	}

	AecmConfig aecConfig;
	aecConfig.cngMode = AecmFalse;
	aecConfig.echoMode = 2;

	if (WebRtcAecm_set_config(AECM_instance, aecConfig) == -1)
	{
		ALOG("WebRtcAecm_set_config unsuccessful id = " + m_Tools.IntegertoStringConvert(id));
	}
	else
	{
		ALOG("WebRtcAecm_set_config successful id = " + m_Tools.IntegertoStringConvert(id));
	}

	memset(m_sZeroBuf, 0, AECM_SAMPLES_IN_FRAME * sizeof(short));
	memset(m_sZeroBuf, 0, MAX_AUDIO_FRAME_SAMPLE_SIZE * sizeof(short));

	m_llLastFarendTime = 0;
	iCounter = 0;
	iCounter2 = 0;
}


WebRTCEchoCanceller::~WebRTCEchoCanceller()
{
	ALOG("WebRtcAec_destructor called");
	WebRtcAecm_Free(AECM_instance);
}


int WebRTCEchoCanceller::AddFarEndData(short *farEndData, int dataLen, bool isLiveStreamRunning)
{
	if (dataLen != CURRENT_AUDIO_FRAME_SAMPLE_SIZE(isLiveStreamRunning))
	{
		ALOG("aec farend Invalid size");
		return false;
	}


	for (int i = 0; i < dataLen; i += AECM_SAMPLES_IN_FRAME)
	{
		if (0 != WebRtcAecm_BufferFarend(AECM_instance, farEndData + i, AECM_SAMPLES_IN_FRAME))
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

	return true;
}


int WebRTCEchoCanceller::CancelEchoFromNearEndData(short *nearEndData, int dataLen, bool isLiveStreamRunning)
{
	if (dataLen != CURRENT_AUDIO_FRAME_SAMPLE_SIZE(isLiveStreamRunning))
	{
		ALOG("aec nearend Invalid size");
		return false;
	}

	iCounter++;

	for (int i = 0; i < CURRENT_AUDIO_FRAME_SAMPLE_SIZE(isLiveStreamRunning); i += AECM_SAMPLES_IN_FRAME)
	{
		bool bFailed = false, bZeroed = false;
		int delay = m_Tools.CurrentTimestamp() - m_llLastFarendTime/*10*/;
		if (delay < 0)
		{
			delay = 0;
		}

		if (0 != WebRtcAecm_Process(AECM_instance, nearEndData + i, NULL, nearEndData + i, AECM_SAMPLES_IN_FRAME, 50))
		{
			ALOG("WebRtcAec_Process failed bAecmCreated = " + m_Tools.IntegertoStringConvert((int)bAecmCreated) + " delay = " + m_Tools.IntegertoStringConvert((int)delay)
				+ " err = " + m_Tools.IntegertoStringConvert(WebRtcAecm_get_error_code(AECM_instance))
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
	}

	return true;
}
