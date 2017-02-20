#ifndef ACC_H
#define ACC_H

#if defined(TARGET_OS_WINDOWS_PHONE)
#include <windows.h>
#endif

#include "Size.h"
#include "LogPrinter.h"
#include "aacdecoder_lib.h"


class CAac
{
private:
	int m_nRC;
	HANDLE_AACDECODER m_hDecoder;

protected:
	short ByteArrayToShortBE(unsigned char *byteArray);
	int ThreeBytesIntoIntBE(unsigned char *byteArray);
	void CreateConfBuf(int sampleRate, int numOfChannels, unsigned char *conf);

public:
	CAac();
	~CAac();

	bool SetParameters(int sampleRate, int numberOfChannels);
	bool DecodeFrame(unsigned char *inputDataBuffer, int inputDataSize, short *outputDataBuffer, int &dataSize);
};


#endif
