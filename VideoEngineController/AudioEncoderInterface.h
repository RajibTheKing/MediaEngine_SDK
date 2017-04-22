#ifndef AUDIO_ENCODER_INTERFACE_H
#define AUDIO_ENCODER_INTERFACE_H



class AudioEncoderInterface
{
public:

	virtual int CreateAudioEncoder() = 0;
	virtual int EncodeAudio(short *in_data, unsigned int in_size, unsigned char *out_buffer) = 0;

	virtual bool SetBitrate(int nBitrate) = 0;
	virtual bool SetComplexity(int nComplexity) = 0;

	virtual int GetCurrentBitrate() = 0;

	virtual void DecideToChangeBitrate(int iNumPacketRecvd) = 0;
	virtual void DecideToChangeComplexity(int iEncodingTime) = 0;

	~AudioEncoderInterface() { }
};


#endif  // !AUDIO_ENCODER_INTERFACE_H


// AudioEncoderInterface --> EncoderOpus / EncoderPCM --> AudioEncoderProvider

