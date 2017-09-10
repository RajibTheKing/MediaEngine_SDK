#ifndef AUDIO_ENCODER_OPUS_H
#define AUDIO_ENCODER_OPUS_H

#include "AudioEncoderInterface.h"
#include "SmartPointer.h"
#include "CommonTypes.h"
#include "AudioMacros.h"
#include "opus.h"

#define OPUS_MIN_COMPLEXITY 1
#define OPUS_MAX_COMPLEXITY 10
#define BYTES_TO_STORE_AUDIO_EFRAME_LEN 2


namespace MediaSDK
{

	class EncoderOpus : public AudioEncoderInterface
	{
	private:

		int	err;
		int m_iCurrentBitRate;
		int m_iComplexity;

		opus_int16 m_DummyData[MAX_AUDIO_FRAME_SAMPLE_SIZE + 10];
		unsigned char m_DummyDataOut[AUDIO_MAX_PACKET_SIZE];

		OpusEncoder	*encoder;
		SharedPointer<CLockHandler> m_pMediaSocketMutex;


	public:

		EncoderOpus();
		~EncoderOpus();

		int CreateAudioEncoder();
		int EncodeAudio(short *in_data, unsigned int in_size, unsigned char *out_buffer);

		bool SetBitrate(int nBitrate);
		bool SetComplexity(int nComplexity);

		int GetCurrentBitrate() { return m_iCurrentBitRate; }
		int GetComplexity() 	{ return m_iComplexity; }
	};

} //namespace MediaSDK

#endif  // !AUDIO_ENCODER_OPUS_H
