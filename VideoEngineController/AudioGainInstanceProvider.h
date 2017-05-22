#ifndef AUDIO_GAIN_INSTANCE_PROVIDER_H
#define AUDIO_GAIN_INSTANCE_PROVIDER_H


#include "AudioTypes.h"
#include "SmartPointer.h"

namespace MediaSDK
{

	class AudioGainInterface;

	class AudioGainInstanceProvider
	{

	public:

		static SmartPointer<AudioGainInterface> GetAudioGainInstance(AudioGainType audioGainType);

	};

} //namespace MediaSDK

#endif  // !AUDIO_GAIN_INSTANCE_PROVIDER_H
