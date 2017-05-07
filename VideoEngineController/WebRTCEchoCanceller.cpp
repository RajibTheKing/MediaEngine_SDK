#include "WebRTCEchoCanceller.h"

#include "Tools.h"
#include "LogPrinter.h"

#ifndef ALOG
#define ALOG(a) CLogPrinter_WriteSpecific6(CLogPrinter::INFO, "ALOG:" + a);
#endif


WebRTCEchoCanceller::WebRTCEchoCanceller() : m_bAecmCreated(false), m_bAecmInited(false)
{

#ifdef USE_AECM
#ifdef ECHO_ANALYSIS
	m_bWritingDump = false;
	EchoFile = fopen("/sdcard/endSignal.pcma", "wb");
#endif

	int iAECERR = WebRtcAecm_Create(&AECM_instance);
	if (iAECERR)
	{
		ALOG("WebRtcAecm_Create failed");
	}
	else
	{
		ALOG("WebRtcAecm_Create successful");
		m_bAecmCreated = true;
	}

	iAECERR = WebRtcAecm_Init(AECM_instance, AUDIO_SAMPLE_RATE);
	if (iAECERR)
	{
		ALOG("WebRtcAecm_Init failed");
	}
	else
	{
		ALOG("WebRtcAecm_Init successful");
		m_bAecmInited = true;
	}

	AecmConfig aecConfig;
	aecConfig.cngMode = AecmFalse;
	aecConfig.echoMode = 2;

	if (WebRtcAecm_set_config(AECM_instance, aecConfig) == -1)
	{
		ALOG("WebRtcAecm_set_config unsuccessful");
	}
	else
	{
		ALOG("WebRtcAecm_set_config successful");
	}

	memset(m_sZeroBuf, 0, AECM_SAMPLES_IN_FRAME * sizeof(short));

	m_llLastFarendTime = 0;
	iCounter = 0;
	iCounter2 = 0;
#endif
}


WebRTCEchoCanceller::~WebRTCEchoCanceller()
{
#ifdef USE_AECM
	ALOG("WebRtcAec_destructor called");
	WebRtcAecm_Free(AECM_instance);

#ifdef ECHO_ANALYSIS
	fclose(EchoFile);
#endif
#endif
}


int WebRTCEchoCanceller::AddFarEndData(short *farEndData, int dataLen, bool isLiveStreamRunning)
{
#ifdef USE_AECM
	if (dataLen != CURRENT_AUDIO_FRAME_SAMPLE_SIZE(isLiveStreamRunning))
	{
		ALOG("aec farend Invalid size");
		return false;
	}

#ifdef ECHO_ANALYSIS
	while (m_bWritingDump)
	{
		Tools::SOSleep(1);
	}
	m_bWritingDump = true;
	short temp = WEBRTC_FAREND;
	fwrite(&temp, sizeof(short), HEADER_SIZE, EchoFile);
	fwrite(farEndData, sizeof(short), dataLen, EchoFile);
	m_bWritingDump = false;
#endif

	for (int i = 0; i < dataLen; i += AECM_SAMPLES_IN_FRAME)
	{
		if (0 != WebRtcAecm_BufferFarend(AECM_instance, farEndData + i, AECM_SAMPLES_IN_FRAME))
		{
			ALOG("WebRtcAec_BufferFarend failed, " + " err = " + Tools::IntegertoStringConvert(WebRtcAecm_get_error_code(AECM_instance))
				+ " iCounter = " + Tools::IntegertoStringConvert(iCounter)
				+ " iCounter2 = " + Tools::IntegertoStringConvert(iCounter2));
		}
		else
		{
			m_llLastFarendTime = Tools::CurrentTimestamp();
			/*ALOG("WebRtcAec_BufferFarend successful id = " + Tools::IntegertoStringConvert(m_ID)
			+ " iCounter = " + Tools::IntegertoStringConvert(iCounter)
			+ " iCounter2 = " + Tools::IntegertoStringConvert(iCounter2));*/
		}
	}
#endif

	return true;
}


int WebRTCEchoCanceller::CancelEcho(short *nearEndData, int dataLen, bool isLiveStreamRunning)
{
#ifdef USE_AECM

	if (dataLen != CURRENT_AUDIO_FRAME_SAMPLE_SIZE(isLiveStreamRunning))
	{
		ALOG("aec nearend Invalid size");
		return false;
	}

	iCounter++;

#ifdef ECHO_ANALYSIS
	while (m_bWritingDump)
	{
		Tools::SOSleep(1);
	}
	m_bWritingDump = true;
	short temp = NEAREND;
	fwrite(&temp, sizeof(short), HEADER_SIZE, EchoFile);
	fwrite(nearEndData, sizeof(short), dataLen, EchoFile);
	m_bWritingDump = false;
#endif

	for (int i = 0; i < CURRENT_AUDIO_FRAME_SAMPLE_SIZE(isLiveStreamRunning); i += AECM_SAMPLES_IN_FRAME)
	{
		bool bFailed = false, bZeroed = false;
		int delay = Tools::CurrentTimestamp() - m_llLastFarendTime/*10*/;
		if (delay < 0)
		{
			delay = 0;
		}

		if (0 != WebRtcAecm_Process(AECM_instance, nearEndData + i, NULL, nearEndData + i, AECM_SAMPLES_IN_FRAME, 50))
		{
			ALOG("WebRtcAec_Process failed bAecmCreated = " + Tools::IntegertoStringConvert((int)bAecmCreated) + " delay = " + Tools::IntegertoStringConvert((int)delay)
				+ " err = " + Tools::IntegertoStringConvert(WebRtcAecm_get_error_code(AECM_instance))
				+ " iCounter = " + Tools::IntegertoStringConvert(iCounter)
				+ " iCounter2 = " + Tools::IntegertoStringConvert(iCounter2));
			bFailed = true;
		}
		else
		{
			/*ALOG("WebRtcAec_Process successful Delay = " + Tools::IntegertoStringConvert((int)delay) + " id = " + Tools::IntegertoStringConvert(m_ID)
			+ " iCounter = " + Tools::IntegertoStringConvert(iCounter)
			+ " iCounter2 = " + Tools::IntegertoStringConvert(iCounter2));*/
		}
	}
#endif

	return true;
}
