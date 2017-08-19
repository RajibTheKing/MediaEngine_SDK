#ifndef AUDIO_GAIN_INTERFACE_H
#define AUDIO_GAIN_INTERFACE_H

#include "AudioMacros.h"

#define MAX_GAIN 12
#ifdef defined(TARGET_OS_IPHONE) || defined(TARGET_IPHONE_SIMULATOR)
#define DEFAULT_GAIN 9
#else
#define DEFAULT_GAIN 10
#endif

namespace MediaSDK
{

	class AudioGainInterface
	{

	public:

		virtual bool SetGain(int iGain) = 0;

		virtual bool AddFarEnd(short *sInBuf, int nBufferSize) = 0;

		virtual bool AddGain(short *sInBuf, int nBufferSize, bool isLiveStreamRunning) = 0;

		virtual ~AudioGainInterface() { }
	};

} //namespace MediaSDK

#endif  // !AUDIO_GAIN_INTERFACE_H


// AudioGainInterface <-- WebRTCGain / CGomGomGain / NaiveGain <-- AudioGainInstanceProvider 