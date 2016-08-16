#ifndef _AUDIO_DECODER_H_
#define _AUDIO_DECODER_H_

#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <string>

#include "SmartPointer.h"
#include "LockHandler.h"
#include "ThreadTools.h"
#include "LockHandler.h"
#include "AudioVideoEngineDefinitions.h"

namespace IPV
{
	class thread;
}

class CCommonElementsBucket;

class CAudioDecoder
{

public:

	CAudioDecoder(CCommonElementsBucket* sharedObject);
	~CAudioDecoder();

	int CreateAudioDecoder();
	int Decode(unsigned char *in_data, unsigned int in_size, unsigned char *out_data);

private:

	CCommonElementsBucket* m_pCommonElementsBucket;

};

#endif