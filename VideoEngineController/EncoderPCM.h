#ifndef AUDIO_NO_ENCODER_H
#define AUDIO_NO_ENCODER_H


#include "AudioEncoderInterface.h"

namespace MediaSDK
{

	class EncoderPCM : public AudioEncoderInterface
	{

	public:
		EncoderPCM() { }
		~EncoderPCM() { }

		int CreateAudioEncoder() { return true; }
		int EncodeAudio(short *in_data, unsigned int in_size, unsigned char *out_buffer);

		bool SetBitrate(int nBitrate) { return true; }
		bool SetComplexity(int nComplexity) { return true; }

		int GetCurrentBitrate() { return -1; }
		int GetComplexity(){ return -1; }

		void DecideToChangeBitrate(int iNumPacketRecvd) {  }
		void DecideToChangeComplexity(int iEncodingTime) {  }

	};


} //namespace MediaSDK


#endif  // !AUDIO_NO_ENCODER_H


