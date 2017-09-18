#ifndef AUDIO_ENCODER_INTERFACE_H
#define AUDIO_ENCODER_INTERFACE_H


namespace MediaSDK
{

	class AudioEncoderInterface
	{
	public:

		virtual int CreateAudioEncoder() = 0;
		virtual int EncodeAudio(short *in_data, unsigned int in_size, unsigned char *out_buffer) = 0;

		virtual bool SetBitrate(int nBitrate) = 0;
		virtual bool SetComplexity(int nComplexity) = 0;

		virtual int GetComplexity() = 0;
		virtual int GetCurrentBitrate() = 0;

		virtual ~AudioEncoderInterface() { }
	};

} //namespace MediaSDK

#endif  // !AUDIO_ENCODER_INTERFACE_H


// AudioEncoderInterface --> EncoderOpus / EncoderPCM --> AudioEncoderProvider

