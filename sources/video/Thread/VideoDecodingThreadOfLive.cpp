
#include "VideoDecodingThreadOfLive.h"
#include "VideoCallSession.h"

#include "CommonElementsBucket.h"
#include "LiveVideoDecodingQueue.h"

#if defined(TARGET_OS_IPHONE) || defined(TARGET_IPHONE_SIMULATOR)
#include <dispatch/dispatch.h>
#endif

namespace MediaSDK
{

	extern map<long long, long long>g_ArribalTime;

#define MINIMUM_DECODING_TIME_FOR_FORCE_FPS 35

	CVideoDecodingThreadOfLive::CVideoDecodingThreadOfLive(CEncodedFrameDepacketizer *encodedFrameDepacketizer, long long llFriendID, CCommonElementsBucket *pCommonElementBucket, CRenderingBuffer *renderingBuffer,
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

	CVideoDecodingThreadOfLive::~CVideoDecodingThreadOfLive()
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

	void CVideoDecodingThreadOfLive::SetCallFPS(int nFPS)
	{
		m_nCallFPS = nFPS;
	}

	void CVideoDecodingThreadOfLive::InstructionToStop()
	{
		bDecodingThreadRunning = false;
	}

	void CVideoDecodingThreadOfLive::StopDecodingThread()
	{
		CLogPrinter_WriteLog(CLogPrinter::INFO, THREAD_LOG, "CVideoDecodingThreadOfLive::StopDecodingThread called");

		//if (pDepacketizationThread.get())
		{
			bDecodingThreadRunning = false;

			while (!bDecodingThreadClosed)
			{
				m_Tools.SOSleep(5);
			}
		}
		//pDepacketizationThread.reset();

		CLogPrinter_WriteLog(CLogPrinter::INFO, THREAD_LOG, "CVideoDecodingThreadOfLive::StopDecodingThread Decoding Thread STOPPED");
	}
	void CVideoDecodingThreadOfLive::Reset()
	{
		m_dbAverageDecodingTime = 0;
		m_dbTotalDecodingTime = 0;
		//int m_nOponnentFPS, m_nMaxProcessableByMine;
		m_iDecodedFrameCounter = 0;
		m_nMaxDecodingTime = 0;
		m_FpsCounter = 0;
	}
	void CVideoDecodingThreadOfLive::StartDecodingThread()
	{
		CLogPrinter_WriteLog(CLogPrinter::INFO, THREAD_LOG, "CVideoDecodingThreadOfLive::StartDecodingThread called");

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

		CLogPrinter_WriteLog(CLogPrinter::INFO, THREAD_LOG, "CVideoDecodingThreadOfLive::StartDecodingThread Decoding Thread started");

		return;
	}

	void *CVideoDecodingThreadOfLive::CreateDecodingThread(void* param)
	{
		CVideoDecodingThreadOfLive *pThis = (CVideoDecodingThreadOfLive*)param;
		pThis->DecodingThreadProcedure();

		return NULL;
	}

	void CVideoDecodingThreadOfLive::ResetForPublisherCallerCallEnd()
	{
		m_bResetForPublisherCallerCallEnd = true;

		while (m_bResetForPublisherCallerCallEnd)
		{
			m_Tools.SOSleep(5);
		}
	}

	void CVideoDecodingThreadOfLive::ResetForViewerCallerCallStartEnd()
	{
		m_bResetForViewerCallerCallStartEnd = true;

		while (m_bResetForViewerCallerCallStartEnd)
		{
			m_Tools.SOSleep(5);
		}
	}

	void CVideoDecodingThreadOfLive::DecodingThreadProcedure()
	{
		CLogPrinter_WriteLog(CLogPrinter::INFO, THREAD_LOG, "CVideoDecodingThreadOfLive::DecodingThreadProcedure() started DecodingThreadProcedure method");

		Tools toolsObject;
        toolsObject.SetThreadName("DecodingLive");

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
					CLogPrinter_WriteLog(CLogPrinter::INFO, THREAD_LOG, "CVideoDecodingThreadOfLive::DecodingThreadProcedure() Got NOTHING for decoding");

					llCountMiss++;

					if (m_pVideoCallSession->GetServiceType() == SERVICE_TYPE_LIVE_STREAM || m_pVideoCallSession->GetServiceType() == SERVICE_TYPE_SELF_STREAM || m_pVideoCallSession->GetServiceType() == SERVICE_TYPE_CHANNEL)
					{
						if (m_pVideoCallSession->GetEntityType() == ENTITY_TYPE_VIEWER_CALLEE && (m_Tools.CurrentTimestamp() - llSlotTimeStamp) >= 29 && m_HasPreviousValues == true)
						{
							nDecodingStatus = DecodeAndSendToClient2();

							llSlotTimeStamp = m_Tools.CurrentTimestamp();
						}
					}

					toolsObject.SOSleep(1);
				}
				else
				{
					long long diifTime;
					CLogPrinter_WriteLog(CLogPrinter::INFO, THREAD_LOG, "CVideoDecodingThreadOfLive::DecodingThreadProcedure() Got packet for decoding");

					long long llCurrentChunkRelativeTime;

					nFrameLength = m_pLiveVideoDecodingQueue->DeQueue(m_PacketizedFrame, llCurrentChunkRelativeTime);
					//packetHeaderObject.setPacketHeader(m_PacketizedFrame);
					videoHeaderObject.SetPacketHeader(m_PacketizedFrame);

					videoHeaderObject.ShowDetails("##RCV : ");

					//printf("#V## Queue: %d\n",nFrameLength);

					currentTime = m_Tools.CurrentTimestamp();

					if (-1 == llFirstFrameTimeStamp)
					{
						toolsObject.SOSleep(__LIVE_FIRST_FRAME_SLEEP_TIME__);
						currentTime = m_Tools.CurrentTimestamp();
						llFirstFrameTimeStamp = currentTime;
						//llExpectedTimeOffset = llFirstFrameTimeStamp - packetHeaderObject.getTimeStamp();
						llExpectedTimeOffset = llFirstFrameTimeStamp - videoHeaderObject.GetTimeStamp();

						m_pVideoCallSession->SetOponentDeviceType(videoHeaderObject.GetSenderDeviceType());

					}
					else
					{
						//diifTime = packetHeaderObject.getTimeStamp() - currentTime + llExpectedTimeOffset;
						//int iCurrentFrame = packetHeaderObject.getFrameNumber();

						diifTime = videoHeaderObject.GetTimeStamp() - currentTime + llExpectedTimeOffset;
						int iCurrentFrame = videoHeaderObject.GetFrameNumber();

						//CLogPrinter_WriteLog(CLogPrinter::INFO, INSTENT_TEST_LOG_2, "CVideoDecodingThreadOfLive::DecodingThreadProcedure()************* FN: " + m_Tools.IntegertoStringConvert(iCurrentFrame) + " DIFT: " + m_Tools.LongLongToString(diifTime));

						//while(packetHeaderObject.getTimeStamp() > currentTime - llExpectedTimeOffset)

						if (videoHeaderObject.GetTimeStamp() < (currentTime - llExpectedTimeOffset)){
							LOG_AAC("#aac#aqv# VideoFrameReceivedAfterTime: %lld", videoHeaderObject.getTimeStamp() - (currentTime - llExpectedTimeOffset));
						}

						while (videoHeaderObject.GetTimeStamp() > currentTime - llExpectedTimeOffset)
						{
							toolsObject.SOSleep(1);
							currentTime = m_Tools.CurrentTimestamp();

							if (m_pVideoCallSession->GetEntityType() == ENTITY_TYPE_VIEWER_CALLEE && (currentTime - llSlotTimeStamp) >= 29 && m_HasPreviousValues == true)
							{
								nDecodingStatus = DecodeAndSendToClient2();
								llSlotTimeStamp = m_Tools.CurrentTimestamp();
							}
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

		CLogPrinter_WriteLog(CLogPrinter::INFO, THREAD_LOG, "CVideoDecodingThreadOfLive::DecodingThreadProcedure() stopped DecodingThreadProcedure method.");
	}

	int CVideoDecodingThreadOfLive::DecodeAndSendToClient2()
	{
		long long currentTimeStamp = CLogPrinter_WriteLog(CLogPrinter::INFO, OPERATION_TIME_LOG);

		long long decTime = m_Tools.CurrentTimestamp();

		m_pCalculatorDecodeTime->UpdateData(m_Tools.CurrentTimestamp() - decTime);

		int iInsetLowerPadding = (int)((m_pVideoCallSession->GetOpponentVideoHeight() * 10) / 100);

		int iWidth = m_PreviousDecodingWidth;
		int iHeight = m_PreviousDecodingHeight;

		int iSmallWidth = m_pColorConverter->GetSmallFrameWidth();
		int iSmallHeight = m_pColorConverter->GetSmallFrameHeight();

		int iPosX = iWidth - iSmallWidth;
		int iPosY = iHeight - iSmallHeight - iInsetLowerPadding;

		memcpy(m_PreviousDecodedFrameConvertedData, m_PreviousDecodedFrame, m_previousDecodedFrameSize);

#if defined(TARGET_OS_IPHONE) || defined(TARGET_IPHONE_SIMULATOR)

		this->m_pColorConverter->ConvertNV12ToI420(m_PreviousDecodedFrameConvertedData, iHeight, iWidth);

#elif defined(__ANDROID__)

		this->m_pColorConverter->ConvertNV21ToI420(m_PreviousDecodedFrameConvertedData, iHeight, iWidth);

#elif defined(TARGET_OS_WINDOWS_PHONE)

		this->m_pColorConverter->ConvertYV12ToI420(m_PreviousDecodedFrameConvertedData, iHeight, iWidth);

#endif

		if (m_pVideoCallSession->GetOponentDeviceType() != DEVICE_TYPE_DESKTOP)
		{
			m_pColorConverter->GetInsetLocation(iHeight, iWidth, iPosX, iPosY);
		}

		this->m_pColorConverter->Merge_Two_Video(m_PreviousDecodedFrameConvertedData, iPosX, iPosY, m_PreviousDecodingHeight, m_PreviousDecodingWidth);

#if defined(TARGET_OS_IPHONE) || defined(TARGET_IPHONE_SIMULATOR)

		this->m_pColorConverter->ConvertI420ToNV12(m_PreviousDecodedFrameConvertedData, m_PreviousDecodingHeight, m_PreviousDecodingWidth);
#elif defined(DESKTOP_C_SHARP)
		m_previousDecodedFrameSize = this->m_pColorConverter->ConverterYUV420ToRGB24(m_PreviousDecodedFrameConvertedData, m_RenderingRGBFrame, m_PreviousDecodingHeight, m_PreviousDecodingWidth);
#elif defined(TARGET_OS_WINDOWS_PHONE)
		this->m_pColorConverter->ConvertI420ToYV12(m_PreviousDecodedFrameConvertedData, m_PreviousDecodingHeight, m_PreviousDecodingWidth);
#else

		this->m_pColorConverter->ConvertI420ToNV21(m_PreviousDecodedFrameConvertedData, m_PreviousDecodingHeight, m_PreviousDecodingWidth);
#endif

		CLogPrinter_WriteLog(CLogPrinter::INFO, OPERATION_TIME_LOG, " ConvertI420ToNV21 ", currentTimeStamp);


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

		m_RenderingBuffer->Queue(m_PreviousFrameNumber, m_RenderingRGBFrame, m_previousDecodedFrameSize, 0, m_PreviousDecodingHeight, m_PreviousDecodingWidth, m_PreviousOrientation, m_naInsetHeights[0], m_naInsetWidths[0]);

		return m_previousDecodedFrameSize;

#else

		m_RenderingBuffer->Queue(m_PreviousFrameNumber, m_PreviousDecodedFrameConvertedData, m_previousDecodedFrameSize, 0, m_PreviousDecodingHeight, m_PreviousDecodingWidth, m_PreviousOrientation, m_naInsetHeights[0], m_naInsetWidths[0]);

		return m_previousDecodedFrameSize;

#endif

	}

	int nIDR_Frame_GapOfLive = -1;
	int CVideoDecodingThreadOfLive::DecodeAndSendToClient(unsigned char *in_data, unsigned int frameSize, int nFramNumber, unsigned int nTimeStampDiff, int nOrientation, int nInsetHeight, int nInsetWidth)
	{
		long long currentTimeStamp = CLogPrinter_WriteLog(CLogPrinter::INFO, OPERATION_TIME_LOG);

		long long decTime = m_Tools.CurrentTimestamp();

		int nalType = m_Tools.GetEncodedFrameType(in_data);

		if (nalType == SPS_DATA)
		{
			printf("TheKing--> IDR FRAME Recieved, nFrameNumber = %d, IDR_FRAME_GAP = %d\n", nFramNumber, nFramNumber - nIDR_Frame_GapOfLive);
			nIDR_Frame_GapOfLive = nFramNumber;
		}

#if defined(TARGET_OS_WINDOWS_PHONE)

		m_decodedFrameSize = m_pVideoDecoder->DecodeVideoFrame(in_data, frameSize, m_TempDecodedFrame, m_decodingHeight, m_decodingWidth);

#else

		m_decodedFrameSize = m_pVideoDecoder->DecodeVideoFrame(in_data, frameSize, m_DecodedFrame, m_decodingHeight, m_decodingWidth);

#endif

		CLogPrinter_WriteFileLog(CLogPrinter::INFO, WRITE_TO_LOG_FILE, "CVideoDecodingThreadOfLive::DecodeAndSendToClient() Decoded Frame m_decodedFrameSize " + m_Tools.getText(m_decodedFrameSize));

		CLogPrinter_WriteLog(CLogPrinter::INFO, THREAD_LOG, "CVideoDecodingThreadOfLive::DecodeAndSendToClient() Decoded Frame m_decodedFrameSize " + m_Tools.getText(m_decodedFrameSize));

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

		if (m_pVideoCallSession->GetServiceType() == SERVICE_TYPE_LIVE_STREAM || m_pVideoCallSession->GetServiceType() == SERVICE_TYPE_SELF_STREAM)
		{
			if (m_pVideoCallSession->GetEntityType() == ENTITY_TYPE_PUBLISHER_CALLER)
			{
				CLogPrinter_WriteLog(CLogPrinter::INFO, INSTENT_TEST_LOG_2, "CVideoDecodingThreadOfLive::DecodeAndSendToClient() SetSmallFrame m_decodingHeight " + m_Tools.getText(m_decodingHeight) + " m_decodingWidth " + m_Tools.getText(m_decodingWidth));

				int iHeight = m_pVideoCallSession->getVideoCallHeight();
				int iWidth = m_pVideoCallSession->getVideoCallWidth();
				this->m_pColorConverter->SetSmallFrame(m_DecodedFrame, m_decodingHeight, m_decodingWidth, m_decodedFrameSize, iHeight, iWidth, m_pVideoCallSession->GetOwnDeviceType() != DEVICE_TYPE_DESKTOP, 0);
			}
			else if (m_pVideoCallSession->GetEntityType() == ENTITY_TYPE_VIEWER_CALLEE)
			{
				memcpy(m_PreviousDecodedFrame, m_DecodedFrame, m_decodedFrameSize);
				m_previousDecodedFrameSize = m_decodedFrameSize;
				m_PreviousDecodingHeight = m_decodingHeight;
				m_nPreviousInsetHeight = nInsetHeight;
				m_nPreviousInsetWidth = nInsetWidth;
				m_PreviousDecodingWidth = m_decodingWidth;
				m_PreviousFrameNumber = nFramNumber;
				m_PreviousOrientation = nOrientation;
				m_HasPreviousValues = true;

				int iInsetLowerPadding = (int)((m_pVideoCallSession->GetOpponentVideoHeight() * 10) / 100);

				int iWidth = m_decodingWidth;
				int iHeight = m_decodingHeight;

				int iSmallWidth = m_pColorConverter->GetSmallFrameWidth();
				int iSmallHeight = m_pColorConverter->GetSmallFrameHeight();

				int iPosX = iWidth - iSmallWidth;
				int iPosY = iHeight - iSmallHeight - iInsetLowerPadding;

				CLogPrinter_WriteLog(CLogPrinter::INFO, INSTENT_TEST_LOG_2, "CVideoDecodingThreadOfLive::DecodeAndSendToClient() Merge_Two_Video iHeight " + m_Tools.getText(iHeight) + " iWidth " + m_Tools.getText(iWidth));

				if (m_pVideoCallSession->GetOponentDeviceType() != DEVICE_TYPE_DESKTOP)
				{
					m_pColorConverter->GetInsetLocation(iHeight, iWidth, iPosX, iPosY);
				}

				this->m_pColorConverter->Merge_Two_Video(m_DecodedFrame, iPosX, iPosY, iHeight, iWidth);
			}
			//TheKing-->Here

		}

		//QuickFix

		//printf("service type = %d, ownDeviceType = %d, OpponentDevice type = %d\n", m_pVideoCallSession->GetServiceType(), m_pVideoCallSession->GetOwnDeviceType(), m_pVideoCallSession->GetOponentDeviceType());

		if ((m_pVideoCallSession->GetServiceType() == SERVICE_TYPE_LIVE_STREAM || m_pVideoCallSession->GetServiceType() == SERVICE_TYPE_SELF_STREAM) && m_pVideoCallSession->GetOwnDeviceType() == DEVICE_TYPE_DESKTOP && m_pVideoCallSession->GetOponentDeviceType() != DEVICE_TYPE_DESKTOP)
		{
			int iHeight = m_pVideoCallSession->getVideoCallHeight();
			int iWidth = m_pVideoCallSession->getVideoCallWidth();

			int iScreenHeight = this->m_pColorConverter->GetScreenHeight();
			int iScreenWidth = this->m_pColorConverter->GetScreenWidth();

			int iCropedHeight = 0;
			int iCropedWidth = 0;

			if (iScreenWidth == -1 || iScreenHeight == -1)
			{
				//Do Nothing
			}
			else
			{
				int iCroppedDataLen;
				this->m_pColorConverter->ConvertI420ToNV12(m_DecodedFrame, m_decodingHeight, m_decodingWidth);
				iCroppedDataLen = this->m_pColorConverter->CropWithAspectRatio_YUVNV12_YUVNV21_RGB24(m_DecodedFrame, m_decodingHeight, m_decodingWidth, iScreenHeight, iScreenWidth, m_CropedFrame, iCropedHeight, iCropedWidth, YUVNV12);

				this->m_pColorConverter->ConvertNV12ToI420(m_CropedFrame, iCropedHeight, iCropedWidth);


				memcpy(m_DecodedFrame, m_CropedFrame, iCroppedDataLen);
				memcpy(m_PreviousDecodedFrame, m_CropedFrame, iCroppedDataLen);

				m_decodingHeight = iCropedHeight;
				m_decodingWidth = iCropedWidth;
				m_decodedFrameSize = iCroppedDataLen;

				m_previousDecodedFrameSize = iCroppedDataLen;
				m_PreviousDecodingHeight = iCropedHeight;
				m_PreviousDecodingWidth = iCropedWidth;
			}
		}



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

		if ((m_pVideoCallSession->GetServiceType() == SERVICE_TYPE_LIVE_STREAM || m_pVideoCallSession->GetServiceType() == SERVICE_TYPE_SELF_STREAM) && (m_pVideoCallSession->GetOwnDeviceType() != DEVICE_TYPE_DESKTOP))
		{
			int iHeight = m_pVideoCallSession->getVideoCallHeight();
			int iWidth = m_pVideoCallSession->getVideoCallWidth();

			int iScreenHeight = this->m_pColorConverter->GetScreenHeight();
			int iScreenWidth = this->m_pColorConverter->GetScreenWidth();

			int iCropedHeight = 0;
			int iCropedWidth = 0;

			if (iScreenWidth == -1 || iScreenHeight == -1)
			{
				//Do Nothing
			}
			else
			{
				int iCroppedDataLen;

				//if(m_pVideoCallSession->GetOwnDeviceType() == DEVICE_TYPE_DESKTOP)
				//{
				//	iCroppedDataLen = this->m_pColorConverter->CropWithAspectRatio_YUVNV12_YUVNV21_RGB24(m_RenderingRGBFrame, m_decodingHeight, m_decodingWidth, iScreenHeight, iScreenWidth, m_CropedFrame, iCropedHeight, iCropedWidth, RGB24);
				//}

				int nColorFormatType = -1;
				int nOwnDeviceType = m_pVideoCallSession->GetOwnDeviceType();

				if (nOwnDeviceType == DEVICE_TYPE_IOS)
				{
					nColorFormatType = YUVNV12;
				}
				else if (nOwnDeviceType == DEVICE_TYPE_ANDROID)
				{
					nColorFormatType = YUVNV21;
				}
				else if (nOwnDeviceType == DEVICE_TYPE_WINDOWS_PHONE)
				{
					nColorFormatType = YUVYV12;
				}

				if (m_pVideoCallSession->GetOponentDeviceType() != DEVICE_TYPE_DESKTOP)
				{
					iCroppedDataLen = this->m_pColorConverter->CropWithAspectRatio_YUVNV12_YUVNV21_RGB24(m_DecodedFrame, m_decodingHeight, m_decodingWidth, iScreenHeight, iScreenWidth, m_CropedFrame, iCropedHeight, iCropedWidth, nColorFormatType);
					memcpy(m_DecodedFrame, m_CropedFrame, iCroppedDataLen);
					memcpy(m_PreviousDecodedFrame, m_CropedFrame, iCroppedDataLen);
					m_decodingHeight = iCropedHeight;
					m_decodingWidth = iCropedWidth;
					m_decodedFrameSize = iCroppedDataLen;
					m_previousDecodedFrameSize = iCroppedDataLen;
					m_PreviousDecodingHeight = iCropedHeight;
					m_PreviousDecodingWidth = iCropedWidth;
				}
				else
				{
					memcpy(m_PreviousDecodedFrame, m_DecodedFrame, m_decodedFrameSize);
					m_previousDecodedFrameSize = m_decodedFrameSize;
					m_PreviousDecodingHeight = m_decodingHeight;
					m_PreviousDecodingWidth = m_decodingWidth;
				}
			}
		}

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

				CLogPrinter_WriteLog(CLogPrinter::INFO, CHECK_CAPABILITY_LOG, " CVideoDecodingThreadOfLive::DecodeAndSendToClient() SUCCESSED for iVideoheight = " + m_Tools.IntegertoStringConvert(m_decodingHeight) + ", iVideoWidth = " + m_Tools.IntegertoStringConvert(m_decodingWidth));

				//printFile("%s", sss.c_str());
				//printfiledone();

			}
			else if (diff > 1000)
			{
				CLogPrinter_WriteLog(CLogPrinter::INFO, CHECK_CAPABILITY_LOG, " CVideoDecodingThreadOfLive::DecodeAndSendToClient() FAILED for iVideoheight = " + m_Tools.IntegertoStringConvert(m_decodingHeight) + ", iVideoWidth = " + m_Tools.IntegertoStringConvert(m_decodingWidth));

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


		m_RenderingBuffer->Queue(nFramNumber, m_DecodedFrame, m_decodedFrameSize, nTimeStampDiff, m_decodingHeight, m_decodingWidth, nOrientation, nInsetHeight, nInsetWidth);
		return m_decodedFrameSize;
#endif
	}

} //namespace MediaSDK
