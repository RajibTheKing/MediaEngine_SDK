#ifndef AUDIO_DECODER_PROVIDER_H
#define AUDIO_DECODER_PROVIDER_H


#include "AudioTypes.h"
#include "AudioDecoderInterface.h"
#include "SmartPointer.h"


class AudioDecoderProvider
{

public:

	static SmartPointer<AudioDecoderInterface> GetAudioDecoder(AudioDecoderType audioDecoderType);

};



#endif  // !AUDIO_DECODER_PROVIDER_H



