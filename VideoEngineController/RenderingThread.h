
#ifndef _VIDEO_RENDERING_THREAD_H_
#define _VIDEO_RENDERING_THREAD_H_

#include "Tools.h"
#include "SmartPointer.h"
#include "LogPrinter.h"
#include "RenderingBuffer.h"

#include "AverageCalculator.h"


#include <thread>

class CCommonElementsBucket;
class CVideoCallSession;

class CVideoRenderingThread
{

public:

	CVideoRenderingThread(LongLong friendID, CRenderingBuffer *renderingBuffer, CCommonElementsBucket* commonElementsBucket, CVideoCallSession *pVideoCallSession, bool bIsCheckCall);
	~CVideoRenderingThread();

	void StartRenderingThread();
	void StopRenderingThread();
	void RenderingThreadProcedure();
	static void *CreateVideoRenderingThread(void* param);
    void CalculateFPS();

private:

	bool bRenderingThreadRunning;
	bool bRenderingThreadClosed;

	CRenderingBuffer *m_RenderingBuffer;					
	CCommonElementsBucket* m_pCommonElementsBucket;		

	LongLong m_FriendID;								

	unsigned char m_RenderingFrame[MAX_VIDEO_DECODER_FRAME_SIZE];

	Tools m_Tools;
    int m_nRenderFrameCount;
    long long m_lRenderCallTime;

	bool m_bIsCheckCall;
    
    CVideoCallSession *m_pVideoCallSession;
    
    CAverageCalculator m_RenderTimeCalculator;
    long long m_llRenderFrameCounter;
	SmartPointer<std::thread> pRenderingThread;
};

#endif 
