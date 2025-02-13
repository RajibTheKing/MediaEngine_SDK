#include "DecoderOpus.h"
#include "MediaLogger.h"
#include "AudioMacros.h"
#include "opus.h"

#include <stdlib.h>

#define BYTES_TO_STORE_AUDIO_EFRAME_LEN 2



namespace MediaSDK
{

	DecoderOpus::DecoderOpus()
	{
		decoder = opus_decoder_create(AUDIO_SAMPLE_RATE, AUDIO_CHANNELS, &err);

		if (err < 0)
		{			
			//MediaLog(LOG_DEBUG, "[FE] opus_decoder_create failed: %d", err);	
		}
	}


	DecoderOpus::~DecoderOpus()
	{
		opus_decoder_destroy(decoder);
	}


	int DecoderOpus::DecodeAudio(unsigned char *in_data, unsigned int in_size, short *out_buffer)
	{
		int frame_size, nDecodedDataSize = 0, iFrameCounter = 0, nProcessedDataSize = 0, nCurrentFrameSize = 0;

		while (nProcessedDataSize + BYTES_TO_STORE_AUDIO_EFRAME_LEN <= in_size)
		{
			nCurrentFrameSize = in_data[nDecodedDataSize + BYTES_TO_STORE_AUDIO_EFRAME_LEN * iFrameCounter + 1];
			nCurrentFrameSize <<= 8;
			nCurrentFrameSize += in_data[nDecodedDataSize + BYTES_TO_STORE_AUDIO_EFRAME_LEN * iFrameCounter];

			if (nProcessedDataSize + nCurrentFrameSize + BYTES_TO_STORE_AUDIO_EFRAME_LEN > in_size) {
				break;
			}

			if (nCurrentFrameSize < 1)
			{
				return 0;
			}

			frame_size = opus_decode(decoder, in_data + nDecodedDataSize + BYTES_TO_STORE_AUDIO_EFRAME_LEN * iFrameCounter + BYTES_TO_STORE_AUDIO_EFRAME_LEN, nCurrentFrameSize, out_buffer + iFrameCounter * AUDIO_FRAME_SIZE, AUDIO_MAX_FRAME_SIZE, 0);
			nDecodedDataSize += nCurrentFrameSize;		//FRAME_SIZE
			++iFrameCounter;
			nProcessedDataSize += nCurrentFrameSize + BYTES_TO_STORE_AUDIO_EFRAME_LEN;
		}

		if (nDecodedDataSize < 0)
		{
			//		fprintf(stderr, "decoder failed: %s\n", opus_strerror(err));
			return EXIT_FAILURE;
		}

		return  iFrameCounter * AUDIO_FRAME_SIZE;
	}

	bool DecoderOpus::SetGain(int gainLevel)
	{
		if (gainLevel < 0 || gainLevel > 10)
		{
			return false;
		}

		const int gainFactor = 256; //TODO: Must adjust the gain factor using trial and error;

		return OPUS_BAD_ARG != opus_decoder_ctl(decoder, OPUS_SET_GAIN(gainLevel * gainFactor)) ? true : false;
	}

} //namespace MediaSDK
