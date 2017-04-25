#ifndef AUDIO_ENCODER_PROVIDER_H
#define AUDIO_ENCODER_PROVIDER_H

#include "AudioResourceTypes.h"
#include "SmartPointer.h"


class AudioEncoderInterface;

class AudioEncoderProvider
{
public:

	static SmartPointer<AudioEncoderInterface> GetAudioEncoder(AudioEncoderType audioEncoderType);

};




#endif  // !AUDIO_ENCODER_PROVIDER_H
