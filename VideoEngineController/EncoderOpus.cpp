#include "EncoderOpus.h"

#include "LogPrinter.h"
#include "LockHandler.h"
#include "Tools.h"


EncoderOpus::EncoderOpus() : encoder(nullptr)
{
	MR_DEBUG("#resorce#encoder#opus# EncoderOpus::EncoderOpus()");

	m_pMediaSocketMutex.reset(new CLockHandler);
}


EncoderOpus::~EncoderOpus()
{
	MR_DEBUG("#resorce#encoder#opus# EncoderOpus::~EncoderOpus()");

	if (encoder != nullptr){
		opus_encoder_destroy(encoder);
	}
}


int EncoderOpus::CreateAudioEncoder()
{
	MR_DEBUG("#resorce#encoder#opus# EncoderOpus::CreateAudioEncoder()");

	int error = 0;
	int sampling_rate = AUDIO_SAMPLE_RATE;
	int dummyDataSize = MAX_AUDIO_FRAME_SAMPLE_SIZE;

	for (int i = 0; i < dummyDataSize; i++)
	{
		m_DummyData[i] = rand();
	}

	encoder = opus_encoder_create(AUDIO_SAMPLE_RATE, AUDIO_CHANNELS, AUDIO_APPLICATION, &err);
	if (err<0)
	{
		return EXIT_FAILURE;
	}

	SetBitrate(AUDIO_BITRATE_INIT);
	
	m_iComplexity = 10;
	long long encodingTime = 0;
	while (m_iComplexity >= 2)
	{
		opus_encoder_ctl(encoder, OPUS_SET_COMPLEXITY(m_iComplexity));
		encodingTime = Tools::CurrentTimestamp();
		EncodeAudio(m_DummyData, dummyDataSize, m_DummyDataOut);
		encodingTime = Tools::CurrentTimestamp() - encodingTime;
		if (encodingTime > AUDIO_MAX_TOLERABLE_ENCODING_TIME)
		{
			m_iComplexity--;
		}
		else
		{
			break;
		}
	}
    
    return EXIT_SUCCESS;
}


int EncoderOpus::EncodeAudio(short *in_data, unsigned int in_size, unsigned char *out_buffer)
{
	MR_DEBUG("#resorce#encoder#opus# EncoderOpus::EncodeAudio()");

	int nbBytes;
	int nEncodedSize = 0, iFrameCounter = 0, nProcessedDataSize = 0;

	while (nProcessedDataSize + AUDIO_FRAME_SIZE <= in_size)
	{
		nbBytes = opus_encode(encoder, in_data + iFrameCounter * AUDIO_FRAME_SIZE, AUDIO_FRAME_SIZE, out_buffer + nEncodedSize + 2 * iFrameCounter + 2, AUDIO_MAX_PACKET_SIZE);
		nbBytes = max(nbBytes, 0); //If opus return -1. Not sure about that.
		if (nbBytes == 0)
		{
			//			ALOG( "#EXP#**************************** Encode Failed");
		}
		out_buffer[nEncodedSize + BYTES_TO_STORE_AUDIO_EFRAME_LEN * iFrameCounter] = (nbBytes & 0x000000FF);
		out_buffer[nEncodedSize + BYTES_TO_STORE_AUDIO_EFRAME_LEN * iFrameCounter + 1] = (nbBytes >> 8);
		nEncodedSize += nbBytes;
		++iFrameCounter;
		nProcessedDataSize += AUDIO_FRAME_SIZE;
	}

	int nEncodedPacketLength = nEncodedSize + BYTES_TO_STORE_AUDIO_EFRAME_LEN * iFrameCounter;

	if (nEncodedSize < 0)
	{
		fprintf(stderr, "encode failed: %s\n", opus_strerror(nbBytes));
		return EXIT_FAILURE;
	}

	if (nProcessedDataSize != in_size)
	{
		//		ALOG( "#EXP# ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^Unused Data");
	}

	return nEncodedPacketLength;
}


bool EncoderOpus::SetBitrate(int nBitrate)
{
	MR_DEBUG("#resorce#encoder#opus# EncoderOpus::SetBitrate()");

	int ret = opus_encoder_ctl(encoder, OPUS_SET_BITRATE(nBitrate));
	m_iCurrentBitRate = nBitrate;

	return ret != 0;
}


bool EncoderOpus::SetComplexity(int nComplexity)
{
	MR_DEBUG("#resorce#encoder#opus# EncoderOpus::SetComplexity()");

	int ret = opus_encoder_ctl(encoder, OPUS_SET_COMPLEXITY(nComplexity));
	m_iComplexity = nComplexity;

	return ret != 0;
}





