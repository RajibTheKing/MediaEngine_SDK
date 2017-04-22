#ifndef AUDIO_DECODER_PROVIDER_H
#define AUDIO_DECODER_PROVIDER_H


#include "SmartPointer.h"
#include "AudioDecoderInterface.h"


enum AudioDecoderType
{
	AAC_Decoder,
	Opus_Decoder,
	PCM_Decoder
};


class AudioDecoderProvider
{

public:

	static SmartPointer<AudioDecoderInterface> GetAudioDecoder(AudioDecoderType audioDecoderType);

};



#endif  // !AUDIO_DECODER_PROVIDER_H



