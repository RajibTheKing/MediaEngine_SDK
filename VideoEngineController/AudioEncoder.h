#ifndef _AUDIO_ENCODER_H_
#define _AUDIO_ENCODER_H_

#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <string>

#include "SmartPointer.h"
#include "LockHandler.h"
#include "ThreadTools.h"

namespace IPV
{
	class thread;
}

class CCommonElementsBucket;

class CAudioEncoder
{

public:

	CAudioEncoder(CCommonElementsBucket* sharedObject);
	~CAudioEncoder();

	int CreateAudioEncoder();
	int Encode(short *in_data, unsigned int in_size, unsigned char *out_buffer);
	int Decode(unsigned char *in_data, unsigned int in_size, short *out_buffer);
	void StartAudioEncoderThread();
	void StopAudioEncoderThread();

	static void *CreateAudioEncoderThread(void* param);

private:

	CCommonElementsBucket* m_pCommonElementsBucket;

protected:

	std::thread* m_pAudioEncoderThread;

	SmartPointer<CLockHandler> m_pMediaSocketMutex;

};

#endif