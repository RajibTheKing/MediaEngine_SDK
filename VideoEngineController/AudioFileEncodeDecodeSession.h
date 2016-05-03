
#ifndef _AUDIO_FILE_ENCODE_DECODE_SESSION_H_
#define _AUDIO_FILE_ENCODE_DECODE_SESSION_H_

#include "G729CodecNative.h"


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

	G729CodecNative *m_pG729CodecNative;
};


#endif
