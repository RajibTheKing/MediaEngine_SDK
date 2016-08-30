
#ifndef _AUDIO_FILE_ENCODE_DECODE_SESSION_H_
#define _AUDIO_FILE_ENCODE_DECODE_SESSION_H_

#define OPUS_ENABLE

#ifdef OPUS_ENABLE
#include "AudioFileCodec.h"
#else
#include "G729CodecNative.h"
#endif

#define OPUS_ENABLE

class CAudioFileEncodeDecodeSession
{

public:

	CAudioFileEncodeDecodeSession();
	~CAudioFileEncodeDecodeSession();

	int StartAudioEncodeDecodeSession();
	int EncodeAudioFile(short *psaEncodingDataBuffer, int nAudioFrameSize, unsigned char *ucaEncodedDataBuffer);
	int DecodeAudioFile(unsigned char *ucaDecodedDataBuffer, int nAudioFrameSize, short *psaDecodingDataBuffer);
	int StopAudioEncodeDecodeSession();

private:
#ifdef OPUS_ENABLE
	CAudioFileCodec *m_pAudioCodec;
#else
	G729CodecNative *m_pG729CodecNative;
#endif
};


#endif
