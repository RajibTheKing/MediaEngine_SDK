#ifndef AUDIO_ENCODER_INTERFACE_H
#define AUDIO_ENCODER_INTERFACE_H



class AudioEncoderInterface
{
public:

	virtual int EncodeAudio(short *in_data, unsigned int in_size, unsigned char *out_buffer) = 0;

	~AudioEncoderInterface() { }
};


#endif  // !AUDIO_ENCODER_INTERFACE_H
