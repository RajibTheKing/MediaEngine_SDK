
#include "VideoDecodingThreadOfChannel.h"
#include "VideoCallSession.h"
#include "Globals.h"
#include "CommonElementsBucket.h"
#include "LiveVideoDecodingQueue.h"

#if defined(TARGET_OS_IPHONE) || defined(TARGET_IPHONE_SIMULATOR)
#include <dispatch/dispatch.h>
#endif

namespace MediaSDK
{

	extern map<long long, long long>g_ArribalTime;

#define MINIMUM_DECODING_TIME_FOR_FORCE_FPS 35

	CVideoDecodingThreadOfChannel::CVideoDecodingThreadOfChannel(CEncodedFrameDepacketizer *encodedFrameDepacketizer, long long llFriendID, CCommonElementsBucket *pCommonElementBucket, CRenderingBuffer *renderingBuffer,
		LiveVideoDecodingQueue *pLiveVideoDecodingQueue, CVideoDecoder *videoDecoder, CColorConverter *colorConverter,
		CVideoCallSession* pVideoCallSession, bool bIsCheckCall, int nFPS) :

		m_pEncodedFrameDepacketizer(encodedFrameDepacketizer),
		m_RenderingBuffer(renderingBuffer),
		m_pVideoDecoder(videoDecoder),
		m_pColorConverter(colorConverter),
		m_pVideoCallSession(pVideoCallSession),
		m_FpsCounter(0),
		m_FPS_TimeDiff(0),
		m_Counter(0),
		m_bIsCheckCall(bIsCheckCall),
		m_nCallFPS(nFPS),
		m_bResetForPublisherCallerCallEnd(false),
		m_bResetForViewerCallerCallStartEnd(false),
		m_HasPreviousValues(false),
		m_llFriendID(llFriendID)

	{
#ifdef DESKTOP_C_SHARP 

		m_iMaxLen = MAX_FRAME_WIDTH * MAX_FRAME_HEIGHT * 3;

#else

		m_iMaxLen = MAX_FRAME_WIDTH * MAX_FRAME_HEIGHT * 3 / 2;

#endif

		m_pCalculatorDecodeTime = new CAverageCalculator();

		m_pCommonElementBucket = pCommonElementBucket;

		m_pLiveVideoDecodingQueue = pLiveVideoDecodingQueue;
		llQueuePrevTime = 0;
		m_pVideoEffect = new CVideoEffects();
		//m_iEffectSelection = 0;
		//m_iNumberOfEffect = 6;
		//m_iNumberOfEffectedFrame = 0;
	}

	CVideoDecodingThreadOfChannel::~CVideoDecodingThreadOfChannel()
	{
		if (NULL != m_pCalculatorDecodeTime)
		{
			delete m_pCalculatorDecodeTime;
			m_pCalculatorDecodeTime = NULL;
		}

		if (NULL != m_pVideoEffect)
		{
			delete m_pVideoEffect;
			m_pVideoEffect = NULL;
		}
	}

	void CVideoDecodingThreadOfChannel::SetCallFPS(int nFPS)
	{
		m_nCallFPS = nFPS;
	}

	void CVideoDecodingThreadOfChannel::InstructionToStop()
	{
		bDecodingThreadRunning = false;
	}

	void CVideoDecodingThreadOfChannel::StopDecodingThread()
	{
		CLogPrinter_WriteLog(CLogPrinter::INFO, THREAD_LOG, "CVideoDecodingThreadOfChannel::StopDecodingThread called");

		//if (pDepacketizationThread.get())
		{
			bDecodingThreadRunning = false;

			while (!bDecodingThreadClosed)
			{
				m_Tools.SOSleep(5);
			}
		}
		//pDepacketizationThread.reset();

		CLogPrinter_WriteLog(CLogPrinter::INFO, THREAD_LOG, "CVideoDecodingThreadOfChannel::StopDecodingThread Decoding Thread STOPPED");
	}
	void CVideoDecodingThreadOfChannel::Reset()
	{
		m_dbAverageDecodingTime = 0;
		m_dbTotalDecodingTime = 0;
		//int m_nOponnentFPS, m_nMaxProcessableByMine;
		m_iDecodedFrameCounter = 0;
		m_nMaxDecodingTime = 0;
		m_FpsCounter = 0;
	}
	void CVideoDecodingThreadOfChannel::StartDecodingThread()
	{
		CLogPrinter_WriteLog(CLogPrinter::INFO, THREAD_LOG, "CVideoDecodingThreadOfChannel::StartDecodingThread called");

		if (pDecodingThread.get())
		{
			pDecodingThread.reset();

			return;
		}

		bDecodingThreadRunning = true;
		bDecodingThreadClosed = false;

#if defined(TARGET_OS_IPHONE) || defined(TARGET_IPHONE_SIMULATOR)

		dispatch_queue_t PacketizationThreadQ = dispatch_queue_create("PacketizationThreadQ", DISPATCH_QUEUE_CONCURRENT);
		dispatch_async(PacketizationThreadQ, ^{
			this->DecodingThreadProcedure();
		});

#else

		std::thread myThread(CreateDecodingThread, this);
		myThread.detach();

#endif

		CLogPrinter_WriteLog(CLogPrinter::INFO, THREAD_LOG, "CVideoDecodingThreadOfChannel::StartDecodingThread Decoding Thread started");

		return;
	}

	void *CVideoDecodingThreadOfChannel::CreateDecodingThread(void* param)
	{
		CVideoDecodingThreadOfChannel *pThis = (CVideoDecodingThreadOfChannel*)param;
		pThis->DecodingThreadProcedure();

		return NULL;
	}

	void CVideoDecodingThreadOfChannel::ResetForPublisherCallerCallEnd()
	{
		m_bResetForPublisherCallerCallEnd = true;

		while (m_bResetForPublisherCallerCallEnd)
		{
			m_Tools.SOSleep(5);
		}
	}

	void CVideoDecodingThreadOfChannel::ResetForViewerCallerCallStartEnd()
	{
		m_bResetForViewerCallerCallStartEnd = true;

		while (m_bResetForViewerCallerCallStartEnd)
		{
			m_Tools.SOSleep(5);
		}
	}

	void CVideoDecodingThreadOfChannel::DecodingThreadProcedure()
	{
		CLogPrinter_WriteLog(CLogPrinter::INFO, THREAD_LOG, "CVideoDecodingThreadOfChannel::DecodingThreadProcedure() started DecodingThreadProcedure method");

		Tools toolsObject;

		int nFrameLength;
		unsigned int nTimeStampDiff = 0;
		long long currentTime;

		int nExpectedTime;

		int nDecodingStatus, fps = -1;

		nExpectedTime = -1;
		long long maxDecodingTime = 0, framCounter = 0;
		double decodingTimeAverage = 0;

		long long llFirstFrameTimeStamp = -1;
		int nFirstFrameNumber = -1;
		long long llTargetTimeStampDiff = -1;
		long long llExpectedTimeOffset = -1;

		long long llCountMiss = 0;

		long long llSlotTimeStamp = 0;

		CVideoHeader videoHeaderObject;

		while (bDecodingThreadRunning)
		{
			if (m_pVideoCallSession->isLiveVideoStreamRunning())
			{
				if (m_bResetForViewerCallerCallStartEnd == true)
				{
					m_pLiveVideoDecodingQueue->ResetBuffer();

					llFirstFrameTimeStamp = -1;
					llCountMiss = 0;
					m_HasPreviousValues = false;

					m_bResetForViewerCallerCallStartEnd = false;
				}

				if (m_pLiveVideoDecodingQueue->GetQueueSize() == 0)
				{
					CLogPrinter_WriteLog(CLogPrinter::INFO, THREAD_LOG, "CVideoDecodingThreadOfChannel::DecodingThreadProcedure() Got NOTHING for decoding");

					toolsObject.SOSleep(1);
				}
				else
				{
					long long diifTime;
					CLogPrinter_WriteLog(CLogPrinter::INFO, THREAD_LOG, "CVideoDecodingThreadOfChannel::DecodingThreadProcedure() Got packet for decoding");

					nFrameLength = m_pLiveVideoDecodingQueue->DeQueue(m_PacketizedFrame);
					//packetHeaderObject.setPacketHeader(m_PacketizedFrame);
					videoHeaderObject.setPacketHeader(m_PacketizedFrame);

					videoHeaderObject.ShowDetails("##RCV : ");

					//printf("#V## Queue: %d\n",nFrameLength);

					currentTime = m_Tools.CurrentTimestamp();

					if (-1 == llFirstFrameTimeStamp)
					{
						toolsObject.SOSleep(__LIVE_FIRST_FRAME_SLEEP_TIME__);
						currentTime = m_Tools.CurrentTimestamp();
						llFirstFrameTimeStamp = currentTime;
						//llExpectedTimeOffset = llFirstFrameTimeStamp - packetHeaderObject.getTimeStamp();
						llExpectedTimeOffset = llFirstFrameTimeStamp - videoHeaderObject.getTimeStamp();

						m_pVideoCallSession->SetOponentDeviceType(videoHeaderObject.getSenderDeviceType());

					}
					else
					{
						//diifTime = packetHeaderObject.getTimeStamp() - currentTime + llExpectedTimeOffset;
						//int iCurrentFrame = packetHeaderObject.getFrameNumber();

						diifTime = videoHeaderObject.getTimeStamp() - currentTime + llExpectedTimeOffset;
						int iCurrentFrame = videoHeaderObject.getFrameNumber();

						//CLogPrinter_WriteLog(CLogPrinter::INFO, INSTENT_TEST_LOG_2, "CVideoDecodingThreadOfChannel::DecodingThreadProcedure()************* FN: " + m_Tools.IntegertoStringConvert(iCurrentFrame) + " DIFT: " + m_Tools.LongLongToString(diifTime));

						//while(packetHeaderObject.getTimeStamp() > currentTime - llExpectedTimeOffset)

						if (videoHeaderObject.getTimeStamp() < (currentTime - llExpectedTimeOffset)){
							LOG_AAC("#aac#aqv# VideoFrameReceivedAfterTime: %lld", videoHeaderObject.getTimeStamp() - (currentTime - llExpectedTimeOffset));
						}

						while (videoHeaderObject.getTimeStamp() > currentTime - llExpectedTimeOffset)
						{
							toolsObject.SOSleep(1);
							currentTime = m_Tools.CurrentTimestamp();
						}
					}

					int nNumberOfInsets = videoHeaderObject.GetNumberOfInset();

					videoHeaderObject.GetInsetHeights(m_naInsetHeights, nNumberOfInsets);
					videoHeaderObject.GetInsetWidths(m_naInsetWidths, nNumberOfInsets);

					//nDecodingStatus = DecodeAndSendToClient(m_PacketizedFrame + PACKET_HEADER_LENGTH, nFrameLength - PACKET_HEADER_LENGTH,0,0,0);
					nDecodingStatus = DecodeAndSendToClient(m_PacketizedFrame + videoHeaderObject.GetHeaderLength(), nFrameLength - videoHeaderObject.GetHeaderLength(), 0, 0, 0, m_naInsetHeights[0], m_naInsetWidths[0]);

					llSlotTimeStamp = m_Tools.CurrentTimestamp();
					toolsObject.SOSleep(1);
				}
				continue;
			}
		}

		bDecodingThreadClosed = true;

		CLogPrinter_WriteLog(CLogPrinter::INFO, THREAD_LOG, "CVideoDecodingThreadOfChannel::DecodingThreadProcedure() stopped DecodingThreadProcedure method.");
	}

	int nIDR_Frame_GapOfChannel = -1;
	int CVideoDecodingThreadOfChannel::DecodeAndSendToClient(unsigned char *in_data, unsigned int frameSize, int nFramNumber, unsigned int nTimeStampDiff, int nOrientation, int nInsetHeight, int nInsetWidth)
	{
		long long currentTimeStamp = CLogPrinter_WriteLog(CLogPrinter::INFO, OPERATION_TIME_LOG);

		long long decTime = m_Tools.CurrentTimestamp();

		int nalType = m_Tools.GetEncodedFrameType(in_data);

		if (nalType == SPS_DATA)
		{
			printf("TheKing--> IDR FRAME Recieved, nFrameNumber = %d, IDR_FRAME_GAP = %d\n", nFramNumber, nFramNumber - nIDR_Frame_GapOfChannel);
			nIDR_Frame_GapOfChannel = nFramNumber;
		}

#if defined(TARGET_OS_WINDOWS_PHONE)

		m_decodedFrameSize = m_pVideoDecoder->DecodeVideoFrame(in_data, frameSize, m_TempDecodedFrame, m_decodingHeight, m_decodingWidth);

#else

		m_decodedFrameSize = m_pVideoDecoder->DecodeVideoFrame(in_data, frameSize, m_DecodedFrame, m_decodingHeight, m_decodingWidth);

#endif

		CLogPrinter_WriteFileLog(CLogPrinter::INFO, WRITE_TO_LOG_FILE, "CVideoDecodingThreadOfChannel::DecodeAndSendToClient() Decoded Frame m_decodedFrameSize " + m_Tools.getText(m_decodedFrameSize));

		CLogPrinter_WriteLog(CLogPrinter::INFO, THREAD_LOG, "CVideoDecodingThreadOfChannel::DecodeAndSendToClient() Decoded Frame m_decodedFrameSize " + m_Tools.getText(m_decodedFrameSize));

		//printf("#V### Decoded Size -> %d +++E.Size:  %d\n",m_decodedFrameSize,(int)frameSize);
		m_pCalculatorDecodeTime->UpdateData(m_Tools.CurrentTimestamp() - decTime);

		//if (m_decodingHeight > MAX_FRAME_HEIGHT || m_decodingWidth > MAX_FRAME_WIDTH)

		if (m_decodedFrameSize > m_iMaxLen)
		{
			m_pCommonElementBucket->m_pEventNotifier->fireVideoNotificationEvent(m_llFriendID, m_pCommonElementBucket->m_pEventNotifier->RESOLUTION_NOT_SUPPORTED);

			return -1;
		}

		// CLogPrinter_WriteLog(CLogPrinter::INFO, INSTENT_TEST_LOG, "TheKing--> DecodingTime  = " + m_Tools.LongLongtoStringConvert(m_Tools.CurrentTimestamp() - decTime) + ", CurrentCallFPS = " + m_Tools.IntegertoStringConvert(m_nCallFPS) + ", iVideoheight = " + m_Tools.IntegertoStringConvert(m_decodingHeight) + ", iVideoWidth = " + m_Tools.IntegertoStringConvert(m_decodingWidth) + ", AverageDecodeTime --> " + m_Tools.DoubleToString(m_pCalculatorDecodeTime->GetAverage()) + ", Decoder returned = " + m_Tools.IntegertoStringConvert(m_decodedFrameSize) + ", FrameNumber = " + m_Tools.IntegertoStringConvert(nFramNumber));

		CLogPrinter_WriteLog(CLogPrinter::INFO, OPERATION_TIME_LOG, " Decode ", currentTimeStamp);

		if (1 > m_decodedFrameSize)
			return -1;

#if defined(TARGET_OS_WINDOWS_PHONE)

		if(m_decodedFrameSize > MAX_VIDEO_DECODER_FRAME_SIZE )
			m_decodedFrameSize = this->m_pColorConverter->DownScaleYUV420_Dynamic(m_TempDecodedFrame, m_decodingHeight, m_decodingWidth, m_DecodedFrame, 2);
		else 
			memcpy(m_DecodedFrame, m_TempDecodedFrame, m_decodedFrameSize);

#endif

		m_pVideoCallSession->SetOpponentVideoHeightWidth(m_decodingHeight, m_decodingWidth);

		if (m_pVideoCallSession->GetServiceType() == SERVICE_TYPE_CHANNEL)
		{
			int iNewHeight = m_decodingHeight - m_decodingHeight % 4;
			int iNewWidth = m_decodingWidth - m_decodingWidth % 4;

			if (iNewWidth != m_decodingWidth || iNewHeight != m_decodingHeight)
			{
				int iNewLen = this->m_pColorConverter->Crop_YUV420(m_DecodedFrame, m_decodingHeight, m_decodingWidth, m_decodingWidth - iNewWidth, 0, m_decodingHeight - iNewHeight, 0, m_CropedFrame, iNewHeight, iNewWidth);

				memcpy(m_DecodedFrame, m_CropedFrame, iNewLen);

				m_decodedFrameSize = iNewLen;
				m_decodingHeight = iNewHeight;
				m_decodingWidth = iNewWidth;
			}

		}

		//QuickFix

		//printf("service type = %d, ownDeviceType = %d, OpponentDevice type = %d\n", m_pVideoCallSession->GetServiceType(), m_pVideoCallSession->GetOwnDeviceType(), m_pVideoCallSession->GetOponentDeviceType());


		currentTimeStamp = CLogPrinter_WriteLog(CLogPrinter::INFO, OPERATION_TIME_LOG, " ConvertI420ToNV21 ");
#if defined(TARGET_OS_IPHONE) || defined(TARGET_IPHONE_SIMULATOR)

		this->m_pColorConverter->ConvertI420ToNV12(m_DecodedFrame, m_decodingHeight, m_decodingWidth);

#elif defined(DESKTOP_C_SHARP)
		//	CLogPrinter_WriteSpecific(CLogPrinter::DEBUGS, "DepacketizationThreadProcedure() For Desktop");
		m_decodedFrameSize = this->m_pColorConverter->ConverterYUV420ToRGB24(m_DecodedFrame, m_RenderingRGBFrame, m_decodingHeight, m_decodingWidth);
#elif defined(TARGET_OS_WINDOWS_PHONE)
		this->m_pColorConverter->ConvertI420ToYV12(m_DecodedFrame, m_decodingHeight, m_decodingWidth);
#else

		this->m_pColorConverter->ConvertI420ToNV21(m_DecodedFrame, m_decodingHeight, m_decodingWidth);
#endif
		CLogPrinter_WriteLog(CLogPrinter::INFO, OPERATION_TIME_LOG, " ConvertI420ToNV21 ", currentTimeStamp);

		if (m_pVideoCallSession->GetCalculationStatus() == true && m_pVideoCallSession->GetResolationCheck() == false)
		{
			m_Counter++;
			long long currentTimeStampForBrust = m_Tools.CurrentTimestamp();
			long long diff = currentTimeStampForBrust - m_pVideoCallSession->GetCalculationStartTime();
			CLogPrinter_WriteLog(CLogPrinter::INFO, INSTENT_TEST_LOG || CHECK_CAPABILITY_LOG, "Inside m_Counter = " + m_Tools.IntegertoStringConvert(m_Counter)
				+ ", CalculationStartTime = " + m_Tools.LongLongtoStringConvert(m_pVideoCallSession->GetCalculationStartTime())
				+ ", CurrentTime = " + m_Tools.LongLongtoStringConvert(currentTimeStampForBrust) + ", m_nCallFPS = " + m_Tools.IntegertoStringConvert(m_nCallFPS) + ", diff = " + m_Tools.IntegertoStringConvert(diff));

			if (m_Counter >= (m_nCallFPS - FPS_TOLERANCE_FOR_HIGH_RESOLUTION) && diff <= 1000)
			{
				//   m_pCommonElementsBucket->m_pEventNotifier->fireVideoEvent(m_FriendID, nFrameNumber, frameSize, m_RenderingFrame, videoHeight, videoWidth);
				m_pVideoCallSession->SetCalculationStartMechanism(false);
				m_pVideoCallSession->DecideHighResolatedVideo(true);

				CLogPrinter_WriteLog(CLogPrinter::INFO, CHECK_CAPABILITY_LOG, " CVideoDecodingThreadOfChannel::DecodeAndSendToClient() SUCCESSED for iVideoheight = " + m_Tools.IntegertoStringConvert(m_decodingHeight) + ", iVideoWidth = " + m_Tools.IntegertoStringConvert(m_decodingWidth));

				//printFile("%s", sss.c_str());
				//printfiledone();

			}
			else if (diff > 1000)
			{
				CLogPrinter_WriteLog(CLogPrinter::INFO, CHECK_CAPABILITY_LOG, " CVideoDecodingThreadOfChannel::DecodeAndSendToClient() FAILED for iVideoheight = " + m_Tools.IntegertoStringConvert(m_decodingHeight) + ", iVideoWidth = " + m_Tools.IntegertoStringConvert(m_decodingWidth));

				m_pVideoCallSession->SetCalculationStartMechanism(false);
				m_pVideoCallSession->DecideHighResolatedVideo(false);
				//printFile("%s", sss.c_str());
				//printfiledone();
			}
		}




		if (m_FPS_TimeDiff == 0) m_FPS_TimeDiff = m_Tools.CurrentTimestamp();

		if (m_Tools.CurrentTimestamp() - m_FPS_TimeDiff < 1000)
		{
			m_FpsCounter++;
		}
		else
		{
			m_FPS_TimeDiff = m_Tools.CurrentTimestamp();

			//printf("Current Decoding FPS = %d\n", m_FpsCounter);
			if (m_FpsCounter >(m_nCallFPS - FPS_TOLERANCE_FOR_FPS))
			{
				//kaj korte hobe
			}

			//if(m_FpsCounter<FPS_MAXIMUM)
			//g_FPSController->SetMaxOwnProcessableFPS(m_FpsCounter);
			m_FpsCounter = 0;
		}





#if defined(DESKTOP_C_SHARP)
		m_RenderingBuffer->Queue(nFramNumber, m_RenderingRGBFrame, m_decodedFrameSize, nTimeStampDiff, m_decodingHeight, m_decodingWidth, nOrientation, nInsetHeight, nInsetWidth);
		return m_decodedFrameSize;
#else


		m_RenderingBuffer->Queue(nFramNumber, m_DecodedFrame, m_decodedFrameSize, nTimeStampDiff, m_decodingHeight, m_decodingWidth, nOrientation, 0, 0);
		return m_decodedFrameSize;
#endif
	}

} //namespace MediaSDK
