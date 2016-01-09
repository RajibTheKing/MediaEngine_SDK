#ifndef __DECODER_H_
#define __DECODER_H_

#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <string>

#include "SmartPointer.h"
#include "LockHandler.h"
#include "ThreadTools.h"
#include "typedefs.h"
#include "macros.h"
#include "codec_api.h"
#include "LockHandler.h"
#include "AudioVideoEngineDefinitions.h"

namespace IPV
{
	class thread;
}

class CCommonElementsBucket;
class CDecodingBuffer;

class CVideoDecoder
{

public:

	CVideoDecoder(CCommonElementsBucket* sharedObject, CDecodingBuffer *decodingBuffer);
	~CVideoDecoder();

	int CreateVideoDecoder();
	int Decode(unsigned char *in_data, unsigned int in_size, unsigned char *out_data, int &iVideoHeight, int &iVideoWidth);

private:
	CCommonElementsBucket* m_pCommonElementsBucket;
	CDecodingBuffer *m_pDecodingBuffer;

protected:

	ISVCDecoder* m_pSVCVideoDecoder;

};

#endif