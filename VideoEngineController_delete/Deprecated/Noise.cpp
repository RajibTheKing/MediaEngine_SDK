#include "Noise.h"
#include "AudioCallSession.h"

#define ANS_SAMPLES_IN_FRAME 80
#define Mild 0
#define Medium 1
#define Aggressive 2

CNoise::CNoise()
{
	int ansret = -1;
	if ((ansret = WebRtcNs_Create(&NS_instance)))
	{
		ALOG("WebRtcNs_Create failed with error code = " + m_Tools.IntegertoStringConvert(ansret));
	}
	else
	{
		ALOG("WebRtcNs_Create successful");
	}
	if ((ansret = WebRtcNs_Init(NS_instance, AUDIO_SAMPLE_RATE)))
	{
		ALOG("WebRtcNs_Init failed with error code= " + m_Tools.IntegertoStringConvert(ansret));
	}
	else
	{
		ALOG("WebRtcNs_Init successful");
	}

	if ((ansret = WebRtcNs_set_policy(NS_instance, Medium)))
	{
		ALOG("WebRtcNs_set_policy failed with error code = " + m_Tools.IntegertoStringConvert(ansret));
	}
	else
	{
		ALOG("WebRtcNs_set_policy successful");
	}
}


CNoise::~CNoise()
{
	WebRtcNs_Free(NS_instance);
}


int CNoise::Denoise(short *sInBuf, int sBufferSize, short * sOutBuf, bool isLiveStreamRunning)
{
	long long llNow = m_Tools.CurrentTimestamp();
	for (int i = 0; i < CURRENT_AUDIO_FRAME_SAMPLE_SIZE(isLiveStreamRunning); i += ANS_SAMPLES_IN_FRAME)
	{
		if (0 != WebRtcNs_Process(NS_instance, sInBuf + i, NULL, sOutBuf + i, NULL))
		{
			ALOG("WebRtcNs_Process failed");
		}
	}
	if (memcmp(sInBuf, sOutBuf, sBufferSize * sizeof(short)) == 0)
	{
		ALOG("WebRtcNs_Process did nothing but took " + m_Tools.LongLongtoStringConvert(m_Tools.CurrentTimestamp() - llNow));
		return false;
	}
	else
	{
		ALOG("WebRtcNs_Process tried to do something, believe me :-(. It took " + m_Tools.LongLongtoStringConvert(m_Tools.CurrentTimestamp() - llNow));
		return true;
	}
}
