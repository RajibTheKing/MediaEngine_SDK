
#include "RenderingThread.h"
#include "CommonElementsBucket.h"

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

	CLogPrinter_Write(CLogPrinter::INFO, "CVideoCallSession::StartRenderingThread Rendering Thread started");

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
	CLogPrinter_Write(CLogPrinter::DEBUGS, "CVideoCallSession::RenderingThreadProcedure() Started EncodingThreadProcedure.");
	Tools toolsObject;
	int frameSize, nFrameNumber, intervalTime;
	unsigned int nTimeStampDiff;
	long long currentFrameTime, decodingTime, firstFrameEncodingTime;
	int videoHeight, videoWidth;
	long long currentTimeStamp;
	int prevTimeStamp = 0;
	int minTimeGap = 51;
	bool m_b1stDecodedFrame = true;
	long long m_ll1stDecodedFrameTimeStamp = 0;

	while (bRenderingThreadRunning)
	{
		//CLogPrinter_Write(CLogPrinter::INFO, "CVideoCallSession::RenderingThreadProcedure");

		if (m_RenderingBuffer->GetQueueSize() == 0)
			toolsObject.SOSleep(10);
		else
		{

			int timeDiffForQueue;

			frameSize = m_RenderingBuffer->DeQueue(nFrameNumber, nTimeStampDiff, m_RenderingFrame, videoHeight, videoWidth, timeDiffForQueue);
			CLogPrinter_WriteForQueueTime(CLogPrinter::DEBUGS, " m_RenderingBuffer " + toolsObject.IntegertoStringConvert(timeDiffForQueue));

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

			if (frameSize<1 && minTimeGap < 50)
				continue;

			toolsObject.SOSleep(5);

			m_pCommonElementsBucket->m_pEventNotifier->fireVideoEvent(m_FriendID, nFrameNumber, frameSize, m_RenderingFrame, videoHeight, videoWidth);
		}
	}

	bRenderingThreadClosed = true;

	CLogPrinter_Write(CLogPrinter::DEBUGS, "CVideoCallSession::RenderingThreadProcedure() Stopped EncodingThreadProcedure");
}