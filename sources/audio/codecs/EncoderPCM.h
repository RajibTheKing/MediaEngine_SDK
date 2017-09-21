#ifndef AUDIO_NO_ENCODER_H
#define AUDIO_NO_ENCODER_H


#include "AudioEncoderInterface.h"


namespace MediaSDK
{
	class EncoderPCM : public AudioEncoderInterface
	{

	public:
		
		EncoderPCM()  { }
		~EncoderPCM() { }

		int CreateAudioEncoder() { return true; }
		int EncodeAudio(short *in_data, unsigned int in_size, unsigned char *out_buffer);

		bool SetBitrate(int nBitrate)       { return true; }
		bool SetComplexity(int nComplexity) { return true; }

		/**
		Sets the strength of the network that the encoders shall use for quality adaption.
		Server considers strength as STRONG(3) when packet loss is 0%-3%, MEDIUM(2) when loss is 4%-12% and WEAK(1) when loss is 13%-25%

		@param [in] level The current strength level of the network

		@return Returns true when sets the level successfully, false otherwise.
		*/

		bool SetAudioQuality(int level) override { return true; }; //Does nothing for PCM

		int GetCurrentBitrate() { return -1; }
		int GetComplexity()     { return -1; }

		void DecideToChangeBitrate(int iNumPacketRecvd)  {  }
		void DecideToChangeComplexity(int iEncodingTime) {  }

	};

} //namespace MediaSDK


#endif  // !AUDIO_NO_ENCODER_H


