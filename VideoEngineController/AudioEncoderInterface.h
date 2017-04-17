#ifndef AUDIO_ENCODER_INTERFACE_H
#define AUDIO_ENCODER_INTERFACE_H


struct EncoderConfig
{
	int bitrate;
	int complexity;

	EncoderConfig() {
		bitrate = -1;
		complexity = -1;
	}
};


class AudioEncoderInterface
{
public:

	virtual bool SetEncoderConfig(EncoderConfig encoderConfig) = 0;

	virtual int EncodeAudio(short *in_data, unsigned int in_size, unsigned char *out_buffer) = 0;

	~AudioEncoderInterface() { }
};


#endif  // !AUDIO_ENCODER_INTERFACE_H
