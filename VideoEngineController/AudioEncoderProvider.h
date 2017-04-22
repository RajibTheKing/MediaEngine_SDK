#ifndef AUDIO_ENCODER_PROVIDER_H
#define AUDIO_ENCODER_PROVIDER_H

#include "SmartPointer.h"
#include "AudioVideoEngineDefinitions.h"

class CAudioCallSession;
class CCommonElementsBucket;
class AudioEncoderInterface;

enum AudioEncoderType
{
	Opus_Encoder,
	PCM_Encoder
};


class AudioEncoderProvider
{
public:

	static SmartPointer<AudioEncoderInterface> GetAudioEncoder(AudioEncoderType audioEncoderType, CCommonElementsBucket* sharedObject, CAudioCallSession * AudioCallSession, LongLong llfriendID);

};




#endif  // !AUDIO_ENCODER_PROVIDER_H
