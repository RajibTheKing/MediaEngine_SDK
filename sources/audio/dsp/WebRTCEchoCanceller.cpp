#include "WebRTCEchoCanceller.h"

#include "Tools.h"
#include "LogPrinter.h"

#ifdef USE_AECM
extern int gEchoType;
#else
int gEchoType = -1;
#endif

//#define ECHO_ANALYSIS

#ifdef ECHO_ANALYSIS
FILE *EchoFile = nullptr;
#define HEADER_SIZE 1
#define WEBRTC_FAREND 1
#define SPEEX_FAREND 2
#define NEAREND 3
#endif

#ifndef ALOG
#define ALOG(a) CLogPrinter_WriteSpecific6(CLogPrinter::INFO, "ALOG:" + a);
#endif

namespace MediaSDK
{

#define ECHO_TYPE_NO_AEC -1
#define ECHO_TYPE_NO_ECHO 0
#define ECHO_TYPE_JUST_ECHO 1
#define ECHO_TYPE_DOUBLE_TALK 2


	WebRTCEchoCanceller::WebRTCEchoCanceller(bool isLiveRunning) : m_bAecmCreated(false), m_bAecmInited(false)
	{
		LOGFARQUAD("WebRTCEchoCanceller constructor");

#ifdef USE_AECM
#ifdef ECHO_ANALYSIS
		EchoFile = fopen("/sdcard/endSignal.pcma3", "wb");
#endif
		m_bNearEndingOrFarEnding = false;

		int iAECERR = -1;

		AECM_instance = WebRtcAecm_Create();
		m_bAecmCreated = true;

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
		aecConfig.echoMode = 4;
		LOG18("##TT echo level %d", (int)aecConfig.echoMode);
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
#endif

#ifdef ECHO_ANALYSIS
		if (EchoFile)
		{
			fclose(EchoFile);
			EchoFile = nullptr;
		}
#endif

	}


	int WebRTCEchoCanceller::CancelEcho(short *sInBuf, int nBufferSize, long long llDelay, short *NearEndNoisyData)
	{
		int nEchoStateFlags = ECHO_TYPE_NO_ECHO; //int containing 10 flags telling whether the 10 80 sized flags contain echo
#ifdef USE_AECM
		iCounter++;

		for (int i = 0; i < nBufferSize; i += AECM_SAMPLES_IN_FRAME)
		{

			while (m_bNearEndingOrFarEnding)
			{
				Tools::SOSleep(1);
			}
			m_bNearEndingOrFarEnding = true;
#ifdef ECHO_ANALYSIS

			short temp = NEAREND;
			fwrite(&temp, sizeof(short), 1, EchoFile);
			fwrite(&llDelay, sizeof(short), 1, EchoFile);
			fwrite(sInBuf + i, sizeof(short), AECM_SAMPLES_IN_FRAME, EchoFile);
#endif
			bool bFailed = false, bZeroed = false;
			int iAecmResult = 0;
			if (NearEndNoisyData == nullptr)
			{
				iAecmResult = WebRtcAecm_Process(AECM_instance, sInBuf + i, NULL, sInBuf + i, AECM_SAMPLES_IN_FRAME, llDelay);
			}
			else
			{
				iAecmResult = WebRtcAecm_Process(AECM_instance, NearEndNoisyData + i, sInBuf + i, sInBuf + i, AECM_SAMPLES_IN_FRAME, llDelay);
			}
			if (0 != iAecmResult)
			{
				ALOG("WebRtcAec_Process failed bAecmCreated = " + m_Tools.IntegertoStringConvert((int)bAecmCreated) + " delay = " + m_Tools.IntegertoStringConvert((int)llDelay)
					+ " err = " + m_Tools.IntegertoStringConvert(WebRtcAecm_get_error_code(AECM_instance)) + " id = " + m_Tools.IntegertoStringConvert(m_ID)
					+ " iCounter = " + m_Tools.IntegertoStringConvert(iCounter)
					+ " iCounter2 = " + m_Tools.IntegertoStringConvert(iCounter2));
				bFailed = true;
			}
	
			m_bNearEndingOrFarEnding = false;
			if (gEchoType == ECHO_TYPE_JUST_ECHO)
			{
				memset(sInBuf + i, 0, AECM_SAMPLES_IN_FRAME * sizeof(short));
			}
			if (gEchoType == ECHO_TYPE_JUST_ECHO || gEchoType == ECHO_TYPE_DOUBLE_TALK)
			{
				nEchoStateFlags |= 1;
			}

			nEchoStateFlags <<= 1;
		}

		nEchoStateFlags >>= 1;

#endif
		return nEchoStateFlags;
	}

	int WebRTCEchoCanceller::AddFarEndData(short *sBuffer, int sBufferSize)
	{
#ifdef USE_AECM

		LOG18("Farending2");

		for (int i = 0; i < sBufferSize; i += AECM_SAMPLES_IN_FRAME)
		{
			while (m_bNearEndingOrFarEnding)
			{
				Tools::SOSleep(1);
			}
			m_bNearEndingOrFarEnding = true;

#ifdef ECHO_ANALYSIS

			short temp = WEBRTC_FAREND;
			short iDelay = 0;
			fwrite(&temp, sizeof(short), 1, EchoFile);
			fwrite(&iDelay, sizeof(short), 1, EchoFile);
			fwrite(sBuffer + i, sizeof(short), AECM_SAMPLES_IN_FRAME, EchoFile);

#endif
			if (0 != WebRtcAecm_BufferFarend(AECM_instance, sBuffer + i, AECM_SAMPLES_IN_FRAME))
			{
				ALOG("WebRtcAec_BufferFarend failed id = " + m_Tools.IntegertoStringConvert(m_ID) + " err = " + m_Tools.IntegertoStringConvert(WebRtcAecm_get_error_code(AECM_instance))
					+ " iCounter = " + m_Tools.IntegertoStringConvert(iCounter)
					+ " iCounter2 = " + m_Tools.IntegertoStringConvert(iCounter2));
			}
			else
			{
				m_llLastFarendTime = Tools::CurrentTimestamp();
				/*ALOG("WebRtcAec_BufferFarend successful id = " + m_Tools.IntegertoStringConvert(m_ID)
				+ " iCounter = " + m_Tools.IntegertoStringConvert(iCounter)
				+ " iCounter2 = " + m_Tools.IntegertoStringConvert(iCounter2));*/
			}
			m_bNearEndingOrFarEnding = false;
		}
#endif

		return true;
	}
} //namespace MediaSDK

