
#include "RenderingThread.h"
#include "CommonElementsBucket.h"

#if defined(TARGET_OS_IPHONE) || defined(TARGET_IPHONE_SIMULATOR)
#include <dispatch/dispatch.h>
#endif

CVideoRenderingThread::CVideoRenderingThread(LongLong friendID, CRenderingBuffer *renderingBuffer, CCommonElementsBucket *commonElementsBucket) :

m_RenderingBuffer(renderingBuffer),
m_pCommonElementsBucket(commonElementsBucket),
m_FriendID(friendID)

{

}

CVideoRenderingThread::~CVideoRenderingThread()
{

}

void CVideoRenderingThread::StopRenderingThread()
{
	//if (pInternalThread.get())
	{

		bRenderingThreadRunning = false;

		while (!bRenderingThreadClosed)
		{
			m_Tools.SOSleep(5);
		}
	}

	//pInternalThread.reset();
}

void CVideoRenderingThread::StartRenderingThread()
{
	CLogPrinter_WriteLog(CLogPrinter::INFO, THREAD_LOG ,"CVideoRenderingThread::StartRenderingThread() called");

	if (pRenderingThread.get())
	{
		pRenderingThread.reset();
		return;
	}

	bRenderingThreadRunning = true;
	bRenderingThreadClosed = false;

#if defined(TARGET_OS_IPHONE) || defined(TARGET_IPHONE_SIMULATOR)

	dispatch_queue_t RenderThreadQ = dispatch_queue_create("RenderThreadQ", DISPATCH_QUEUE_CONCURRENT);
	dispatch_async(RenderThreadQ, ^{
		this->RenderingThreadProcedure();
	});

#else

	std::thread myThread(CreateVideoRenderingThread, this);
	myThread.detach();

#endif

	CLogPrinter_WriteLog(CLogPrinter::INFO, THREAD_LOG ,"CVideoRenderingThread::StartRenderingThread() Rendering Thread started");

	return;
}

void *CVideoRenderingThread::CreateVideoRenderingThread(void* param)
{
	CVideoRenderingThread *pThis = (CVideoRenderingThread*)param;
	pThis->RenderingThreadProcedure();

	return NULL;
}

void CVideoRenderingThread::RenderingThreadProcedure()
{
	CLogPrinter_WriteLog(CLogPrinter::INFO, THREAD_LOG ,"CVideoRenderingThread::RenderingThreadProcedure() started RenderingThreadProcedure method");

	Tools toolsObject;
	int frameSize, nFrameNumber, intervalTime;
	long long nTimeStampDiff;
	long long currentFrameTime, decodingTime, firstFrameEncodingTime;
	int videoHeight, videoWidth;
	long long currentTimeStamp;
	int prevTimeStamp = 0;
	int minTimeGap = 51;
	bool m_b1stDecodedFrame = true;
	long long m_ll1stDecodedFrameTimeStamp = 0;
    long long lRenderingTimeDiff = 0;
	while (bRenderingThreadRunning)
	{
		CLogPrinter_WriteLog(CLogPrinter::INFO, THREAD_LOG ,"CVideoRenderingThread::RenderingThreadProcedure() RUNNING RenderingThreadProcedure method");

		if (m_RenderingBuffer->GetQueueSize() == 0)
		{
			CLogPrinter_WriteLog(CLogPrinter::INFO, THREAD_LOG ,"CVideoRenderingThread::RenderingThreadProcedure() NOTHING for Rendering method");

			toolsObject.SOSleep(10);
		}
		else
		{

			int timeDiffForQueue;

			frameSize = m_RenderingBuffer->DeQueue(nFrameNumber, nTimeStampDiff, m_RenderingFrame, videoHeight, videoWidth, timeDiffForQueue);
			CLogPrinter_WriteLog(CLogPrinter::INFO, QUEUE_TIME_LOG ,"CVideoRenderingThread::RenderingThreadProcedure() m_RenderingBuffer " + toolsObject.IntegertoStringConvert(timeDiffForQueue));

			currentFrameTime = toolsObject.CurrentTimestamp();

			if (m_b1stDecodedFrame)
			{
				m_ll1stDecodedFrameTimeStamp = currentFrameTime;
				firstFrameEncodingTime = nTimeStampDiff;
				m_b1stDecodedFrame = false;
			}
			else
			{
				minTimeGap = nTimeStampDiff - prevTimeStamp;
			}

			prevTimeStamp = nTimeStampDiff;
            
            CLogPrinter_WriteLog(CLogPrinter::INFO, INSTENT_TEST_LOG, "TheKing--> Rendering TimeDiff = " + m_Tools.LongLongtoStringConvert(m_Tools.CurrentTimestamp() - lRenderingTimeDiff));
            
			if (frameSize<1 || minTimeGap < 50)
				continue;
            
            lRenderingTimeDiff = m_Tools.CurrentTimestamp();
            
			toolsObject.SOSleep(5);

			m_pCommonElementsBucket->m_pEventNotifier->fireVideoEvent(m_FriendID, nFrameNumber, frameSize, m_RenderingFrame, videoHeight, videoWidth);
		}
	}

	bRenderingThreadClosed = true;

	CLogPrinter_WriteLog(CLogPrinter::INFO, THREAD_LOG ,"CVideoRenderingThread::RenderingThreadProcedure() stopped RenderingThreadProcedure method.");
}