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

		/**
		Sets the audio quality that the encoders shall provide. Shall be used when server signals network strength
		Server considers strength as STRONG(3) when packet loss is 0%-3%, MEDIUM(2) when loss is 4%-12% and WEAK(1) when loss is 13%-25%

		@param [in] level The current strength level of the network.

		@return Returns true when sets the level successfully, false otherwise.
		*/

		virtual bool SetAudioQuality(int level) = 0;

		virtual int GetComplexity() = 0;
		virtual int GetCurrentBitrate() = 0;

		virtual ~AudioEncoderInterface() { }
	};

} //namespace MediaSDK

#endif  // !AUDIO_ENCODER_INTERFACE_H


// AudioEncoderInterface --> EncoderOpus / EncoderPCM --> AudioEncoderProvider

