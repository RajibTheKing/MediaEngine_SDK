#ifndef AUDIO_ENCODER_PROVIDER_H
#define AUDIO_ENCODER_PROVIDER_H


#include "AudioEncoderInterface.h"
#include "AudioCallSession.h"


enum AudioEncoderType
{
	Opus,
	NoEncoder
};


class AudioEncoderProvider
{
public:

	static AudioEncoderInterface* GetAudioEncoder(AudioEncoderType audioEncoderType, CCommonElementsBucket* sharedObject, CAudioCallSession * AudioCallSession, LongLong llfriendID);

};




#endif  // !AUDIO_ENCODER_PROVIDER_H
