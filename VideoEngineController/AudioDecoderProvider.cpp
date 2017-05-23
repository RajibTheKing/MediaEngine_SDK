#include "AudioDecoderProvider.h"
#include "AudioDecoderInterface.h"

#include "DecoderAAC.h"
#include "DecoderOpus.h"
#include "DecoderPCM.h"

namespace MediaSDK
{


	SmartPointer<AudioDecoderInterface> AudioDecoderProvider::GetAudioDecoder(AudioDecoderType audioDecoderType)
	{
		AudioDecoderInterface *audioDecoder = nullptr;

		switch (audioDecoderType)
		{
		case AAC_Decoder:
			audioDecoder = new DecoderAAC();
			break;

		case Opus_Decoder:
			audioDecoder = new DecoderOpus();
			break;

		case PCM_Decoder:
			audioDecoder = new DecoderPCM();
			break;

		default:
			//Do nothing;
			break;
		}

		return SmartPointer<AudioDecoderInterface>(audioDecoder);
	}

} //namespace MediaSDK



