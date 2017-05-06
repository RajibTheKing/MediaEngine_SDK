#ifndef AUDIO_OPUS_DECODER_H
#define AUDIO_OPUS_DECODER_H


#include <stdlib.h>

#include "AudioDecoderInterface.h"
#include "opus.h"
#include "Tools.h"
#include "size.h"
#include "AudioMacros.h"
#include "LogPrinter.h"


class DecoderOpus : public AudioDecoderInterface
{
	int err;

	OpusDecoder	*decoder;
	Tools m_Tools;


public:

	DecoderOpus();

	~DecoderOpus();

	bool SetParameters(int sampleRate, int numberOfChannels) { return true; }

	int DecodeAudio(unsigned char *in_data, unsigned int in_size, short *out_buffer);

};


#endif  // !AUDIO_OPUS_DECODER_H

