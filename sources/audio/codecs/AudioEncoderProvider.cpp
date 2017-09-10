#include "AudioEncoderProvider.h"

#include "EncoderOpus.h"
#include "EncoderPCM.h"

namespace MediaSDK
{

	SharedPointer<AudioEncoderInterface> AudioEncoderProvider::GetAudioEncoder(AudioEncoderType audioEncoderType)
	{
		SharedPointer<AudioEncoderInterface> pInstance;

		switch (audioEncoderType)
		{
		case Opus_Encoder:
			pInstance.reset(new EncoderOpus());
			break;

		case PCM_Encoder:
			pInstance.reset(new EncoderPCM());
			break;

		default:
			break;
		}

		return pInstance;
	}

} //namespace MediaSDK

