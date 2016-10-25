
#include "RenderingThread.h"
#include "CommonElementsBucket.h"
#include "VideoCallSession.h"

#if defined(TARGET_OS_IPHONE) || defined(TARGET_IPHONE_SIMULATOR)
#include <dispatch/dispatch.h>
#endif

long long g_llFirstFrameReceiveTime;

CVideoRenderingThread::CVideoRenderingThread(LongLong friendID, CRenderingBuffer *renderingBuffer, CCommonElementsBucket *commonElementsBucket, CVideoCallSession *pVideoCallSession, bool bIsCheckCall) :

m_RenderingBuffer(renderingBuffer),
m_pCommonElementsBucket(commonElementsBucket),
m_FriendID(friendID),
m_lRenderCallTime(0),
m_bIsCheckCall(bIsCheckCall)

{
    m_llRenderFrameCounter = 0;
    m_pVideoCallSession = pVideoCallSession;
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
	long long prevFrameTimeStamp = 0;
	int currentTimeGap = 52;
	int prevTimeStamp = 0;
	int minTimeGap = 51;
	bool m_b1stDecodedFrame = true;
	long long m_ll1stDecodedFrameTimeStamp = 0;
    long long lRenderingTimeDiff = 0;
    long long llPrevTimeStamp = 0;
	CAverageCalculator *pRenderingFps = new CAverageCalculator();
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

            
			int timeDiffForQueue, orientation;

			frameSize = m_RenderingBuffer->DeQueue(nFrameNumber, nTimeStampDiff, m_RenderingFrame, videoHeight, videoWidth, timeDiffForQueue, orientation);
            
            
            
            m_llRenderFrameCounter++;
			if (m_bIsCheckCall == DEVICE_ABILITY_CHECK_MOOD && m_llRenderFrameCounter<FPS_MAXIMUM * 2)
            {
                //printf("Skipping for frame = %lld\n", m_llRenderFrameCounter);
                continue;
            }
            
            
			CLogPrinter_WriteLog(CLogPrinter::INFO, QUEUE_TIME_LOG ,"CVideoRenderingThread::RenderingThreadProcedure() m_RenderingBuffer " + toolsObject.IntegertoStringConvert(timeDiffForQueue));

			currentFrameTime = toolsObject.CurrentTimestamp();
            
            if(m_pVideoCallSession->GetCalculationStartTime() == 0)
            {
                m_pVideoCallSession->SetCalculationStartMechanism(true);

				CLogPrinter_WriteLog(CLogPrinter::INFO, CHECK_CAPABILITY_LOG, "CVideoRenderingThread::RenderingThreadProcedure() device check calculation stated");
            }
            

			if (m_b1stDecodedFrame)
			{
				firstFrameEncodingTime = nTimeStampDiff;
				m_b1stDecodedFrame = false;
			}
			else
			{
				minTimeGap = nTimeStampDiff - prevTimeStamp;
				currentTimeGap = currentFrameTime - prevFrameTimeStamp;
                m_RenderTimeCalculator.UpdateData(currentTimeGap);
                //CLogPrinter_WriteLog(CLogPrinter::INFO, INSTENT_TEST_LOG, "Library Rendering TimeDiff = " + m_Tools.DoubleToString(m_RenderTimeCalculator.GetAverage()));
                
			}





			CLogPrinter_WriteSpecific5(CLogPrinter::INFO, " minTimeGap " + toolsObject.IntegertoStringConvert(minTimeGap) + " currentTimeGap " + toolsObject.IntegertoStringConvert(currentTimeGap));

            /*
			if( (currentTimeGap < 50 && (currentTimeGap + 10) < minTimeGap))
			{
				CLogPrinter_WriteSpecific5(CLogPrinter::INFO, " minTimeGap break " + toolsObject.IntegertoStringConvert( minTimeGap) + " currentTimeGap "
																  + toolsObject.IntegertoStringConvert( currentTimeGap));
				continue;

			}*/

			prevFrameTimeStamp = currentFrameTime;
			prevTimeStamp = nTimeStampDiff;
            
            //CLogPrinter_WriteLog(CLogPrinter::INFO, INSTENT_TEST_LOG, "TheKing--> Rendering TimeDiff = " + m_Tools.LongLongtoStringConvert(m_Tools.CurrentTimestamp() - lRenderingTimeDiff));
            
            //CLogPrinter_WriteLog(CLogPrinter::INFO, INSTENT_TEST_LOG, "TheKing--> DiffWithFirstFrame From Camera = " + m_Tools.LongLongtoStringConvert(m_Tools.CurrentTimestamp() - g_llFirstFrameReceiveTime));
            
			//if (frameSize<1 || minTimeGap < 50)
			//	continue;
            
            lRenderingTimeDiff = m_Tools.CurrentTimestamp();
            

            //CalculateFPS();
            
            //if(m_pVideoCallSession->GetResolutionNegotiationStatus() == true)


			if (m_bIsCheckCall == LIVE_CALL_MOOD)
            {
				pRenderingFps->CalculateFPS("RenderingFPS--> ");
				long long nowCurrentTimeStampDiff = m_Tools.CurrentTimestamp() - llPrevTimeStamp;
				LOGE(" fahadRajib  time diff for rendering-->----------------------  %d", minTimeGap );
				if(nowCurrentTimeStampDiff < 30)
					toolsObject.SOSleep(30 - nowCurrentTimeStampDiff);
				else toolsObject.SOSleep(0);
				m_pCommonElementsBucket->m_pEventNotifier->fireVideoEvent(m_FriendID, nFrameNumber, frameSize, m_RenderingFrame, videoHeight, videoWidth, orientation);

				llPrevTimeStamp = m_Tools.CurrentTimestamp();
            }

		}
	}

	bRenderingThreadClosed = true;

	CLogPrinter_WriteLog(CLogPrinter::INFO, THREAD_LOG ,"CVideoRenderingThread::RenderingThreadProcedure() stopped RenderingThreadProcedure method.");
}

void CVideoRenderingThread::CalculateFPS()
{
    
    if(m_Tools.CurrentTimestamp() - m_lRenderCallTime >= 1000)
    {
    //    CLogPrinter_WriteLog(CLogPrinter::INFO, INSTENT_TEST_LOG, ">>>>>>>> FPS = (" + m_Tools.IntegertoStringConvert(m_nRenderFrameCount));
        m_nRenderFrameCount = 0;
        m_lRenderCallTime = m_Tools.CurrentTimestamp();
        
    }
    m_nRenderFrameCount++;
}



