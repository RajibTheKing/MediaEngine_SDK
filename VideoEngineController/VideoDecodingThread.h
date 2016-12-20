
#ifndef _VIDEO_DECODING_THREAD_H_
#define _VIDEO_DECODING_THREAD_H_

#include "Tools.h"
#include "SmartPointer.h"
#include "LogPrinter.h"
#include "EncodedFrameDepacketizer.h"
#include "RenderingBuffer.h"
#include "VideoDecoder.h"
#include "ColorConverter.h"
#include "FPSController.h"
#include "AverageCalculator.h"
#include "LiveVideoDecodingQueue.h"
#include "VideoHeader.h"

//#include "Helper_IOS.hpp"
#include <thread>

class CVideoCallSession;

class CVideoDecodingThread
{

public:

	CVideoDecodingThread(CEncodedFrameDepacketizer *encodedFrameDepacketizer,
                         CRenderingBuffer *renderingBuffer,
                         LiveVideoDecodingQueue *pLiveVideoDecodingQueue,
                         CVideoDecoder *videoDecoder,
                         CColorConverter *colorConverter,
                         CVideoCallSession* pVideoCallSession,
                         bool bIsCheckCall,
                         int nFPS
                         );
    
	~CVideoDecodingThread();
    void Reset();
	void StartDecodingThread();
	void StopDecodingThread();
	void DecodingThreadProcedure();
	static void *CreateDecodingThread(void* param);

	void InstructionToStop();

	void SetCallFPS(int nFPS);

	int DecodeAndSendToClient(unsigned char *in_data, unsigned int frameSize, int nFramNumber, unsigned int nTimeStampDiff, int nOrientation);

private:

	CVideoCallSession* m_pVideoCallSession;
	bool bDecodingThreadRunning;
	bool bDecodingThreadClosed;

	int m_decodingHeight;
	int m_decodingWidth;
	int m_decodedFrameSize;
    
    int m_Counter;

	int m_nCallFPS;

	CEncodedFrameDepacketizer *m_pEncodedFrameDepacketizer;		
	CRenderingBuffer *m_RenderingBuffer;						
	CVideoDecoder *m_pVideoDecoder;								
	CColorConverter *m_pColorConverter;

	bool m_bIsCheckCall;

	unsigned char m_DecodedFrame[MAX_VIDEO_DECODER_FRAME_SIZE];
	unsigned char m_PacketizedFrame[MAX_VIDEO_DECODER_FRAME_SIZE];
	unsigned char m_RenderingRGBFrame[MAX_VIDEO_DECODER_FRAME_SIZE];

	Tools m_Tools;
    CAverageCalculator *m_pCalculatorDecodeTime;
	SmartPointer<std::thread> pDecodingThread;
    
    
    
    double m_dbAverageDecodingTime = 0, m_dbTotalDecodingTime = 0;
    int m_nOponnentFPS, m_nMaxProcessableByMine;
    int m_iDecodedFrameCounter = 0;
  	long long m_nMaxDecodingTime = 0;
    
    
    int m_FpsCounter;
    long long m_FPS_TimeDiff;
    long long llQueuePrevTime;
    LiveVideoDecodingQueue *m_pLiveVideoDecodingQueue;
    
};

#endif 
