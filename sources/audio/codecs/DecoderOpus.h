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

		/**
		Sets the decoder gain level.

		@param [in] gainLevel The gain factor with which the decoded output shall be scaled. Possible value is 0 to 10.
		*/
		bool SetGain(int gainLevel);

		int DecodeAudio(unsigned char *in_data, unsigned int in_size, short *out_buffer);

	};

} //namespace MediaSDK

#endif  // !AUDIO_OPUS_DECODER_H

