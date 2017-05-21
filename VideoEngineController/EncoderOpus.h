#ifndef AUDIO_ENCODER_OPUS_H
#define AUDIO_ENCODER_OPUS_H

#include "AudioEncoderInterface.h"
#include "SmartPointer.h"
#include "AudioMacros.h"
#include "CommonTypes.h"
#include "opus.h"

#define OPUS_MIN_COMPLEXITY 1
#define OPUS_MAX_COMPLEXITY 10
#define BYTES_TO_STORE_AUDIO_EFRAME_LEN 2

//class CLockHandler;

class EncoderOpus : public AudioEncoderInterface
{
private:

	int m_iCurrentBitRate;
	int m_iComplexity;

	OpusEncoder	*encoder;
	int 		err;

	opus_int16 m_DummyData[MAX_AUDIO_FRAME_SAMPLE_SIZE + 10];
	unsigned char m_DummyDataOut[AUDIO_MAX_PACKET_SIZE];

protected:

	SmartPointer<CLockHandler> m_pMediaSocketMutex;

public:

	EncoderOpus();
	~EncoderOpus();

	int CreateAudioEncoder();
	int EncodeAudio(short *in_data, unsigned int in_size, unsigned char *out_buffer);

	bool SetBitrate(int nBitrate);
	bool SetComplexity(int nComplexity);

	int GetCurrentBitrate()
	{
		return m_iCurrentBitRate;
	}

	int GetComplexity()
	{
		return m_iComplexity;
	}
};


#endif  // !AUDIO_ENCODER_OPUS_H
