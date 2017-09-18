#ifndef AUDIO_OPUS_DECODER_H
#define AUDIO_OPUS_DECODER_H


#include "AudioDecoderInterface.h"

struct OpusDecoder;

namespace MediaSDK
{

	class DecoderOpus : public AudioDecoderInterface
	{
		int err;
		OpusDecoder	*decoder;

	public:

		DecoderOpus();

		~DecoderOpus();

		bool SetParameters(int sampleRate, int numberOfChannels) { return true; }

		int DecodeAudio(unsigned char *in_data, unsigned int in_size, short *out_buffer);

	};

} //namespace MediaSDK

#endif  // !AUDIO_OPUS_DECODER_H

