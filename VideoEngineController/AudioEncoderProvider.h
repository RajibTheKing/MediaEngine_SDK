#ifndef AUDIO_ENCODER_PROVIDER_H
#define AUDIO_ENCODER_PROVIDER_H

#include "AudioTypes.h"
#include "SmartPointer.h"


namespace MediaSDK
{

	class AudioEncoderInterface;

	class AudioEncoderProvider
	{
	public:

		static SharedPointer<AudioEncoderInterface> GetAudioEncoder(AudioEncoderType audioEncoderType);

	};

} //namespace MediaSDK



#endif  // !AUDIO_ENCODER_PROVIDER_H
