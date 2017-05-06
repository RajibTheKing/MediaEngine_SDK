#ifndef AUDIO_AAC_DECODER_H
#define AUDIO_AAC_DECODER_H


#if defined(TARGET_OS_WINDOWS_PHONE)
#include <windows.h>
#endif

#include "AudioDecoderInterface.h"
#include "Size.h"
#include "LogPrinter.h"
#include "aacdecoder_lib.h"



class DecoderAAC : public AudioDecoderInterface
{

private:

	int m_nRC;
	HANDLE_AACDECODER m_hDecoder;


protected:

	short ByteArrayToShortBE(unsigned char *byteArray);

	int ThreeBytesIntoIntBE(unsigned char *byteArray);

	void CreateConfBuf(int sampleRate, int numOfChannels, unsigned char *conf);


public:

	DecoderAAC();

	~DecoderAAC();

	bool SetParameters(int sampleRate, int numberOfChannels);
	
	int DecodeAudio(unsigned char *inputDataBuffer, unsigned int inputDataSize, short *outputDataBuffer);

};


#endif  // !AUDIO_AAC_DECODER_H


