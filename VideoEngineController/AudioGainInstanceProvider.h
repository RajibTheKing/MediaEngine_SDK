#ifndef AUDIO_GAIN_INSTANCE_PROVIDER_H
#define AUDIO_GAIN_INSTANCE_PROVIDER_H

#include "SmartPointer.h"

class AudioGainInterface;

enum AudioGainType
{
	WebRTC_AGC,
	GomGomGain_AGC,
	Naive_AGC
};


class AudioGainInstanceProvider
{

public:

	static SmartPointer<AudioGainInterface> GetAudioGainInstance(AudioGainType audioGainType);

};


#endif  // !AUDIO_GAIN_INSTANCE_PROVIDER_H
