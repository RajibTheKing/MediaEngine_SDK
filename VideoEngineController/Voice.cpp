#include "Voice.h"
#include "AudioCallSession.h"

#define VAD_ANALYSIS_SAMPLES_IN_FRAME 80
#define NEXT_N_FRAMES_MAYE_VOICE 11

CVoice::CVoice()
{
	int vadret = -1;
	if ((vadret = WebRtcVad_Create(&VAD_instance)))
	{
		ALOG("WebRtcVad_Create failed with error code = " + m_Tools.IntegertoStringConvert(vadret));
	}
	else
	{
		ALOG("WebRtcVad_Create successful");
	}

	if ((vadret = WebRtcVad_Init(VAD_instance)))
	{
		ALOG("WebRtcVad_Init failed with error code= " + m_Tools.IntegertoStringConvert(vadret));
	}
	else
	{
		ALOG("WebRtcVad_Init successful");
	}

	nNextFrameMayHaveVoice = 0;
}


CVoice::~CVoice()
{
	WebRtcVad_Free(VAD_instance);
}


bool CVoice::HasVoice(short *sInBuf, int sBufferSize)
{
	if (WebRtcVad_ValidRateAndFrameLength(AUDIO_SAMPLE_RATE, VAD_ANALYSIS_SAMPLES_IN_FRAME) == 0)
	{
		long long vadtimeStamp = m_Tools.CurrentTimestamp();
		int nhasVoice = 0;
		for (int i = 0; i < sBufferSize; i += VAD_ANALYSIS_SAMPLES_IN_FRAME)
		{
			int iVadRet = WebRtcVad_Process(VAD_instance, AUDIO_SAMPLE_RATE, sInBuf + i, VAD_ANALYSIS_SAMPLES_IN_FRAME);
			if (iVadRet != 1)
			{
				ALOG("No voice found " + Tools::IntegertoStringConvert(iVadRet));
				//memset(m_saAudioEncodingFrame + i, 0, VAD_ANALYSIS_SAMPLES_IN_FRAME * sizeof(short));						
			}
			else
			{
				ALOG("voice found " + Tools::IntegertoStringConvert(iVadRet));
				nhasVoice = 1;
				nNextFrameMayHaveVoice = NEXT_N_FRAMES_MAYE_VOICE;
			}
		}
		if (!nhasVoice)
		{
			if (nNextFrameMayHaveVoice > 0)
			{
				nNextFrameMayHaveVoice--;
			}
		}
		ALOG(" vad time = " + m_Tools.LongLongtoStringConvert(m_Tools.CurrentTimestamp() - vadtimeStamp));
		if (!nhasVoice && !nNextFrameMayHaveVoice)
		{
			ALOG("not sending audio");
			m_Tools.SOSleep(70);
			return false;
		}
		else
		{
			ALOG("sending audio");
			return true;
		}
	}
	else
	{
		ALOG("Invalid combo");
		return true;
	}
}
