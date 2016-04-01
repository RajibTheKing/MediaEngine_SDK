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

    int SetBitrate(int iFps);
    void SetNetworkType(int iNetworkType);
	int SetMaxBitrate(int iFps);
    int GetBitrate();
    int GetMaxBitrate();

private:

	int m_iHeight;
	int m_iWidth;
    
    int m_iNetworkType;
	Tools m_Tools;
    
    int m_iMaxBitrate;
    int m_iBitrate;

	ISVCEncoder* m_pSVCVideoEncoder;
	CCommonElementsBucket* m_pCommonElementsBucket;

protected:

	SmartPointer<CLockHandler> m_pMediaSocketMutex;
};

#endif