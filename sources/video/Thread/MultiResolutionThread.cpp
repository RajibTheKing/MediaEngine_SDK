
#include "MultiResolutionThread.h"


#if defined(TARGET_OS_IPHONE) || defined(TARGET_IPHONE_SIMULATOR)
#include <dispatch/dispatch.h>
#endif

#if __ANDROID__

#include <android/log.h>
#define LOG_TAG "LibraryLog"
#define LOGFF(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

#endif

namespace MediaSDK
{

	MultiResolutionThread::MultiResolutionThread(VideoFrameBuffer *pcVideoFrameBuffer, CCommonElementsBucket *pCommonElementsBucket, int *targetHeight, int *targetWidth, int iLen) :
		m_pcVideoFrameBuffer(pcVideoFrameBuffer)
	{

		m_TargetHeight = targetHeight;
		m_TargetWidth = targetWidth;
		m_Len = iLen;

		m_pcCommonElementsBucket =  pCommonElementsBucket;
		this->m_pColorConverter = new CColorConverter(352, 352, m_pcCommonElementsBucket, 200);
	}

	MultiResolutionThread::~MultiResolutionThread()
	{

	}

	void MultiResolutionThread::StopMultiResolutionThread()
	{
		CLogPrinter_WriteLog(CLogPrinter::INFO, THREAD_LOG, "MultiResolutionThread::StopMultiResolutionThread() called");

		//if (pInternalThread.get())
		{

			m_bMultiResolutionThreadRunning = false;

			while (!m_bMultiResolutionThreadClosed)
			{
				m_Tools.SOSleep(5);
			}
		}

		if (NULL != m_pColorConverter)
		{
			delete m_pColorConverter;

			m_pColorConverter = NULL;
		}

		if (NULL != m_pcCommonElementsBucket)
		{
			delete m_pcCommonElementsBucket;

			m_pcCommonElementsBucket = NULL;
		}

		//pInternalThread.reset();

		CLogPrinter_WriteLog(CLogPrinter::INFO, THREAD_LOG, "MultiResolutionThread::StopMultiResolutionThread() Rendering Thread STOPPPP");
	}

	void MultiResolutionThread::StartMultiResolutionThread()
	{
		CLogPrinter_WriteLog(CLogPrinter::INFO, THREAD_LOG, "MultiResolutionThread::StartMultiResolutionThread() called");


		m_bMultiResolutionThreadRunning = true;
		m_bMultiResolutionThreadClosed = false;

#if defined(TARGET_OS_IPHONE) || defined(TARGET_IPHONE_SIMULATOR)

		dispatch_queue_t MultiResolutionThreadQ = dispatch_queue_create("MultiResolutionThreadQ", DISPATCH_QUEUE_CONCURRENT);
		dispatch_async(MultiResolutionThreadQ, ^{
			this->MultiResolutionThreadProcedure();
		});

#else

		std::thread myThread(CreateMultiResolutionThread, this);
		myThread.detach();

#endif

		CLogPrinter_WriteLog(CLogPrinter::INFO, THREAD_LOG, "MultiResolutionThread::StartMultiResolutionThread() Rendering Thread started");

		return;
	}

	void *MultiResolutionThread::CreateMultiResolutionThread(void* pParam)
	{
		MultiResolutionThread *pThis = (MultiResolutionThread*)pParam;
		pThis->MultiResolutionThreadProcedure();

		return NULL;
	}


	void MultiResolutionThread::MultiResolutionThreadProcedure()
	{
		CLogPrinter_WriteLog(CLogPrinter::INFO, THREAD_LOG, "MultiResolutionThread::MultiResolutionThreadProcedure() started MultiResolutionThreadProcedure method");

		Tools toolsObject;
        toolsObject.SetThreadName("MultiResolutionThread");
        int frameSize;
        
		int videoHeight, videoWidth;


		while (m_bMultiResolutionThreadRunning)
		{
			//CLogPrinter_WriteLog(CLogPrinter::INFO, THREAD_LOG ,"MultiResolutionThread::MultiResolutionThreadProcedure() RUNNING MultiResolutionThreadProcedure method");

			if (m_pcVideoFrameBuffer->GetQueueSize() == 0)
			{
				CLogPrinter_WriteLog(CLogPrinter::INFO, THREAD_LOG, "MultiResolutionThread::MultiResolutionThreadProcedure() NOTHING for Rendering method");

				toolsObject.SOSleep(10);
			}
			else
			{
				CLogPrinter_WriteLog(CLogPrinter::INFO, THREAD_LOG, "MultiResolutionThread::MultiResolutionThreadProcedure() GOT FRAME for Rendering method");


				frameSize = m_pcVideoFrameBuffer->DeQueue( m_ucaVideoFrame, videoHeight, videoWidth);

				int iDataLength[m_Len];
                for(int i= 0; i < m_Len; i++)
				{
					iDataLength[i] = this->m_pColorConverter->DownScaleYUV420_Dynamic_Version2(m_ucaVideoFrame, videoHeight, videoWidth, m_ucaMultVideoFrame[i], m_TargetHeight[i], m_TargetWidth[i]);

					LOGFF("fahad ------->>  ----------- dataLength = %d, targetHeight = %d, targetWidth = %d, m_Len = %d", iDataLength[i], m_TargetHeight[i], m_TargetWidth[i], m_Len);
				}


				m_pcCommonElementsBucket->m_pEventNotifier->fireMultVideoEvent(m_ucaMultVideoFrame, iDataLength, m_TargetHeight, m_TargetWidth, m_Len);

				toolsObject.SOSleep(1);

			}
		}

		m_bMultiResolutionThreadClosed = true;

		CLogPrinter_WriteLog(CLogPrinter::INFO, THREAD_LOG, "MultiResolutionThread::MultiResolutionThreadProcedure() stopped MultiResolutionThreadProcedure method.");
	}


} //namespace MediaSDK


