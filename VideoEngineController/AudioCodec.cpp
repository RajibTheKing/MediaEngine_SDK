#include "AudioCodec.h"
#include "CommonElementsBucket.h"
#include "DefinedDataTypes.h"
#include "LogPrinter.h"
#include "G729CodecNative.h"



CAudioCodec::CAudioCodec(CCommonElementsBucket* sharedObject) :
m_pCommonElementsBucket(sharedObject)
{
	m_pMediaSocketMutex.reset(new CLockHandler);
	
	m_inoLOssSlot = 0;
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
	int sampling_rate = 8000;
	//encoder_ = opus_encoder_create(sampling_rate, 1, OPUS_APPLICATION_AUDIO, &error);

	/*encode = opus_encoder_create(sampling_rate, 2, OPUS_APPLICATION_VOIP, &err);
	decode = opus_decoder_create(sampling_rate, 2, &err);
	opus_encoder_ctl(encode, OPUS_GET_BANDWIDTH(&length));
	length = 480;

	if (error != 0) {
		qWarning() << "opus_encoder_create() falied:" << error;
	}*/

	encoder = opus_encoder_create(SAMPLE_RATE, CHANNELS, APPLICATION, &err);
	if (err<0) return EXIT_FAILURE;

	err = opus_encoder_ctl(encoder, OPUS_SET_BITRATE(AUDIO_BITRATE_INIT));
	if (err<0) return EXIT_FAILURE;

	//err = opus_encoder_ctl(encoder, OPUS_SET_COMPLEXITY(10));
	//if (err<0) return EXIT_FAILURE;

	//err = opus_encoder_ctl(encoder,  OPUS_SET_SIGNAL(OPUS_SIGNAL_VOICE));
	//if (err<0) return EXIT_FAILURE;


	decoder = opus_decoder_create(SAMPLE_RATE, CHANNELS, &err);
	if (err<0) return EXIT_FAILURE;


	//encodeDecodeTest();

	return 1;
}

int CAudioCodec::encodeAudio(short *in_data, unsigned int in_size, unsigned char *out_buffer)
{
	//m_Tools.WriteToFile(in_data, (int)in_size, 1);

	int nbBytes = opus_encode(encoder, in_data, FRAME_SIZE, out_buffer, MAX_PACKET_SIZE);
	if (nbBytes < 0)
	{
		fprintf(stderr, "encode failed: %s\n", opus_strerror(nbBytes));
		return EXIT_FAILURE;
	}
	return nbBytes;
}

int CAudioCodec::decodeAudio(unsigned char *in_data, unsigned int in_size, short *out_buffer)
{
	int frame_size = opus_decode(decoder, in_data, in_size, out_buffer, MAX_FRAME_SIZE, 0);
	if (frame_size<0)
	{
		fprintf(stderr, "decoder failed: %s\n", opus_strerror(err));
		return EXIT_FAILURE;
	}
	return  frame_size;
}

void CAudioCodec::DecideToChangeBitrate(int iNumPacketRecvd)
{
	if (iNumPacketRecvd == AUDIO_SLOT_SIZE)
	{
		m_inoLOssSlot++;
	}
	else
	{
		m_inoLOssSlot = 0;
		SetBitrateOpus((iNumPacketRecvd * m_iCurrentBitRate) / AUDIO_SLOT_SIZE);
	}

	if (m_inoLOssSlot == AUDIO_MAX_NO_LOSS_SLOT)
	{
		if (m_iCurrentBitRate + AUDIO_BITRATE_UP_STEP <= AUDIO_MAX_BITRATE)
		{
			SetBitrateOpus(m_iCurrentBitRate + AUDIO_BITRATE_UP_STEP);
		}
		m_inoLOssSlot = 0;
	}
}

bool CAudioCodec::SetBitrateOpus(int nBitrate){
	int ret = opus_encoder_ctl(encoder, OPUS_SET_BITRATE(nBitrate));
	m_iCurrentBitRate = nBitrate;
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
		CLogPrinter_WriteSpecific5(CLogPrinter::DEBUGS, "failed to create an encoder: "); //+ opus_strerror(err)
		return EXIT_FAILURE;
	}

	/*Set the desired bit-rate. You can also set other parameters if needed.
              The Opus library is designed to have good defaults, so only set
              parameters you know you need. Doing otherwise is likely to result
              in worse quality, but better. */

	err = opus_encoder_ctl(encoder, OPUS_SET_BITRATE(BITRATE));
	if (err<0)
	{
		CLogPrinter_WriteSpecific5(CLogPrinter::DEBUGS, "failed to set bitrate: "); //+ opus_strerror(err)
		return EXIT_FAILURE;
	}


	fin = fopen("/sdcard/pcmDataDump.pcm", "r");
	if (fin==NULL)
	{
		CLogPrinter_WriteSpecific5(CLogPrinter::DEBUGS,  "failed to open file: "); //+ strerror(errno)
		return EXIT_FAILURE;
	}


/* Create a new decoder state. */

	decoder = opus_decoder_create(SAMPLE_RATE, CHANNELS, &err);
	if (err<0)
	{
		CLogPrinter_WriteSpecific5(CLogPrinter::DEBUGS, "failed to create decoder: "); //+ opus_strerror(err)
		return EXIT_FAILURE;
	}

	//fout = fopen(outFile, "w");
	fout = fopen("/sdcard/outputData.pcm", "w");
	if (fout==NULL)
	{
		CLogPrinter_WriteSpecific5(CLogPrinter::DEBUGS, "failed to open file: "); //+ strerror(errno)
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

		CLogPrinter_WriteSpecific5(CLogPrinter::DEBUGS, "Test_opus -->  FRAME_SIZE "+ Tools::IntegertoStringConvert(FRAME_SIZE) + " endoedSize: " + Tools::IntegertoStringConvert(nbBytes));
		if (nbBytes<0)
		{
			CLogPrinter_WriteSpecific5(CLogPrinter::DEBUGS, "encode failed: ");  // + opus_strerror(nbBytes)
			return EXIT_FAILURE;
		}



/* Decode the data. In this example, frame_size will be constant because
             the encoder is using a constant frame size. However, that may not
             be the case for all encoders, so the decoder must always check
             the frame size returned. */

		frame_size = opus_decode(decoder, cbits, nbBytes, out, MAX_FRAME_SIZE, 0);
		if (frame_size<0)
		{
			CLogPrinter_WriteSpecific5(CLogPrinter::DEBUGS, "decoder failed: " );  // + opus_strerror(err)
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

