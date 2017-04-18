#ifndef AUDIO_GAIN_INSTANCE_PROVIDER_H
#define AUDIO_GAIN_INSTANCE_PROVIDER_H


#include "AudioGainInterface.h"


enum AudioGainType
{
	WebRTC,
	GomGomGain,
	Naive
};


class AudioGainInstanceProvider
{

public:

	static AudioGainInterface* GetAudioGainInstance(AudioGainType audioGainType);

};


#endif  // !AUDIO_GAIN_INSTANCE_PROVIDER_H
