#include "AudioEncoderProvider.h"
#include "EncoderOpus.h"
#include "EncoderPCM.h"



AudioEncoderInterface* AudioEncoderProvider::GetAudioEncoder(AudioEncoderType audioEncoderType, CCommonElementsBucket* sharedObject = nullptr, CAudioCallSession * AudioCallSession = nullptr, LongLong llfriendID = -1)
{
	switch (audioEncoderType)
	{
	case Opus_Encoder:
		return new EncoderOpus(sharedObject, AudioCallSession, llfriendID);

	case PCM_Encoder:
		return new EncoderPCM();

	default:
		return nullptr;
	}

}


