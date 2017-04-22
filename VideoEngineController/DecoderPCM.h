#ifndef AUDIO_NO_DECODER_H
#define AUDIO_NO_DECODER_H


#include "AudioDecoderInterface.h"


class DecoderPCM : public AudioDecoderInterface
{

public:

	bool SetParameters(int sampleRate, int numberOfChannels) { return true; }

	int DecodeAudio(unsigned char *in_data, unsigned int in_size, short *out_buffer);

	~DecoderPCM() { }
};


#endif  // !AUDIO_NO_DECODER_H
