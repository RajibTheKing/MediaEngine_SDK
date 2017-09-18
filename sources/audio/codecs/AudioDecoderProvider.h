#ifndef AUDIO_DECODER_PROVIDER_H
#define AUDIO_DECODER_PROVIDER_H


#include "AudioTypes.h"
#include "SmartPointer.h"


namespace MediaSDK
{
	class AudioDecoderInterface;


	class AudioDecoderProvider
	{

	public:

		static SharedPointer<AudioDecoderInterface> GetAudioDecoder(AudioDecoderType audioDecoderType);

	};

} //namespace MediaSDK


#endif  // !AUDIO_DECODER_PROVIDER_H



