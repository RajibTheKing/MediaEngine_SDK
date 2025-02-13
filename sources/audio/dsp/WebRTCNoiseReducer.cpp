#include "WebRTCNoiseReducer.h"

#include "AudioMacros.h"
#include "Tools.h"
#include "MediaLogger.h"



#define ANS_SAMPLES_IN_FRAME 80

//#define NOISE_DUMP

//TODO: Use neon version for android.

namespace MediaSDK
{
#ifdef NOISE_DUMP
	FILE* noiseIn = nullptr;
	FILE* noiseOut = nullptr;
#endif
	enum ANRMode{
		Mild = 0,
		Medium = 1,
		Aggressive = 2,
		Undocumented = 3
	};

	WebRTCNoiseReducer::WebRTCNoiseReducer()
	{
#ifdef NOISE_DUMP
		noiseIn = fopen("/sdcard/noise.input.pcm", "wb");
		noiseOut = fopen("/sdcard/noise.output.pcm", "wb");

		/*const std::string path = std::string(getenv("HOME")) + "/Documents/";
		std::string a = path + "noise.input.pcm";
		std::string b = path + "noise.output.pcm";
		noiseIn = fopen(a.c_str(), "wb");
		noiseOut = fopen(b.c_str(), "wb");*/
#endif

		this->Init();
	}

	WebRTCNoiseReducer::~WebRTCNoiseReducer()
	{
		Release();

#ifdef NOISE_DUMP
		if(noiseIn) fclose(noiseIn);
		if(noiseOut) fclose(noiseOut);
#endif
	}

	void WebRTCNoiseReducer::Init()
	{
#ifdef USE_ANS
		int ansret = -1;
		NS_instance = WebRtcNsx_Create();
		if ((ansret = WebRtcNsx_Init(NS_instance, AUDIO_SAMPLE_RATE)))
		{
			//MediaLog(CODE_TRACE, "WebRtcNs_Init failed with error code= %d",ansret);
		}
		else
		{
			//MediaLog(CODE_TRACE, "WebRtcNs_Init successful");
		}

		if ((ansret = WebRtcNsx_set_policy(NS_instance, Undocumented)))
		{
			//MediaLog(CODE_TRACE, "WebRtcNs_set_policy failed with error code = %d", ansret);
		}
		else
		{
			//MediaLog(CODE_TRACE, "WebRtcNs_set_policy successful");
		}

#endif
	}

	void WebRTCNoiseReducer::Release()
	{
#ifdef USE_ANS
		if (NS_instance)
		{
			WebRtcNsx_Free(NS_instance);
			NS_instance = nullptr;
		}
#endif
	}

	void WebRTCNoiseReducer::Reset()
	{
		Release();
		Init();
	}


	int WebRTCNoiseReducer::Denoise(short *sInBuf, int sBufferSize, short * sOutBuf, bool isLiveStreamRunning)
	{
		MediaLog(LOG_DEBUG, "[NR] Denoise Buffer Size: %d, Live stringming: %d",sBufferSize, isLiveStreamRunning);
#ifdef USE_ANS
		memset(m_tmpbuffer, 0, sBufferSize * sizeof(short));
#ifdef NOISE_DUMP
		fwrite(sOutBuf, 2, sBufferSize, noiseIn);
#endif

		//long long llNow = Tools::CurrentTimestamp();
		for (int i = 0; i < CURRENT_AUDIO_FRAME_SAMPLE_SIZE(isLiveStreamRunning); i += ANS_SAMPLES_IN_FRAME)
		{
			int16_t* nsIn = sInBuf + i;
			int16_t* nsOut = m_tmpbuffer + i;
			WebRtcNsx_Process(NS_instance, (const short *const *)&nsIn, 1, (short *const *)&nsOut);
		}

		memcpy(sOutBuf, m_tmpbuffer, sBufferSize * 2);

#ifdef NOISE_DUMP
		fwrite(sOutBuf, 2, sBufferSize, noiseOut);
#endif

		//LOGT("##NS NS done.Took:%lld", Tools::CurrentTimestamp() - llNow);
#endif 
		return true;
	}

} //namespace MediaSDK
