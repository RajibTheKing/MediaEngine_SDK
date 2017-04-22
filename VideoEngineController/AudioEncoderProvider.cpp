#include "AudioEncoderProvider.h"

#include "EncoderOpus.h"
#include "EncoderPCM.h"


SmartPointer<AudioEncoderInterface> AudioEncoderProvider::GetAudioEncoder(AudioEncoderType audioEncoderType, CCommonElementsBucket* sharedObject = nullptr, CAudioCallSession * AudioCallSession = nullptr, LongLong llfriendID = -1)
{
	SmartPointer<AudioEncoderInterface> pInstance;

	switch (audioEncoderType)
	{
	case Opus_Encoder:
		pInstance.reset(new EncoderOpus(sharedObject, AudioCallSession, llfriendID));
		break;

	case PCM_Encoder:
		pInstance.reset(new EncoderPCM());
		break;

	default:
		break;
	}

	return pInstance;
}


