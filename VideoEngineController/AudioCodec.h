#ifndef _AUDIO_ENCODER_H_
#define _AUDIO_ENCODER_H_

#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <string>

#include "SmartPointer.h"
#include "LockHandler.h"
#include "ThreadTools.h"
#include "opus.h"
#include "Tools.h"

#define FRAME_SIZE 960
#define SAMPLE_RATE 16000
#define CHANNELS 1
#define APPLICATION OPUS_APPLICATION_VOIP
#define BITRATE 24000

#define MAX_FRAME_SIZE 6*960
#define MAX_PACKET_SIZE (3*1276)


namespace IPV
{
	class thread;
}

class CCommonElementsBucket;

class CAudioCodec
{

public:

	CAudioCodec(CCommonElementsBucket* sharedObject);
	~CAudioCodec();

	int CreateAudioEncoder();
	int Encode(short *in_data, unsigned int in_size, unsigned char *out_buffer);
	int Decode(unsigned char *in_data, unsigned int in_size, short *out_buffer);

	int decodeAudio(unsigned char *in_data, unsigned int in_size, short *out_buffer);
	int encodeAudio(short *in_data, unsigned int in_size, unsigned char *out_buffer);
	int encodeDecodeTest();
	bool SetBitrateOpus(int nBitrate);

private:

	CCommonElementsBucket* m_pCommonElementsBucket;

	OpusEncoder	*encoder;
	OpusDecoder	*decoder;
	opus_int32	length;
	int 		err;


	opus_int16 in[FRAME_SIZE*CHANNELS];
	opus_int16 out[MAX_FRAME_SIZE*CHANNELS];
	unsigned char cbits[MAX_PACKET_SIZE];
	int nbBytes;

	FILE *m_fin;

	Tools m_Tools;

protected:


	SmartPointer<CLockHandler> m_pMediaSocketMutex;

};

#endif