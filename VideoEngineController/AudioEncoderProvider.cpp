#include "AudioEncoderProvider.h"

#include "EncoderOpus.h"
#include "EncoderPCM.h"

namespace MediaSDK
{

	SmartPointer<AudioEncoderInterface> AudioEncoderProvider::GetAudioEncoder(AudioEncoderType audioEncoderType)
	{
		SmartPointer<AudioEncoderInterface> pInstance;

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

