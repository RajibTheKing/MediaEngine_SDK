#include "AudioDecoderProvider.h"
#include "AudioDecoderInterface.h"

#include "AudioAacDecoder.h"
#include "AudioOpusDecoder.h"
#include "AudioPCMDecoder.h"



SmartPointer<AudioDecoderInterface> AudioDecoderProvider::GetAudioDecoder(AudioDecoderType audioDecoderType)
{
	AudioDecoderInterface *audioDecoder = nullptr;

	switch (audioDecoderType)
	{
	case AAC_Decoder:
		audioDecoder = new AudioAacDecoder();
		break;

	case Opus_Decoder:
		audioDecoder = new AudioOpusDecoder();
		break;

	case PCM_Decoder:
		audioDecoder = new AudioPCMDecoder();
		break;

	default:
		//Do nothing;
		break;
	}

	return SmartPointer<AudioDecoderInterface>(audioDecoder);
}




