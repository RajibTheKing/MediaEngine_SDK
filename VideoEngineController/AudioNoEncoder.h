#ifndef AUDIO_NO_ENCODER_H
#define AUDIO_NO_ENCODER_H


#include "AudioEncoderInterface.h"


class AudioNoEncoder : public AudioEncoderInterface
{

public:
	AudioNoEncoder() { }
	~AudioNoEncoder() { }

	int CreateAudioEncoder() { return true; }
	int encodeAudio(short *in_data, unsigned int in_size, unsigned char *out_buffer);

	bool SetBitrateOpus(int nBitrate) { return true; }
	bool SetComplexityOpus(int nComplexity) { return true; }

	int GetCurrentBitrateOpus() { return true; }

	void DecideToChangeBitrate(int iNumPacketRecvd) {  }
	void DecideToChangeComplexity(int iEncodingTime) {  }

};




#endif  // !AUDIO_NO_ENCODER_H


