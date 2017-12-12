#include "Voice.h"
#include "AudioCallSession.h"
#include "AudioMacros.h"
#include "MediaLogger.h"


namespace MediaSDK
{

	#define VAD_ANALYSIS_SAMPLES_IN_FRAME 80
	#define NEXT_N_FRAMES_MAYE_VOICE 11

	CVoice::CVoice()
	{
		int vadret = -1;
		if ((vadret = WebRtcVad_Create(&VAD_instance)))
		{
			//MediaLog(CODE_TRACE, "WebRtcVad_Create failed with error code = %d",vadret);
		}
		else
		{
			//MediaLog(CODE_TRACE, "WebRtcVad_Create successful");
		}

		if ((vadret = WebRtcVad_Init(VAD_instance)))
		{
			//MediaLog(CODE_TRACE, "WebRtcVad_Init failed with error code= %d",vadret);
		}
		else
		{
			//MediaLog(CODE_TRACE, "WebRtcVad_Init successful");
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
					//MediaLog(CODE_TRACE, "No voice found %d",iVadRet);
					//memset(m_saAudioRecorderFrame + i, 0, VAD_ANALYSIS_SAMPLES_IN_FRAME * sizeof(short));						
				}
				else
				{
					//MediaLog(CODE_TRACE, "voice found %d",iVadRet);
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
			//MediaLog(CODE_TRACE, "vad time = %lld",(m_Tools.CurrentTimestamp() - vadtimeStamp));
			if (!nhasVoice && !nNextFrameMayHaveVoice)
			{
				//MediaLog(CODE_TRACE, "not sending audio");
				m_Tools.SOSleep(70);
				return false;
			}
			else
			{
				//MediaLog(CODE_TRACE, "sending audio");
				return true;
			}
		}
		else
		{
			//MediaLog(CODE_TRACE, "Invalid combo");
			return true;
		}
	}

} //namespace MediaSDK