
#ifndef _VIDEO_DECODING_THREAD_H_
#define _VIDEO_DECODING_THREAD_H_

#include "Tools.h"
#include "SmartPointer.h"
#include "LogPrinter.h"
#include "EncodedFrameDepacketizer.h"
#include "RenderingBuffer.h"
#include "VideoDecoder.h"
#include "ColorConverter.h"
#include "DecodingBuffer.h"
#include "FPSController.h"

#include <thread>

class CVideoDecodingThread
{

public:

	CVideoDecodingThread(CEncodedFrameDepacketizer *encodedFrameDepacketizer, CRenderingBuffer *renderingBuffer, CVideoDecoder *videoDecoder, CColorConverter *colorConverter);
	~CVideoDecodingThread();

	void StartDecodingThread();
	void StopDecodingThread();
	void DecodingThreadProcedure();
	static void *CreateDecodingThread(void* param);

	int DecodeAndSendToClient(unsigned char *in_data, unsigned int frameSize, int nFramNumber, unsigned int nTimeStampDiff);

private:

	bool bDecodingThreadRunning;
	bool bDecodingThreadClosed;

	int m_decodingHeight;
	int m_decodingWidth;
	int m_decodedFrameSize;

	CEncodedFrameDepacketizer *m_pEncodedFrameDepacketizer;		// bring
	CRenderingBuffer *m_RenderingBuffer;							// bring
	CVideoDecoder *m_pVideoDecoder;								// bring
	CColorConverter *m_pColorConverter;							// bring

	unsigned char m_DecodedFrame[MAX_VIDEO_DECODER_FRAME_SIZE];
	unsigned char m_PacketizedFrame[MAX_VIDEO_DECODER_FRAME_SIZE];
	unsigned char m_RenderingRGBFrame[MAX_VIDEO_DECODER_FRAME_SIZE];

	Tools m_Tools;

	SmartPointer<std::thread> pDecodingThread;
};

#endif 