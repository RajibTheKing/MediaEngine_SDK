#ifndef AUDIO_GAIN_INSTANCE_PROVIDER_H
#define AUDIO_GAIN_INSTANCE_PROVIDER_H


#include "AudioTypes.h"
#include "SmartPointer.h"


class AudioGainInterface;

class AudioGainInstanceProvider
{

public:

	static SmartPointer<AudioGainInterface> GetAudioGainInstance(AudioGainType audioGainType);

};


#endif  // !AUDIO_GAIN_INSTANCE_PROVIDER_H
