#include "WebRTCEchoCanceller.h"

#include "Tools.h"
#include "MediaLogger.h"

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


namespace MediaSDK
{

#define ECHO_TYPE_NO_AEC -1
#define ECHO_TYPE_NO_ECHO 0
#define ECHO_TYPE_JUST_ECHO 1
#define ECHO_TYPE_DOUBLE_TALK 2


	WebRTCEchoCanceller::WebRTCEchoCanceller(bool isLiveRunning) : m_bAecmCreated(false), m_bAecmInited(false)
	{
		//MediaLog(LOG_DEBUG, "WebRTCEchoCanceller constructor");

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
			//MediaLog(LOG_DEBUG, "WebRtcAecm_Init failed");
		}
		else
		{
			//MediaLog(LOG_DEBUG, "WebRtcAecm_Init successful");
			m_bAecmInited = true;
		}

		AecmConfig aecConfig;
		aecConfig.cngMode = AecmFalse;
		aecConfig.echoMode = 4;
		//MediaLog(CODE_TRACE, "##TT echo level %d", (int)aecConfig.echoMode);
		if (WebRtcAecm_set_config(AECM_instance, aecConfig) == -1)
		{
			//MediaLog(CODE_TRACE, "WebRtcAecm_set_config unsuccessful");
		}
		else
		{
			//MediaLog(CODE_TRACE, "WebRtcAecm_set_config successful");
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
		//MediaLog(CODE_TRACE, "WebRtcAec_destructor called");
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
		MediaLog(LOG_DEBUG, "[WebRTCE] Cancelling Echo buffer size: %d, Delay: %lld", nBufferSize, llDelay);
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
				/*MediaLog(CODE_TRACE, "WebRtcAec_Process failed bAecmCreated = %d delay = %d err = %d id = %d iCounter = %d iCounter2 = %d",
					(int)bAecmCreated, (int)llDelay, (int)WebRtcAecm_get_error_code(AECM_instance), (int)m_ID, iCounter, iCounter2);*/
					
				bFailed = true;
			}
	
			m_bNearEndingOrFarEnding = false;
			/*if (gEchoType == ECHO_TYPE_JUST_ECHO)
			{
				memset(sInBuf + i, 0, AECM_SAMPLES_IN_FRAME * sizeof(short));
			}*/
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
		MediaLog(LOG_DEBUG, "[WebRTCE] Add Far end Data: %d", sBufferSize);
#ifdef USE_AECM

		//MediaLog(CODE_TRACE, "Farending2");

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
				/*MediaLog(CODE_TRACE, "WebRtcAec_BufferFarend failed id = %lld err =  iCounter = %d iCounter2 = %d"
				, m_ID, (int)WebRtcAecm_get_error_code(AECM_instance), iCounter, iCounter2);
				*/
					
			}
			else
			{
				m_llLastFarendTime = Tools::CurrentTimestamp();				

				/*MediaLog(CODE_TRACE, "WebRtcAec_BufferFarend successful id = %lld err =  iCounter = %d iCounter2 = %d"
				, m_ID, (int)WebRtcAecm_get_error_code(AECM_instance), iCounter, iCounter2);				*/
			}
			m_bNearEndingOrFarEnding = false;
		}
#endif

		return true;
	}
} //namespace MediaSDK

