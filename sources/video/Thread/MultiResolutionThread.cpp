
#include "MultiResolutionThread.h"
#include "Size.h"


#if defined(TARGET_OS_IPHONE) || defined(TARGET_IPHONE_SIMULATOR)
#include <dispatch/dispatch.h>
#endif


#define IFRAME_INTERVAL_MULT_SESSION 15
#define FPS_MULT_SESSION 30


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

		this->m_pVideoDecoder = new CVideoDecoder(m_pcCommonElementsBucket);


		for (int i = 0; i < iLen; ++i) {

			CVideoEncoder *pvideoEncoder = new CVideoEncoder(m_pcCommonElementsBucket, 200);
			pvideoEncoder->CreateVideoEncoder(targetHeight[i], targetWidth[i], FPS_MULT_SESSION, IFRAME_INTERVAL_MULT_SESSION, false, SERVICE_TYPE_LIVE_STREAM);
			m_pVideoEncoderVecotr.push_back(pvideoEncoder);
		}

		m_pVideoDecoder->CreateVideoDecoder();

	}

	MultiResolutionThread::~MultiResolutionThread()
	{

	}

	void MultiResolutionThread::StopMultiResolutionThread()
	{
		CLogPrinter_LOG( MULTI_RESOLUTION_LOG, "MultiResolutionThread::StopMultiResolutionThread() called");

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

		CLogPrinter_LOG( MULTI_RESOLUTION_LOG, "MultiResolutionThread::StopMultiResolutionThread() Rendering Thread STOPPPP");
	}

	void MultiResolutionThread::StartMultiResolutionThread()
	{
		CLogPrinter_LOG( MULTI_RESOLUTION_LOG, "MultiResolutionThread::StartMultiResolutionThread() called");


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

		CLogPrinter_LOG( MULTI_RESOLUTION_LOG, "MultiResolutionThread::StartMultiResolutionThread() Rendering Thread started");

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
		CLogPrinter_LOG( MULTI_RESOLUTION_LOG, "MultiResolutionThread::MultiResolutionThreadProcedure() started MultiResolutionThreadProcedure method");

		Tools toolsObject;
        toolsObject.SetThreadName("MultiResolutionThread");
        int encodedFrameSize;
        
		int videoHeight = 0, videoWidth = 0;
        int decodedFrameSize = 0;


		while (m_bMultiResolutionThreadRunning)
		{
			//CLogPrinter_LOG( MULTI_RESOLUTION_LOG ,"MultiResolutionThread::MultiResolutionThreadProcedure() RUNNING MultiResolutionThreadProcedure method");

			if (m_pcVideoFrameBuffer->GetQueueSize() == 0)
			{
				CLogPrinter_LOG( MULTI_RESOLUTION_LOG, "MultiResolutionThread::MultiResolutionThreadProcedure() NOTHING for Rendering method");

				toolsObject.SOSleep(10);
			}
			else
			{
				CLogPrinter_LOG( MULTI_RESOLUTION_LOG, "MultiResolutionThread::MultiResolutionThreadProcedure() GOT FRAME for Rendering method");


				encodedFrameSize = m_pcVideoFrameBuffer->DeQueue( m_ucaEncodedVideoFrame);

				CLogPrinter_LOG(MULTI_RESOLUTION_LOG, "Library MultiResolutionThread:: encoded frame size = %d", encodedFrameSize);

                decodedFrameSize = m_pVideoDecoder->DecodeVideoFrame(m_ucaEncodedVideoFrame, encodedFrameSize, m_ucaVideoFrame, videoHeight, videoWidth);

				CLogPrinter_LOG(MULTI_RESOLUTION_LOG, "Library MultiResolutionThread:: decodedFrameSize = %d, videoHeight = %d, videoWidth = %d", decodedFrameSize, videoHeight, videoWidth);

				if (decodedFrameSize > 5)
				{

					for(int i= 0; i < m_Len; i++)
					{
						int iNewFrameLength = this->m_pColorConverter->DownScaleYUV420_Dynamic_Version2(m_ucaVideoFrame, videoHeight, videoWidth, m_ucaNewVideoFrame, m_TargetHeight[i], m_TargetWidth[i]);

						m_iDataLength[i] = m_pVideoEncoderVecotr.at(i)->EncodeVideoFrame(m_ucaNewVideoFrame, iNewFrameLength, m_ucaMultEncodedVideoFrame[i], false);

						CLogPrinter_LOG(MULTI_RESOLUTION_LOG, "MultiResolutionThread ----------- dataLength = %d, targetHeight = %d, targetWidth = %d, m_Len = %d", m_iDataLength[i], m_TargetHeight[i], m_TargetWidth[i], m_Len);
					}


					m_pcCommonElementsBucket->m_pEventNotifier->fireMultVideoEvent(m_ucaMultEncodedVideoFrame, m_iDataLength, m_TargetHeight, m_TargetWidth, m_Len);
				}
				else{
					m_pcCommonElementsBucket->m_pEventNotifier->fireMultVideoEvent(m_ucaMultEncodedVideoFrame, m_iDataLength, m_TargetHeight, m_TargetWidth, 0);
				}

				toolsObject.SOSleep(1);

			}
		}

		m_bMultiResolutionThreadClosed = true;

		CLogPrinter_LOG( MULTI_RESOLUTION_LOG, "MultiResolutionThread::MultiResolutionThreadProcedure() stopped MultiResolutionThreadProcedure method.");
	}


} //namespace MediaSDK


