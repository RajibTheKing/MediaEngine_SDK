
#include "RenderingThreadOfLive.h"
#include "CommonElementsBucket.h"
#include "VideoCallSession.h"

#if defined(TARGET_OS_IPHONE) || defined(TARGET_IPHONE_SIMULATOR)
#include <dispatch/dispatch.h>
#endif

namespace MediaSDK
{

	CRenderingThreadOfLive::CRenderingThreadOfLive(long long llFriendID, CRenderingBuffer *pcRenderingBuffer, CCommonElementsBucket *pcCommonElementsBucket, CVideoCallSession *pcVideoCallSession, bool bIsCheckCall) :

		m_pcRenderingBuffer(pcRenderingBuffer),
		m_pcCommonElementsBucket(pcCommonElementsBucket),
		m_llFriendID(llFriendID),
		m_lRenderCallTime(0),
		m_bIsCheckCall(bIsCheckCall),
		m_nInsetHeight(-1),
		m_nInsetWidth(-1)

	{
		m_llRenderFrameCounter = 0;
		m_pcVideoCallSession = pcVideoCallSession;
	}

	CRenderingThreadOfLive::~CRenderingThreadOfLive()
	{

	}

	void CRenderingThreadOfLive::StopRenderingThread()
	{
		CLogPrinter_WriteLog(CLogPrinter::INFO, THREAD_LOG, "CRenderingThreadOfLive::StopRenderingThread() called");

		//if (pInternalThread.get())
		{

			m_bRenderingThreadRunning = false;

			while (!m_bRenderingThreadClosed)
			{
				m_Tools.SOSleep(5);
			}
		}

		//pInternalThread.reset();

		CLogPrinter_WriteLog(CLogPrinter::INFO, THREAD_LOG, "CRenderingThreadOfLive::StopRenderingThread() Rendering Thread STOPPPP");
	}

	void CRenderingThreadOfLive::StartRenderingThread()
	{
		CLogPrinter_WriteLog(CLogPrinter::INFO, THREAD_LOG, "CRenderingThreadOfLive::StartRenderingThread() called");

		if (pRenderingThread.get())
		{
			pRenderingThread.reset();
			return;
		}

		m_bRenderingThreadRunning = true;
		m_bRenderingThreadClosed = false;

#if defined(TARGET_OS_IPHONE) || defined(TARGET_IPHONE_SIMULATOR)

		dispatch_queue_t RenderThreadQ = dispatch_queue_create("RenderThreadQ", DISPATCH_QUEUE_CONCURRENT);
		dispatch_async(RenderThreadQ, ^{
			this->RenderingThreadProcedure();
		});

#else

		std::thread myThread(CreateVideoRenderingThread, this);
		myThread.detach();

#endif

		CLogPrinter_WriteLog(CLogPrinter::INFO, THREAD_LOG, "CRenderingThreadOfLive::StartRenderingThread() Rendering Thread started");

		return;
	}

	void *CRenderingThreadOfLive::CreateVideoRenderingThread(void* pParam)
	{
		CRenderingThreadOfLive *pThis = (CRenderingThreadOfLive*)pParam;
		pThis->RenderingThreadProcedure();

		return NULL;
	}


	void CRenderingThreadOfLive::RenderingThreadProcedure()
	{
		CLogPrinter_WriteLog(CLogPrinter::INFO, THREAD_LOG, "CRenderingThreadOfLive::RenderingThreadProcedure() started RenderingThreadProcedure method");

		Tools toolsObject;
        toolsObject.SetThreadName("RenderingLive");
        int frameSize;
        long long nFrameNumber;
		long long nTimeStampDiff;
		long long currentFrameTime, firstFrameEncodingTime;
		int videoHeight, videoWidth;
		long long prevFrameTimeStamp = 0;
		long long currentTimeGap = 52;
		long long prevTimeStamp = 0;
		long long minTimeGap = 51;
		bool m_b1stDecodedFrame = true;
		long long lRenderingTimeDiff = 0;

		while (m_bRenderingThreadRunning)
		{
			//CLogPrinter_WriteLog(CLogPrinter::INFO, THREAD_LOG ,"CRenderingThreadOfLive::RenderingThreadProcedure() RUNNING RenderingThreadProcedure method");

			if (m_pcRenderingBuffer->GetQueueSize() == 0)
			{
				CLogPrinter_WriteLog(CLogPrinter::INFO, THREAD_LOG, "CRenderingThreadOfLive::RenderingThreadProcedure() NOTHING for Rendering method");

				toolsObject.SOSleep(10);
			}
			else
			{
				CLogPrinter_WriteLog(CLogPrinter::INFO, THREAD_LOG, "CRenderingThreadOfLive::RenderingThreadProcedure() GOT FRAME for Rendering method");

				int timeDiffForQueue, orientation;
				int insetHeight, insetWidth;

				frameSize = m_pcRenderingBuffer->DeQueue(nFrameNumber, nTimeStampDiff, m_ucaRenderingFrame, videoHeight, videoWidth, timeDiffForQueue, orientation, insetHeight, insetWidth);

				m_llRenderFrameCounter++;

				if (m_bIsCheckCall == DEVICE_ABILITY_CHECK_MOOD && m_llRenderFrameCounter < FPS_MAXIMUM * 2)
				{
					//printf("Skipping for frame = %lld\n", m_llRenderFrameCounter);
					continue;
				}

				CLogPrinter_WriteLog(CLogPrinter::INFO, QUEUE_TIME_LOG, "CRenderingThreadOfLive::RenderingThreadProcedure() m_pcRenderingBuffer " + toolsObject.IntegertoStringConvert(timeDiffForQueue));

				currentFrameTime = toolsObject.CurrentTimestamp();

				if (m_pcVideoCallSession->GetCalculationStartTime() == 0)
				{
					m_pcVideoCallSession->SetCalculationStartMechanism(true);

					CLogPrinter_WriteLog(CLogPrinter::INFO, CHECK_CAPABILITY_LOG, "CRenderingThreadOfLive::RenderingThreadProcedure() device check calculation stated");
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
					m_cRenderTimeCalculator.UpdateData(currentTimeGap);
					//CLogPrinter_WriteLog(CLogPrinter::INFO, INSTENT_TEST_LOG, "Library Rendering TimeDiff = " + m_Tools.DoubleToString(m_cRenderTimeCalculator.GetAverage()));      
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

				//if(m_pcVideoCallSession->GetResolutionNegotiationStatus() == true)

				if (m_bIsCheckCall == LIVE_CALL_MOOD)
				{
					{
						//                    pRenderingFps->CalculateFPS("RenderingFPS--> ");
						//                    long long nowCurrentTimeStampDiff = m_Tools.CurrentTimestamp() - llPrevTimeStamp;
						//                    //LOGE(" fahadRajib  time diff for rendering-->----------------------  %d", minTimeGap );
						//                    if(nowCurrentTimeStampDiff < 30)
						//                        toolsObject.SOSleep(30 - nowCurrentTimeStampDiff);
						//                    else
						//                        toolsObject.SOSleep(0);
						//
						toolsObject.SOSleep(1);

						if (m_pcVideoCallSession->GetEntityType() != ENTITY_TYPE_PUBLISHER_CALLER)
						{

							m_pcCommonElementsBucket->m_pEventNotifier->fireVideoEvent(m_llFriendID, SERVICE_TYPE_LIVE_STREAM, nFrameNumber, frameSize, m_ucaRenderingFrame, videoHeight, videoWidth, insetHeight, insetWidth, orientation);
						}

						//                    llPrevTimeStamp = m_Tools.CurrentTimestamp();
					}
				}
			}
		}

		m_bRenderingThreadClosed = true;

		CLogPrinter_WriteLog(CLogPrinter::INFO, THREAD_LOG, "CRenderingThreadOfLive::RenderingThreadProcedure() stopped RenderingThreadProcedure method.");
	}

	void CRenderingThreadOfLive::CalculateFPS()
	{

		if (m_Tools.CurrentTimestamp() - m_lRenderCallTime >= 1000)
		{
			//    CLogPrinter_WriteLog(CLogPrinter::INFO, INSTENT_TEST_LOG, ">>>>>>>> FPS = (" + m_Tools.IntegertoStringConvert(m_nRenderFrameCount));
			m_nRenderFrameCount = 0;
			m_lRenderCallTime = m_Tools.CurrentTimestamp();

		}
		m_nRenderFrameCount++;
	}

} //namespace MediaSDK


