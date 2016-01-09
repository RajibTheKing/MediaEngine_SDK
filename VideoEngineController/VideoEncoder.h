#ifndef _ENCODER_H_
#define _ENCODER_H_

#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <string>

#include "SmartPointer.h"
#include "LockHandler.h"
#include "ThreadTools.h"
#include "typedefs.h"
#include "macros.h"
#include "codec_api.h"
#include "EncodedFramePacketizer.h"
#include "AudioVideoEngineDefinitions.h"

namespace IPV
{
	class thread;
}

class CCommonElementsBucket;

class CVideoEncoder
{

public:

	CVideoEncoder(CCommonElementsBucket* sharedObject);
	~CVideoEncoder();

	int CreateVideoEncoder(int iWidth, int iHeight);
	int EncodeAndTransfer(unsigned char *in_data, unsigned int in_size, unsigned char *out_buffer);
	CEncodedFramePacketizer* GetEncodedFramePacketizer();
	void StartVideoEncoderThread();
	void StopVideoEncoderThread();

	static void *CreateVideoEncoderThread(void* param);

private:

	int m_iHeight;
	int m_iWidth;

	ISVCEncoder* m_pSVCVideoEncoder;
	CEncodedFramePacketizer* m_pCEncodedFramePacketizer;
	CCommonElementsBucket* m_pCommonElementsBucket;

protected:

	std::thread* m_pVideoEncoderThread;

	SmartPointer<CLockHandler> m_pMediaSocketMutex;

};

#endif