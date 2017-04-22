#ifndef AUDIO_ENCODER_PROVIDER_H
#define AUDIO_ENCODER_PROVIDER_H

#include "SmartPointer.h"

class AudioEncoderInterface;

enum AudioEncoderType
{
	Opus_Encoder,
	PCM_Encoder
};


class AudioEncoderProvider
{
public:

	static SmartPointer<AudioEncoderInterface> GetAudioEncoder(AudioEncoderType audioEncoderType);

};




#endif  // !AUDIO_ENCODER_PROVIDER_H
