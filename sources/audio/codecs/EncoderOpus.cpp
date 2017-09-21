#include "EncoderOpus.h"
#include "LogPrinter.h"
#include "Tools.h"
#include "MediaLogger.h"

#include <stdlib.h>

namespace MediaSDK
{

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
		int dummyDataSize = MAX_AUDIO_FRAME_SAMPLE_SIZE;

		for (int i = 0; i < dummyDataSize; i++)
		{
			m_DummyData[i] = rand();
		}

		int iApplicationType = OPUS_APPLICATION_AUDIO;
		int sampling_rate = AUDIO_SAMPLE_RATE;
		int iAudioChannelType = AUDIO_CHANNELS;

		encoder = opus_encoder_create(sampling_rate, iAudioChannelType, iApplicationType, &err);

		if (err < 0)
		{
			return EXIT_FAILURE;
		}

		SetBitrate(OPUS_BITRATE_INIT_CALL);
		
		int iSignalType = OPUS_SIGNAL_MUSIC;
		opus_encoder_ctl(encoder, OPUS_SET_SIGNAL(iSignalType));

		//We work with mono sound, so tell it to the encoder
		opus_encoder_ctl(encoder, OPUS_SET_FORCE_CHANNELS(AUDIO_CHANNEL_MONO));

		//Set the audio bandwidth to auto
		opus_encoder_ctl(encoder, OPUS_SET_MAX_BANDWIDTH(OPUS_AUTO));

		//Set the default packet loss on network
		opus_encoder_ctl(encoder, OPUS_SET_PACKET_LOSS_PERC(AUDIO_PACKET_LOSS_DEFAULT));

		//Enable the Forward error correction to provide robustness against packet loss
		opus_encoder_ctl(encoder, OPUS_SET_INBAND_FEC(1)); //1: Enable, 0: Disable

		//Enable discontinuous transmission that shall reduce
		//the bitrate during silence or background noise
		opus_encoder_ctl(encoder, OPUS_SET_DTX(1)); ///1: Enable, 0: Disable

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

		MediaLog(LOG_DEBUG, "[EO] OPUS-SETTING# SampleRate = %d, BitRate = %d, Complexity = %d, AppType = %d, Channels = %d, SignalType = %d",
			sampling_rate, OPUS_BITRATE_INIT_CALL, m_iComplexity, iApplicationType, iAudioChannelType, iSignalType);

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

	bool EncoderOpus::SetAudioQuality(int level)
	{
		int packetLoss; //in percentage
		bool disablePrediction;

		switch (level)
		{
		case 1:
			packetLoss = 25;
			disablePrediction = false; //Enabled
			break;
		case 2:
			packetLoss = 9;
			disablePrediction = false; //Enabled
			break;

		case 3:
			packetLoss = 3;
			disablePrediction = true; //Disable prediction when network is too bad
			break;
		}

		//Sets the packet loss percentage
		opus_encoder_ctl(encoder, OPUS_SET_PACKET_LOSS_PERC(packetLoss));

		//Sets the prediction state
		opus_encoder_ctl(encoder, OPUS_SET_PREDICTION_DISABLED((int)disablePrediction));

		return true;
	}

} //namespace MediaSDK
