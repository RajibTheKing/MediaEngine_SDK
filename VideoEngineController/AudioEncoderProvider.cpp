#include "AudioEncoderProvider.h"
#include "AudioEncoderOpus.h"
#include "AudioNoEncoder.h"



AudioEncoderInterface* AudioEncoderProvider::GetAudioEncoder(AudioEncoderType audioEncoderType, CCommonElementsBucket* sharedObject = nullptr, CAudioCallSession * AudioCallSession = nullptr, LongLong llfriendID = -1)
{
	switch (audioEncoderType)
	{
	case Opus:
		return new AudioEncoderOpus(sharedObject, AudioCallSession, llfriendID);

	case NoEncoder:
		return new AudioNoEncoder();

	default:
		return nullptr;
	}

}


