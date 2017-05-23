//
// Created by Fahad-PC on 8/29/2016.
//

#include "AudioFileCodec.h"

namespace MediaSDK
{


	CAudioFileCodec::CAudioFileCodec()
	{

	}

	CAudioFileCodec::~CAudioFileCodec()
	{
		opus_encoder_destroy(encoder);
		opus_decoder_destroy(decoder);
	}

	int CAudioFileCodec::CreateAudioEncoder()
	{
		int error = 0;

		encoder = opus_encoder_create(AUDIO_SAMPLE_RATE, AUDIO_CHANNELS, AUDIO_APPLICATION, &err);
		if (err < 0)
		{
			return EXIT_FAILURE;
		}

		decoder = opus_decoder_create(AUDIO_SAMPLE_RATE, AUDIO_CHANNELS, &err);
		if (err < 0){
			return EXIT_FAILURE;
		}

		return 1;
	}

	int CAudioFileCodec::encodeAudio(short *in_data, unsigned int in_size, unsigned char *out_buffer)
	{

		int nbBytes;
		int nEncodedSize = 0, iFrameCounter = 0, nProcessedDataSize = 0;

		while (nProcessedDataSize + AUDIO_FRAME_SIZE <= in_size)
		{
			nbBytes = opus_encode(encoder, in_data + iFrameCounter * AUDIO_FRAME_SIZE, AUDIO_FRAME_SIZE, out_buffer + nEncodedSize + 2 * iFrameCounter + 2, AUDIO_MAX_PACKET_SIZE);

			nbBytes = max(nbBytes, 0); //If opus return -1. Not sure about that.
			if (nbBytes == 0)
			{
				AFLOG("#EXP#**************************** Encode Failed");
			}
			out_buffer[nEncodedSize + 2 * iFrameCounter] = (nbBytes & 0x000000FF);
			out_buffer[nEncodedSize + 2 * iFrameCounter + 1] = (nbBytes >> 8);
			nEncodedSize += nbBytes;
			++iFrameCounter;
			nProcessedDataSize += AUDIO_FRAME_SIZE;
		}
		int nEncodedPacketLength = nEncodedSize + 2 * iFrameCounter;

		if (nEncodedSize < 0)
		{
			fprintf(stderr, "encode failed: %s\n", opus_strerror(nbBytes));
			return EXIT_FAILURE;
		}

		if (nProcessedDataSize != in_size)
		{
			AFLOG("#EXP# ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^Unused Data");
		}
		return nEncodedPacketLength;
	}

	int CAudioFileCodec::decodeAudio(unsigned char *in_data, unsigned int in_size, short *out_buffer)
	{
		int frame_size, nDecodedDataSize = 0, iFrameCounter = 0, nProcessedDataSize = 0, nCurrentFrameSize = 0;

		while (nProcessedDataSize + 2 <= in_size)
		{
			nCurrentFrameSize = in_data[nDecodedDataSize + 2 * iFrameCounter + 1];
			nCurrentFrameSize <<= 8;
			nCurrentFrameSize += in_data[nDecodedDataSize + 2 * iFrameCounter];

			if (nProcessedDataSize + nCurrentFrameSize + 2 > in_size) {
				AFLOG("#EXP# Encoded data not matched.");
				break;
			}

			if (nCurrentFrameSize < 1)
			{
				AFLOG("#EXP# ZERO Frame For Decoding");
				return 0;
			}
			frame_size = opus_decode(decoder, in_data + nDecodedDataSize + 2 * iFrameCounter + 2, nCurrentFrameSize, out_buffer + iFrameCounter * AUDIO_FRAME_SIZE, AUDIO_MAX_FRAME_SIZE, 0);

			nDecodedDataSize += nCurrentFrameSize;
			++iFrameCounter;
			nProcessedDataSize += nCurrentFrameSize + 2;
		}

		if (nDecodedDataSize < 0)
		{
			fprintf(stderr, "decoder failed: %s\n", opus_strerror(err));
			return EXIT_FAILURE;
		}
		return  iFrameCounter * AUDIO_FRAME_SIZE;
	}


	bool CAudioFileCodec::SetBitrateOpus(int nBitrate){

		int ret = opus_encoder_ctl(encoder, OPUS_SET_BITRATE(nBitrate));

		AFLOG("#BR# =========================>  NOW BR: " + m_Tools.IntegertoStringConvert(nBitrate));

		return ret != 0;
	}


	bool CAudioFileCodec::SetComplexityOpus(int nComplexity){

		int ret = opus_encoder_ctl(encoder, OPUS_SET_COMPLEXITY(nComplexity));

		AFLOG("#COMPLEXITY# ---------------------->  NOW Complexity: " + m_Tools.IntegertoStringConvert(nComplexity));

		return ret != 0;
	}

} //namespace MediaSDK

