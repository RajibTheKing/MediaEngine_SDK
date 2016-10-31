
#include "CommonElementsBucket.h"
#include "DefinedDataTypes.h"
#include "LogPrinter.h"
#ifdef OPUS_ENABLE
#include "AudioCodec.h"
#else
#include "G729CodecNative.h"
#endif
//int g_StopVideoSending = 0;
//extern int g_iNextPacketType;

#define BYTES_TO_STORE_AUDIO_EFRAME_LEN 2

CAudioCodec::CAudioCodec(CCommonElementsBucket* sharedObject, CAudioCallSession * AudioCallSession) :
m_pCommonElementsBucket(sharedObject),
m_bAudioQualityLowNotified(false),
m_bAudioQualityHighNotified(false),
m_bAudioShouldStopNotified(false)
{
	m_pMediaSocketMutex.reset(new CLockHandler);
	m_pAudioCallSession = AudioCallSession;
	m_inoLossSlot = 0;
	m_ihugeLossSlot = 0;
	CLogPrinter_Write(CLogPrinter::INFO, "CAudioCodec::CAudioCodec");
}

CAudioCodec::~CAudioCodec()
{

	opus_encoder_destroy(encoder);
	opus_decoder_destroy(decoder);

	SHARED_PTR_DELETE(m_pMediaSocketMutex);
}

int CAudioCodec::CreateAudioEncoder()
{
	CLogPrinter_Write(CLogPrinter::INFO, "CAudioCodec::CreateAudioEncoder");

	int error = 0;
	int sampling_rate = AUDIO_SAMPLE_RATE;
	int dummyDataSize = AUDIO_CLIENT_SAMPLE_SIZE;
	dummyData = new opus_int16[dummyDataSize];
	unsigned char * dummyDataOut = new unsigned char[dummyDataSize * 2];
	for (int i = 0; i < dummyDataSize; i++)
	{
		dummyData[i] = rand();
	}

	//encoder_ = opus_encoder_create(sampling_rate, 1, OPUS_APPLICATION_AUDIO, &error);

	/*encode = opus_encoder_create(sampling_rate, 2, OPUS_APPLICATION_VOIP, &err);
	decode = opus_decoder_create(sampling_rate, 2, &err);
	opus_encoder_ctl(encode, OPUS_GET_BANDWIDTH(&length));
	length = 480;

	if (error != 0) {
		qWarning() << "opus_encoder_create() falied:" << error;
	}*/
//	ALOG("#BR# opus_encoder_create init: ");

	encoder = opus_encoder_create(AUDIO_SAMPLE_RATE, AUDIO_CHANNELS, AUDIO_APPLICATION, &err);
	if (err<0)
	{
//		ALOG("#BR# opus_encoder_create failed: " + m_Tools.IntegertoStringConvert(err));
		return EXIT_FAILURE;
	}

	/*err = opus_encoder_ctl(encoder, OPUS_SET_BITRATE(AUDIO_BITRATE_INIT));
	if (err<0) return EXIT_FAILURE;*/
	SetBitrateOpus(AUDIO_BITRATE_INIT);


	m_iComplexity = 10;
	long long encodingTime;
	while (m_iComplexity >= 2)
	{
		opus_encoder_ctl(encoder, OPUS_SET_COMPLEXITY(m_iComplexity));
		encodingTime = m_Tools.CurrentTimestamp();
		encodeAudio(dummyData, dummyDataSize, dummyDataOut);
		encodingTime = m_Tools.CurrentTimestamp() - encodingTime;
		if (encodingTime > AUDIO_MAX_TOLERABLE_ENCODING_TIME)
		{
			m_iComplexity--;
		}
		else
		{
			break;
		}
	}
//	ALOG("#BR# m_iComplexity: " + m_Tools.IntegertoStringConvert(m_iComplexity)
//		+ "#BR# encodingTime: " + m_Tools.IntegertoStringConvert(encodingTime));
	delete dummyData;
	delete dummyDataOut;
	//err = opus_encoder_ctl(encoder, OPUS_SET_COMPLEXITY(10));
	//if (err<0) return EXIT_FAILURE;

	//err = opus_encoder_ctl(encoder,  OPUS_SET_SIGNAL(OPUS_SIGNAL_VOICE));
	//if (err<0) return EXIT_FAILURE;


	decoder = opus_decoder_create(AUDIO_SAMPLE_RATE, AUDIO_CHANNELS, &err);
	if (err<0) return EXIT_FAILURE;


	//encodeDecodeTest();

	return 1;
}

int CAudioCodec::encodeAudio(short *in_data, unsigned int in_size, unsigned char *out_buffer)
{
	//m_Tools.WriteToFile(in_data, (int)in_size, 1);
//	ALOG("#EN  #CO# InSize: "+Tools::IntegertoStringConvert(in_size) +"  FS: "+Tools::IntegertoStringConvert(FRAME_SIZE));
	int nbBytes;
	int nEncodedSize = 0, iFrameCounter = 0, nProcessedDataSize = 0;
//	ALOG( "#EN  In: "+Tools::IntegertoStringConvert(in_size) +"  EFC: "+Tools::IntegertoStringConvert(nEncodedSize));
	while(nProcessedDataSize + AUDIO_FRAME_SIZE <= in_size)
	{
		nbBytes = opus_encode(encoder, in_data + iFrameCounter * AUDIO_FRAME_SIZE, AUDIO_FRAME_SIZE, out_buffer + nEncodedSize + 2 * iFrameCounter + 2, AUDIO_MAX_PACKET_SIZE);
//		ALOG( "#EN   #CO# Opus--> "+Tools::IntegertoStringConvert(nbBytes)+" ["+Tools::IntegertoStringConvert( nEncodedSize + 2 * iFrameCounter));
		nbBytes = max( nbBytes, 0); //If opus return -1. Not sure about that.
		if(nbBytes == 0)
		{
//			ALOG( "#EXP#**************************** Encode Failed");
		}
		out_buffer[ nEncodedSize + BYTES_TO_STORE_AUDIO_EFRAME_LEN * iFrameCounter ] = (nbBytes & 0x000000FF);
		out_buffer[ nEncodedSize + BYTES_TO_STORE_AUDIO_EFRAME_LEN * iFrameCounter + 1 ] = (nbBytes >> 8);
		nEncodedSize += nbBytes;
		++iFrameCounter;
		nProcessedDataSize += AUDIO_FRAME_SIZE;
	}
	int nEncodedPacketLenght = nEncodedSize + BYTES_TO_STORE_AUDIO_EFRAME_LEN * iFrameCounter;
//	nbBytes = opus_encode(encoder, in_data, FRAME_SIZE, out_buffer, MAX_PACKET_SIZE);

	if (nEncodedSize < 0)
	{
		fprintf(stderr, "encode failed: %s\n", opus_strerror(nbBytes));
		return EXIT_FAILURE;
	}

	if(nProcessedDataSize != in_size)
	{
//		ALOG( "#EXP# ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^Unused Data");
	}
	return nEncodedPacketLenght;
}

int CAudioCodec::decodeAudio(unsigned char *in_data, unsigned int in_size, short *out_buffer)
{
	int frame_size, nDecodedDataSize = 0, iFrameCounter = 0, nProcessedDataSize = 0, nCurrentFrameSize = 0;
//	ALOG("#DE#:  #CO# InSize: "+m_Tools.IntegertoStringConvert(in_size));
	while(nProcessedDataSize + BYTES_TO_STORE_AUDIO_EFRAME_LEN <= in_size)
	{
		nCurrentFrameSize =  in_data[nDecodedDataSize + BYTES_TO_STORE_AUDIO_EFRAME_LEN * iFrameCounter + 1];
		nCurrentFrameSize <<= 8;
		nCurrentFrameSize += in_data[nDecodedDataSize + BYTES_TO_STORE_AUDIO_EFRAME_LEN * iFrameCounter];

		if(nProcessedDataSize + nCurrentFrameSize + BYTES_TO_STORE_AUDIO_EFRAME_LEN > in_size) {
//			ALOG("#EXP# Encoded data not matched.");
			break;
		}
//		ALOG("#DE#:  #CO# Decode: "+m_Tools.IntegertoStringConvert(nCurrentFrameSize)
//				+"  ["+ Tools::IntegertoStringConvert( nDecodedDataSize + 2*iFrameCounter) );
		if(nCurrentFrameSize < 1)
		{
//			ALOG("#EXP# ZERO Frame For Decoding");
			return 0;
		}
		frame_size = opus_decode(decoder, in_data + nDecodedDataSize + BYTES_TO_STORE_AUDIO_EFRAME_LEN * iFrameCounter + BYTES_TO_STORE_AUDIO_EFRAME_LEN,
								 nCurrentFrameSize, out_buffer + iFrameCounter * AUDIO_FRAME_SIZE, AUDIO_MAX_FRAME_SIZE, 0);
//		ALOG("#DE#:  #CO# Decode Done : " + Tools::IntegertoStringConvert(frame_size));
		nDecodedDataSize += nCurrentFrameSize;		//FRAME_SIZE
		++iFrameCounter;
		nProcessedDataSize += nCurrentFrameSize + BYTES_TO_STORE_AUDIO_EFRAME_LEN;
	}

	if (nDecodedDataSize<0)
	{
		fprintf(stderr, "decoder failed: %s\n", opus_strerror(err));
		return EXIT_FAILURE;
	}
	return  iFrameCounter * AUDIO_FRAME_SIZE;
}

void CAudioCodec::DecideToChangeBitrate(int iNumPacketRecvd)
{
#ifndef AUDIO_FIXED_BITRATE
//	ALOG("#BR# DecideToChangeBitrate: "+m_Tools.IntegertoStringConvert(iNumPacketRecvd));
	if (iNumPacketRecvd == AUDIO_SLOT_SIZE)
	{
		m_inoLossSlot++;
		m_ihugeLossSlot = 0;
	}
	else
	{
		m_inoLossSlot = 0;
		int nChangedBitRate = (iNumPacketRecvd * m_iCurrentBitRate) / AUDIO_SLOT_SIZE;
//		ALOG("now br trying to set : "+Tools::IntegertoStringConvert(nChangedBitRate));
		
		if (nChangedBitRate < AUDIO_LOW_BITRATE && nChangedBitRate >= AUDIO_MIN_BITRATE)
		{
			m_ihugeLossSlot = 0;

			SetBitrateOpus(nChangedBitRate);

			if (false == m_bAudioQualityLowNotified)
			{
				m_pCommonElementsBucket->m_pEventNotifier->fireNetworkStrengthNotificationEvent(200, CEventNotifier::NETWORK_STRENTH_GOOD);
			
				m_bAudioQualityLowNotified = true;
				m_bAudioQualityHighNotified = false;
				m_bAudioShouldStopNotified = false;
			}
		}
		else if (nChangedBitRate <AUDIO_MIN_BITRATE)
		{
			m_ihugeLossSlot++;

			SetBitrateOpus(AUDIO_MIN_BITRATE);

			if (false == m_bAudioShouldStopNotified && m_ihugeLossSlot >= AUDIO_MAX_HUGE_LOSS_SLOT)
			{
				m_pCommonElementsBucket->m_pEventNotifier->fireNetworkStrengthNotificationEvent(200, CEventNotifier::NETWORK_STRENTH_BAD);
				m_pCommonElementsBucket->m_pEventNotifier->fireAudioAlarm(AUDIO_EVENT_I_TOLD_TO_STOP_VIDEO, 0, 0);
				m_pAudioCallSession->m_iNextPacketType = AUDIO_NOVIDEO_PACKET_TYPE;

				m_bAudioShouldStopNotified = true;
				m_bAudioQualityHighNotified = false;
				m_bAudioQualityLowNotified = false;
			}
		}
		else if (nChangedBitRate >= AUDIO_LOW_BITRATE)
		{
			m_ihugeLossSlot = 0;

			SetBitrateOpus(nChangedBitRate);

			if (false == m_bAudioQualityHighNotified)
			{
				m_pCommonElementsBucket->m_pEventNotifier->fireNetworkStrengthNotificationEvent(200, CEventNotifier::NETWORK_STRENTH_EXCELLENT);

				m_bAudioQualityHighNotified = true;
				m_bAudioQualityLowNotified = false;
				m_bAudioShouldStopNotified = false;
			}
		}
	}

	if (m_inoLossSlot == AUDIO_MAX_NO_LOSS_SLOT)
	{
		if (m_iCurrentBitRate + AUDIO_BITRATE_UP_STEP <= AUDIO_MAX_BITRATE)
		{
			SetBitrateOpus(m_iCurrentBitRate + AUDIO_BITRATE_UP_STEP);
		}
		else
		{
			SetBitrateOpus(AUDIO_MAX_BITRATE);
		}
		m_inoLossSlot = 0;
	}
//	ALOG("#V# E: DecideToChangeBitrate: Done");
#endif
}


void CAudioCodec::DecideToChangeComplexity(int iEncodingTime)
{
	if (iEncodingTime > AUDIO_MAX_TOLERABLE_ENCODING_TIME && m_iComplexity > OPUS_MIN_COMPLEXITY)
	{
		SetComplexityOpus(m_iComplexity - 1);
	}
	if (iEncodingTime < AUDIO_MAX_TOLERABLE_ENCODING_TIME / 2 && m_iComplexity < OPUS_MAX_COMPLEXITY)
	{
		SetComplexityOpus(m_iComplexity + 1);
	}

//	ALOG("#COMPLEXITY# DecideToChangeComplexity E.Time: " + m_Tools.IntegertoStringConvert(iEncodingTime)
//		+ " m_iComplexity: " + m_Tools.IntegertoStringConvert(m_iComplexity)
//		);
}

bool CAudioCodec::SetBitrateOpus(int nBitrate){
	/*if (nBitrate >= (AUDIO_MIN_BITRATE + AUDIO_BITRATE_INIT) / 2)
	{
		g_StopVideoSending = 0;
		g_iNextPacketType = NORMALPACKET;
	}*/
	int ret = opus_encoder_ctl(encoder, OPUS_SET_BITRATE(nBitrate));
	m_iCurrentBitRate = nBitrate;

//	ALOG("#BR# =========================>  NOW BR: "+m_Tools.IntegertoStringConvert(nBitrate));

	return ret != 0;
}


bool CAudioCodec::SetComplexityOpus(int nComplexity){
	int ret = opus_encoder_ctl(encoder, OPUS_SET_COMPLEXITY(nComplexity));
	m_iComplexity = nComplexity;

//	ALOG("#COMPLEXITY# ---------------------->  NOW Complexity: " + m_Tools.IntegertoStringConvert(nComplexity));

	return ret != 0;
}

int CAudioCodec::Encode(short *in_data, unsigned int in_size, unsigned char *out_buffer)
{
	int size = Encode(in_data, in_size, out_buffer);
	return size;
}



int CAudioCodec::Decode(unsigned char *in_data, unsigned int in_size, short *out_buffer)
{
	CLogPrinter_Write(CLogPrinter::INFO, "CAudioCodec::EncodeAudioData");
	int size = Decode(in_data, in_size, out_buffer);
	return size;
}

/*
#define FRAME_SIZE 960
#define SAMPLE_RATE 48000
#define CHANNELS 2
#define APPLICATION OPUS_APPLICATION_AUDIO
#define BITRATE 64000

#define MAX_FRAME_SIZE 6*960
#define MAX_PACKET_SIZE (3*1276)
*/

#if 0
int CAudioCodec::encodeDecodeTest()
{
	char *inFile;
	FILE *fin;
	char *outFile;
	FILE *fout;
	opus_int16 in[FRAME_SIZE*CHANNELS];
	opus_int16 out[MAX_FRAME_SIZE*CHANNELS];
	unsigned char cbits[MAX_PACKET_SIZE];
	int nbBytes;


/*Holds the state of the encoder and decoder */

	OpusEncoder *encoder;
	OpusDecoder *decoder;


	int err;

/*Create a new encoder state */

	encoder = opus_encoder_create(SAMPLE_RATE, CHANNELS, APPLICATION, &err);
	if (err<0)
	{
//		CLogPrinter_WriteSpecific5(CLogPrinter::DEBUGS, "failed to create an encoder: "); //+ opus_strerror(err)
		return EXIT_FAILURE;
	}

	/*Set the desired bit-rate. You can also set other parameters if needed.
              The Opus library is designed to have good defaults, so only set
              parameters you know you need. Doing otherwise is likely to result
              in worse quality, but better. */

	err = opus_encoder_ctl(encoder, OPUS_SET_BITRATE(BITRATE));
	if (err<0)
	{
//		CLogPrinter_WriteSpecific5(CLogPrinter::DEBUGS, "failed to set bitrate: "); //+ opus_strerror(err)
		return EXIT_FAILURE;
	}


	fin = fopen("/sdcard/pcmDataDump.pcm", "r");
	if (fin==NULL)
	{
//		CLogPrinter_WriteSpecific5(CLogPrinter::DEBUGS,  "failed to open file: "); //+ strerror(errno)
		return EXIT_FAILURE;
	}


/* Create a new decoder state. */

	decoder = opus_decoder_create(SAMPLE_RATE, CHANNELS, &err);
	if (err<0)
	{
//		CLogPrinter_WriteSpecific5(CLogPrinter::DEBUGS, "failed to create decoder: "); //+ opus_strerror(err)
		return EXIT_FAILURE;
	}

	//fout = fopen(outFile, "w");
	fout = fopen("/sdcard/outputData.pcm", "w");
	if (fout==NULL)
	{
//		CLogPrinter_WriteSpecific5(CLogPrinter::DEBUGS, "failed to open file: "); //+ strerror(errno)
		return EXIT_FAILURE;
	}

	unsigned char pcm_bytes2[44];

	fread(pcm_bytes2, sizeof(short)*CHANNELS, 44, fin);
	while (1)
	{
		int i;
		unsigned char pcm_bytes[MAX_FRAME_SIZE*CHANNELS*2];
		int frame_size;


/* Read a 16 bits/sample audio frame. */

		fread(pcm_bytes, sizeof(short)*CHANNELS, FRAME_SIZE, fin);
		if (feof(fin))
			break;

/* Convert from little-endian ordering. */

		for (i=0;i<CHANNELS*FRAME_SIZE;i++)
			in[i]=pcm_bytes[2*i+1]<<8 | pcm_bytes[2*i];


/* Encode the frame. */

		nbBytes = opus_encode(encoder, in, FRAME_SIZE, cbits, MAX_PACKET_SIZE);

//		CLogPrinter_WriteSpecific5(CLogPrinter::DEBUGS, "Test_opus -->  FRAME_SIZE "+ Tools::IntegertoStringConvert(FRAME_SIZE) + " endoedSize: " + Tools::IntegertoStringConvert(nbBytes));
		if (nbBytes<0)
		{
//			CLogPrinter_WriteSpecific5(CLogPrinter::DEBUGS, "encode failed: ");  // + opus_strerror(nbBytes)
			return EXIT_FAILURE;
		}



/* Decode the data. In this example, frame_size will be constant because
             the encoder is using a constant frame size. However, that may not
             be the case for all encoders, so the decoder must always check
             the frame size returned. */

		frame_size = opus_decode(decoder, cbits, nbBytes, out, MAX_FRAME_SIZE, 0);
		if (frame_size<0)
		{
//			CLogPrinter_WriteSpecific5(CLogPrinter::DEBUGS, "decoder failed: " );  // + opus_strerror(err)
			return EXIT_FAILURE;
		}


/* Convert to little-endian ordering. */

		for(i=0;i<CHANNELS*frame_size;i++)
		{
			pcm_bytes[2*i]=out[i]&0xFF;
			pcm_bytes[2*i+1]=(out[i]>>8)&0xFF;
		}

/* Write the decoded audio to file. */

		fwrite(pcm_bytes, sizeof(short), frame_size*CHANNELS, fout);
	}

/*Destroy the encoder state*/

	opus_encoder_destroy(encoder);
	opus_decoder_destroy(decoder);
	fclose(fin);
	fclose(fout);
	return EXIT_SUCCESS;
}


#endif

