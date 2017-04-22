
#ifndef IPV_VIDEO_DECODER_H
#define IPV_VIDEO_DECODER_H

//#define _CRT_SECURE_NO_WARNINGS

#include "SmartPointer.h"
#include "LockHandler.h"
#include "codec_api.h"

class CCommonElementsBucket;

class CVideoDecoder
{

public:

	CVideoDecoder(CCommonElementsBucket* pSharedObject);
	~CVideoDecoder();

	int CreateVideoDecoder();
	int SetDecoderOption(int nKey, int nValue);
	int DecodeVideoFrame(unsigned char *ucaDecodingVideoFrameData, unsigned int unLength, unsigned char *ucaDecodedVideoFrameData, int &rnVideoHeight, int &rnVideoWidth);

private:

	CCommonElementsBucket* m_pcCommonElementsBucket;
	ISVCDecoder* m_pcSVCVideoDecoder;

};

#endif