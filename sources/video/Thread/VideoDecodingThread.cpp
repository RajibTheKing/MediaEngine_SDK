
#include "VideoDecodingThread.h"
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

	CVideoDecodingThread::CVideoDecodingThread(CEncodedFrameDepacketizer *encodedFrameDepacketizer, long long llFriendID, CCommonElementsBucket *pCommonElementBucket, CRenderingBuffer *renderingBuffer,
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
		m_VideoBeautificationer(NULL),
		m_llFriendID(llFriendID),
		m_nPreviousDecodingHeight(320),
		m_nPreviousDecodingWidth(180),
		m_nUpperOffsetOfInset(SPLIT_TYPE_DEVIDE_UPPER_OFFSET)

	{
#ifdef DESKTOP_C_SHARP 

		m_iMaxLen = MAX_FRAME_WIDTH * MAX_FRAME_HEIGHT * 3;

#else

		m_iMaxLen = MAX_FRAME_WIDTH * MAX_FRAME_HEIGHT * 3 / 2;

#endif

		m_DecodeFailCounter = 0;
		m_NoFrameCounter = 0;

		m_pCalculatorDecodeTime = new CAverageCalculator();

		m_pCommonElementBucket = pCommonElementBucket;

		m_pLiveVideoDecodingQueue = pLiveVideoDecodingQueue;
		llQueuePrevTime = 0;
		m_pVideoEffect = new CVideoEffects();
		//m_iEffectSelection = 0;
		//m_iNumberOfEffect = 6;
		//m_iNumberOfEffectedFrame = 0;

		//pFile = fopen ("/sdcard/myfile.h264","w");
	}

	CVideoDecodingThread::~CVideoDecodingThread()
	{
		if (NULL != m_pCalculatorDecodeTime)
		{
			delete m_pCalculatorDecodeTime;
			m_pCalculatorDecodeTime = NULL;
		}

		/*if (NULL != m_VideoBeautificationer)
		{
			delete m_VideoBeautificationer;
			m_VideoBeautificationer = NULL;
		}*/

		if (NULL != m_pVideoEffect)
		{
			delete m_pVideoEffect;
			m_pVideoEffect = NULL;
		}

		//fclose(pFile);
	}

	void CVideoDecodingThread::SetCallFPS(int nFPS)
	{
		m_nCallFPS = nFPS;
	}

	void CVideoDecodingThread::InstructionToStop()
	{
		bDecodingThreadRunning = false;
	}

	void CVideoDecodingThread::StopDecodingThread()
	{
		CLogPrinter_WriteLog(CLogPrinter::INFO, THREAD_LOG, "CVideoDecodingThread::StopDecodingThread called");

		//if (pDepacketizationThread.get())
		{
			bDecodingThreadRunning = false;

			while (!bDecodingThreadClosed)
			{
				m_Tools.SOSleep(5);
			}
		}
		//pDepacketizationThread.reset();

		CLogPrinter_WriteLog(CLogPrinter::INFO, THREAD_LOG, "CVideoDecodingThread::StopDecodingThread Decoding Thread STOPPED");
	}
	void CVideoDecodingThread::Reset()
	{
		m_dbAverageDecodingTime = 0;
		m_dbTotalDecodingTime = 0;
		//int m_nOponnentFPS, m_nMaxProcessableByMine;
		m_iDecodedFrameCounter = 0;
		m_nMaxDecodingTime = 0;
		m_FpsCounter = 0;
	}
	void CVideoDecodingThread::StartDecodingThread()
	{
		CLogPrinter_WriteLog(CLogPrinter::INFO, THREAD_LOG, "CVideoDecodingThread::StartDecodingThread called");

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

		CLogPrinter_WriteLog(CLogPrinter::INFO, THREAD_LOG, "CVideoDecodingThread::StartDecodingThread Decoding Thread started");

		return;
	}

	void *CVideoDecodingThread::CreateDecodingThread(void* param)
	{
		CVideoDecodingThread *pThis = (CVideoDecodingThread*)param;
		pThis->DecodingThreadProcedure();

		return NULL;
	}

	void CVideoDecodingThread::ResetForPublisherCallerCallEnd()
	{
		m_bResetForPublisherCallerCallEnd = true;

		while (m_bResetForPublisherCallerCallEnd)
		{
			m_Tools.SOSleep(5);
		}
	}

	void CVideoDecodingThread::ResetForViewerCallerCallStartEnd()
	{
		m_bResetForViewerCallerCallStartEnd = true;

		while (m_bResetForViewerCallerCallStartEnd)
		{
			m_Tools.SOSleep(5);
		}
	}

	void CVideoDecodingThread::DecodingThreadProcedure()
	{
		CLogPrinter_WriteLog(CLogPrinter::INFO, THREAD_LOG, "CVideoDecodingThread::DecodingThreadProcedure() started DecodingThreadProcedure method");

		Tools toolsObject;
        toolsObject.SetThreadName("DecodingCommon");
        
        long long nFrameNumber, nEncodingTime;
		int nFrameLength, nOrientation;
		
		long long currentTime;

		long long nExpectedTime;

		int nDecodingStatus, fps = -1;

		int nOponnentFPS, nMaxProcessableByMine;
		nExpectedTime = -1;
		long long  decodingTime, nBeforeDecodingTime;

		long long llFirstFrameTimeStamp = -1;
		long long nFirstFrameNumber = -1;
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
					CLogPrinter_WriteLog(CLogPrinter::INFO, THREAD_LOG, "CVideoDecodingThread::DecodingThreadProcedure() Got NOTHING for decoding");

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
					CLogPrinter_WriteLog(CLogPrinter::INFO, THREAD_LOG, "CVideoDecodingThread::DecodingThreadProcedure() Got packet for decoding");

					long long llCurrentChunkRelativeTime;

					nFrameLength = m_pLiveVideoDecodingQueue->DeQueue(m_PacketizedFrame, llCurrentChunkRelativeTime);
					//packetHeaderObject.setPacketHeader(m_PacketizedFrame);
					videoHeaderObject.SetPacketHeader(m_PacketizedFrame);

					videoHeaderObject.ShowDetails("##RCV : ");

					printf("#V## DeviceOrientation from Header: %d\n",videoHeaderObject.GetDeviceOrientation());

					m_nUpperOffsetOfInset = videoHeaderObject.GetInsetUppperOffset();

					currentTime = m_Tools.CurrentTimestamp();

					if (-1 == llFirstFrameTimeStamp)
					{
						toolsObject.SOSleep(__LIVE_FIRST_FRAME_SLEEP_TIME__);
						currentTime = m_Tools.CurrentTimestamp();
						llFirstFrameTimeStamp = currentTime;
						//llExpectedTimeOffset = llFirstFrameTimeStamp - packetHeaderObject.getTimeStamp();

						if (m_pVideoCallSession->GetServiceType() == SERVICE_TYPE_CHANNEL)
						{
							llExpectedTimeOffset = llFirstFrameTimeStamp - llCurrentChunkRelativeTime;		// this will be for live also, it is much more appropiate 
						}
						else
						{
							llExpectedTimeOffset = llFirstFrameTimeStamp - videoHeaderObject.GetTimeStamp();
						}

						m_pVideoCallSession->SetOponentDeviceType(videoHeaderObject.GetSenderDeviceType());

					}
					else
					{
						//diifTime = packetHeaderObject.getTimeStamp() - currentTime + llExpectedTimeOffset;

						diifTime = videoHeaderObject.GetTimeStamp() - currentTime + llExpectedTimeOffset;

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
                    
                    m_pVideoCallSession->setOpponentVideoLibraryVersion(videoHeaderObject.GetLibraryVersion());
                    
					nDecodingStatus = DecodeAndSendToClient(m_PacketizedFrame + videoHeaderObject.GetHeaderLength(), nFrameLength - videoHeaderObject.GetHeaderLength(), 0, 0, videoHeaderObject.GetDeviceOrientation(), m_naInsetHeights[0], m_naInsetWidths[0], videoHeaderObject.GetLibraryVersion());

					llSlotTimeStamp = m_Tools.CurrentTimestamp();
					toolsObject.SOSleep(1);
				}
				continue;
			}

			if (m_bResetForPublisherCallerCallEnd == true)
			{
				m_pEncodedFrameDepacketizer->ResetEncodedFrameDepacketizer();

				m_bResetForPublisherCallerCallEnd = false;
			}

			if (-1 == m_pVideoCallSession->GetShiftedTime())
			{
				CLogPrinter_WriteLog(CLogPrinter::INFO, THREAD_LOG, "CVideoDecodingThread::DecodingThreadProcedure() Not started transmission");
				toolsObject.SOSleep(10);
				continue;
			}
			currentTime = toolsObject.CurrentTimestamp();
			nExpectedTime = currentTime - m_pVideoCallSession->GetShiftedTime();

			nFrameLength = m_pEncodedFrameDepacketizer->GetReceivedFrame(m_PacketizedFrame, nFrameNumber, nEncodingTime, nExpectedTime, 0, nOrientation);

			if (m_bIsCheckCall == true && m_pVideoCallSession->GetResolationCheck() == false)
			{
				if (llFirstFrameTimeStamp != -1 && (m_Tools.CurrentTimestamp() - llFirstFrameTimeStamp) > llTargetTimeStampDiff)
				{
					printf("Force Device Fire NOOOTTTT SSSUUUPPPOOORRTTEEDDD\n");
					m_pVideoCallSession->SetCalculationStartMechanism(false);
					m_pVideoCallSession->DecideHighResolatedVideo(false);
				}
			}

			if (nFrameLength > -1)
			{
				CLogPrinter_WriteLog(CLogPrinter::DEBUGS, DEPACKETIZATION_LOG, "#$Dec# FN: " +
					m_Tools.getText(
					nFrameNumber) + "  Len: " +
					m_Tools.getText(
					nFrameLength) +
					"  E.Time: " +
					m_Tools.getText(
					nEncodingTime)
					+ "  Exp E.Time: " +
					m_Tools.getText(
					nExpectedTime) + " -> " +
					m_Tools.getText(
					nExpectedTime -
					nEncodingTime) + "Orientation = " +
					m_Tools.getText(nOrientation));
				CLogPrinter_WriteLog(CLogPrinter::DEBUGS, DEPACKETIZATION_LOG, "#$ Cur: " + m_Tools.LongLongToString(currentTime) + " diff: " + m_Tools.LongLongToString(currentTime - g_ArribalTime[nFrameNumber]));


			}


			if (-1 == nFrameLength)
			{
				CLogPrinter_WriteLog(CLogPrinter::INFO, THREAD_LOG, "CVideoDecodingThread::DecodingThreadProcedure() NOTHING for decoding method");

				toolsObject.SOSleep(10);
			}
			else
			{
				CLogPrinter_WriteLog(CLogPrinter::INFO, THREAD_LOG, "CVideoDecodingThread::DecodingThreadProcedure() GOT FRAME FOR DDDDDDDDDDDDDdecoding method");

				nBeforeDecodingTime = toolsObject.CurrentTimestamp();
				//printf("Decoding end--> fn = %d\n", nFrameNumber);

				if (llFirstFrameTimeStamp == -1)
				{
					llFirstFrameTimeStamp = nBeforeDecodingTime;
					nFirstFrameNumber = nFrameNumber;

					llTargetTimeStampDiff = (FPS_MAXIMUM * 5 - nFirstFrameNumber) * (1000 / FPS_MAXIMUM);
					//printf("llFirstFrameTimeStamp = %lld, nFirstFrameNumber = %d, llTargetTimeStampDiff = %lld\n",llFirstFrameTimeStamp, nFirstFrameNumber, llTargetTimeStampDiff);

				}


				nOponnentFPS = m_pVideoCallSession->GetFPSController()->GetOpponentFPS();
				nMaxProcessableByMine = m_pVideoCallSession->GetFPSController()->GetMaxOwnProcessableFPS();

				/*if (nOponnentFPS > 1 + nMaxProcessableByMine && (nFrameNumber & 7) > 3) {
					//printf("TheKing-->nMaxProcessableByMine inside ifffffff, nFrameNumber = %d\n", nFrameNumber);

					CLogPrinter_WriteSpecific(CLogPrinter::DEBUGS, "CVideoDecodingThread::DecodingThreadProcedure() Force:: Frame: " + m_Tools.IntegertoStringConvert(nFrameNumber) + "  FPS: " + m_Tools.IntegertoStringConvert(nOponnentFPS) + " ~" + toolsObject.IntegertoStringConvert(nMaxProcessableByMine));
					toolsObject.SOSleep(5);
					continue;
					}*/

				//printf("TheKing-->nMaxProcessableByMine success, nFrameNumber = %d\n", nFrameNumber);



				/*
				if(nFrameNumber<200)
				{
				string str = "/Decode/" + m_Tools.IntegertoStringConvert(nFrameNumber) + "_" + m_Tools.IntegertoStringConvert(nFrameLength);
				str+=".dump";
				[[Helper_IOS GetInstance] WriteToFile:str.c_str() withData:m_PacketizedFrame dataLength:nFrameLength];
				}
				*/


				nDecodingStatus = DecodeAndSendToClient(m_PacketizedFrame, nFrameLength, nFrameNumber, nEncodingTime, nOrientation, 0, 0, 0);
				//printf("decode:  %d, nDecodingStatus %d\n", nFrameNumber, nDecodingStatus);
				//			toolsObject.SOSleep(100);



				if (nDecodingStatus > 0)
				{

					decodingTime = toolsObject.CurrentTimestamp() - nBeforeDecodingTime;
					m_dbTotalDecodingTime += decodingTime;
					++m_iDecodedFrameCounter;

					if (m_nMaxDecodingTime < decodingTime)
						//printf("Increasing   nMaxDecodingTime to %lld\n", m_nMaxDecodingTime);
						m_nMaxDecodingTime = max(m_nMaxDecodingTime, decodingTime);

					if (0 == (m_iDecodedFrameCounter & 3))
					{
						m_dbAverageDecodingTime = m_dbTotalDecodingTime / m_iDecodedFrameCounter;
						m_dbAverageDecodingTime *= 1.5;
						//printf("Average Decoding time = %lf, fps = %d\n", m_dbAverageDecodingTime, fps);
						if (m_dbAverageDecodingTime > MINIMUM_DECODING_TIME_FOR_FORCE_FPS)
						{
							fps = 1000 / m_dbAverageDecodingTime;
							//printf("WinD--> Error Case Average Decoding time = %lf, fps = %d\n", m_dbAverageDecodingTime, fps);
							if (fps < m_nCallFPS)
								m_pVideoCallSession->GetFPSController()->SetMaxOwnProcessableFPS(fps);
						}
					}
					CLogPrinter_WriteSpecific(CLogPrinter::DEBUGS, "CVideoDecodingThread::DecodingThreadProcedure() Force:: AVG Decoding Time:" + m_Tools.DoubleToString(dbAverageDecodingTime) + "  Max Decoding-time: " + m_Tools.IntegertoStringConvert(nMaxDecodingTime) + "  MaxOwnProcessable: " + m_Tools.IntegertoStringConvert(fps));
				}




				toolsObject.SOSleep(1);
			}
		}

		bDecodingThreadClosed = true;

		CLogPrinter_WriteLog(CLogPrinter::INFO, THREAD_LOG, "CVideoDecodingThread::DecodingThreadProcedure() stopped DecodingThreadProcedure method.");
	}

	int CVideoDecodingThread::DecodeAndSendToClient2()
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

		if (m_pVideoCallSession->GetScreenSplitType() == LIVE_CALL_SCREEN_SPLIT_TYPE_DIVIDE)
		{
			int upperOffset;
			
			if (m_nUpperOffsetOfInset > 4 && m_nUpperOffsetOfInset < 320)
				upperOffset = m_nUpperOffsetOfInset;
			else
				upperOffset = SPLIT_TYPE_DEVIDE_UPPER_OFFSET;

			CLogPrinter_LOG(CO_HOST_CALL_LOG, "CVideoDecodingThread::DecodeAndSendToClient2 nScreenSplitType %d", m_pVideoCallSession->GetScreenSplitType());

			if (m_pColorConverter->GetSmallFrameStatus() == true)
			{
				int nSmallFrameHeight;
				int nSmallFrameWidth;

				CLogPrinter_LOG(CO_HOST_CALL_LOG, "CVideoDecodingThread::DecodeAndSendToClient2 1 nScreenSplitType %d", m_pVideoCallSession->GetScreenSplitType());

				this->m_pColorConverter->GetSmallFrame(m_ucaOpponentSmallFrame, nSmallFrameHeight, nSmallFrameWidth);

				if ((nSmallFrameHeight > nSmallFrameWidth) && (m_pVideoCallSession->GetOponentDeviceType() == DEVICE_TYPE_DESKTOP && m_PreviousDecodingWidth > m_PreviousDecodingHeight))
				{
					CLogPrinter_LOG(CO_HOST_CALL_LOG, "CVideoDecodingThread::DecodeAndSendToClient2 2 nScreenSplitType %d", m_pVideoCallSession->GetScreenSplitType());

					this->m_pColorConverter->DownScaleYUV420_Dynamic_Version222(m_ucaOpponentSmallFrame, nSmallFrameHeight, nSmallFrameWidth, m_ucaTempFrame2, SPLIT_TYPE_DEVICE_DESKTOP_HEIGHT, SPLIT_TYPE_DEVICE_DESKTOP_MOBILE_WIDTH);

					this->m_pColorConverter->Merge_Two_Video2(false, m_PreviousDecodedFrameConvertedData, SPLIT_TYPE_DEVICE_DESKTOP_WIDTH, 0, iHeight, iWidth, m_ucaTempFrame2, SPLIT_TYPE_DEVICE_DESKTOP_HEIGHT, SPLIT_TYPE_DEVICE_DESKTOP_MOBILE_WIDTH);
				}
				else
				{
					if (m_nPreviousInsetHeight > 16 && (m_nPreviousInsetHeight < iHeight / 2))
					{
						int nHeightDiff = iHeight / 2 - m_nPreviousInsetHeight;
						int nSmallFrameModifiedHeight;
						int nSmallFrameModifiedWidth;

						this->m_pColorConverter->DownScaleYUV420_Dynamic_Version222(m_ucaOpponentSmallFrame, nSmallFrameHeight, nSmallFrameWidth, m_ucaTempTempFrame, iHeight / 2, iWidth / 2);

						this->m_pColorConverter->Crop_YUV420(m_ucaTempTempFrame, iHeight / 2, iWidth / 2, 0, 0, nHeightDiff / 2, nHeightDiff / 2, m_ucaTempFrame2, nSmallFrameModifiedHeight, nSmallFrameModifiedWidth);

						this->m_pColorConverter->Merge_Two_Video2(false, m_PreviousDecodedFrameConvertedData, iWidth / 2, upperOffset, iHeight, iWidth, m_ucaTempFrame2, iHeight / 2 - nHeightDiff, iWidth / 2);
					}
					else
					{
						this->m_pColorConverter->DownScaleYUV420_Dynamic_Version222(m_ucaOpponentSmallFrame, nSmallFrameHeight, nSmallFrameWidth, m_ucaTempFrame2, iHeight / 2, iWidth / 2);

						this->m_pColorConverter->Merge_Two_Video2(false, m_PreviousDecodedFrameConvertedData, iWidth / 2, upperOffset, iHeight, iWidth, m_ucaTempFrame2, iHeight / 2, iWidth / 2);
					}
				}
			}
		}
		else
		{
			CLogPrinter_LOG(CO_HOST_CALL_LOG, "CVideoDecodingThread::DecodeAndSendToClient2 nScreenSplitType %d", m_pVideoCallSession->GetScreenSplitType());

			this->m_pColorConverter->Merge_Two_Video(m_PreviousDecodedFrameConvertedData, iPosX, iPosY, m_PreviousDecodingHeight, m_PreviousDecodingWidth);
		}

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

	long long nIDR_Frame_Gap = -1;
	int CVideoDecodingThread::DecodeAndSendToClient(unsigned char *in_data, unsigned int frameSize, long long nFramNumber, long long nTimeStampDiff, int nOrientation, int nInsetHeight, int nInsetWidth, int iOpponentLibraryVersion)
	{
		int nOrientationForRotation = nOrientation;

		long long currentTimeStamp = CLogPrinter_WriteLog(CLogPrinter::INFO, OPERATION_TIME_LOG);

		long long decTime = m_Tools.CurrentTimestamp();

		int nalType = m_Tools.GetEncodedFrameType(in_data);

		if (nalType == SPS_DATA)
		{
			printf("TheKing--> IDR FRAME Recieved, nFrameNumber = %lld, IDR_FRAME_GAP = %lld\n", nFramNumber, nFramNumber - nIDR_Frame_Gap);
			nIDR_Frame_Gap = nFramNumber;
		}

		m_decodedFrameSize = 0;

		if (frameSize > 0)
		{

			//fwrite (in_data , sizeof(char), frameSize, pFile);
			//fflush(pFile);

#if defined(TARGET_OS_WINDOWS_PHONE)

			m_decodedFrameSize = m_pVideoDecoder->DecodeVideoFrame(in_data, frameSize, m_TempDecodedFrame, m_decodingHeight, m_decodingWidth);

#else

			m_decodedFrameSize = m_pVideoDecoder->DecodeVideoFrame(in_data, frameSize, m_DecodedFrame, m_decodingHeight, m_decodingWidth);

#endif
		}

		if (frameSize > 0 && m_decodedFrameSize == 0)
			m_DecodeFailCounter++;

		if (frameSize == 0)
			m_NoFrameCounter++;

		CLogPrinter_LOG(DECODING_FAIL_LOG, "CVideoDecodingThread::DecodeAndSendToClient frameSize %d m_decodedFrameSize %d m_decodingHeight %d, m_decodingWidth %d, m_NoFrameCounter %d m_DecodeFailCounter %d", frameSize, m_decodedFrameSize, m_decodingHeight, m_decodingWidth, m_NoFrameCounter, m_DecodeFailCounter);

		CLogPrinter_WriteFileLog(CLogPrinter::INFO, WRITE_TO_LOG_FILE, "CVideoDecodingThread::DecodeAndSendToClient() Decoded Frame m_decodedFrameSize " + m_Tools.getText(m_decodedFrameSize));

		CLogPrinter_WriteLog(CLogPrinter::INFO, THREAD_LOG, "CVideoDecodingThread::DecodeAndSendToClient() Decoded Frame m_decodedFrameSize " + m_Tools.getText(m_decodedFrameSize));

		//printf("#V### Decoded Size -> %d +++E.Size:  %d\n",m_decodedFrameSize,(int)frameSize);
		printf("TheKing--> Decoded Height:Width = %d:%d, iLen = %d\n", m_decodingHeight, m_decodingWidth, m_decodedFrameSize);
		m_pCalculatorDecodeTime->UpdateData(m_Tools.CurrentTimestamp() - decTime);

		//if (m_decodingHeight > MAX_FRAME_HEIGHT || m_decodingWidth > MAX_FRAME_WIDTH)

		if (1 > m_decodedFrameSize)
			return -1;

#if defined(TARGET_OS_WINDOWS_PHONE)

		if(m_decodedFrameSize > MAX_VIDEO_DECODER_FRAME_SIZE )
			m_decodedFrameSize = this->m_pColorConverter->DownScaleYUV420_Dynamic(m_TempDecodedFrame, m_decodingHeight, m_decodingWidth, m_DecodedFrame, 2);
		else
			memcpy(m_DecodedFrame, m_TempDecodedFrame, m_decodedFrameSize);

#endif

		if (m_decodedFrameSize > m_iMaxLen)
		{
			m_pCommonElementBucket->m_pEventNotifier->fireVideoNotificationEvent(m_llFriendID, m_pCommonElementBucket->m_pEventNotifier->RESOLUTION_NOT_SUPPORTED);

			return -1;
		}

		if (m_pVideoCallSession->GetServiceType() == SERVICE_TYPE_CHANNEL)
		{
			if (m_VideoBeautificationer == NULL)
			{
				CLogPrinter_LOG(CHANNEL_ENHANCE_LOG, "CVideoDecodingThread::DecodeAndSendToClient starting decoder with m_decodingHeight %d, m_decodingWidth %d", m_decodingHeight, m_decodingWidth);

				m_VideoBeautificationer = new CVideoBeautificationer(m_decodingHeight, m_decodingWidth, m_pVideoCallSession->GetChannelType());
			}
			else if (m_nPreviousDecodingHeight != m_decodingHeight || m_nPreviousDecodingWidth != m_decodingWidth)
			{
				if (NULL != m_VideoBeautificationer)
				{
					delete m_VideoBeautificationer;
					m_VideoBeautificationer = NULL;
				}

				CLogPrinter_LOG(CHANNEL_ENHANCE_LOG, "CVideoDecodingThread::DecodeAndSendToClient Re Starting decoder with m_decodingHeight %d, m_decodingWidth %d m_nPreviousDecodingHeight %d m_nPreviousDecodingWidth %d", m_decodingHeight, m_decodingWidth, m_nPreviousDecodingHeight, m_nPreviousDecodingWidth);

				m_VideoBeautificationer = new CVideoBeautificationer(m_decodingHeight, m_decodingWidth, m_pVideoCallSession->GetChannelType());
			}

			pair<int, int> resultPair = m_VideoBeautificationer->BeautificationFilterForChannel(m_DecodedFrame, m_decodedFrameSize, m_decodingHeight, m_decodingWidth, m_decodingHeight, m_decodingWidth, true);

			m_nPreviousDecodingHeight = m_decodingHeight;
			m_nPreviousDecodingWidth = m_decodingWidth;
		}
		
		// CLogPrinter_WriteLog(CLogPrinter::INFO, INSTENT_TEST_LOG, "TheKing--> DecodingTime  = " + m_Tools.LongLongtoStringConvert(m_Tools.CurrentTimestamp() - decTime) + ", CurrentCallFPS = " + m_Tools.IntegertoStringConvert(m_nCallFPS) + ", iVideoheight = " + m_Tools.IntegertoStringConvert(m_decodingHeight) + ", iVideoWidth = " + m_Tools.IntegertoStringConvert(m_decodingWidth) + ", AverageDecodeTime --> " + m_Tools.DoubleToString(m_pCalculatorDecodeTime->GetAverage()) + ", Decoder returned = " + m_Tools.IntegertoStringConvert(m_decodedFrameSize) + ", FrameNumber = " + m_Tools.IntegertoStringConvert(nFramNumber));

		CLogPrinter_WriteLog(CLogPrinter::INFO, OPERATION_TIME_LOG, " Decode ", currentTimeStamp);

		m_pVideoCallSession->SetOpponentVideoHeightWidth(m_decodingHeight, m_decodingWidth);

		if (m_pVideoCallSession->GetServiceType() == SERVICE_TYPE_CHANNEL)
		{
			nOrientation = 0;
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

		if (m_pVideoCallSession->GetServiceType() == SERVICE_TYPE_LIVE_STREAM || m_pVideoCallSession->GetServiceType() == SERVICE_TYPE_SELF_STREAM)
		{
			nOrientation = 0;
			if (m_pVideoCallSession->GetEntityType() == ENTITY_TYPE_PUBLISHER_CALLER)
			{
				CLogPrinter_WriteLog(CLogPrinter::INFO, INSTENT_TEST_LOG_2, "CVideoDecodingThread::DecodeAndSendToClient() SetSmallFrame m_decodingHeight " + m_Tools.getText(m_decodingHeight) + " m_decodingWidth " + m_Tools.getText(m_decodingWidth));

				int iHeight = m_pVideoCallSession->getVideoCallHeight();
				int iWidth = m_pVideoCallSession->getVideoCallWidth();
                
                if(m_pVideoCallSession->GetOwnDeviceType() != DEVICE_TYPE_DESKTOP && m_pVideoCallSession->GetOponentDeviceType() != DEVICE_TYPE_DESKTOP)
                {
                    //inset Rotation is Turned off While User is in Desktop
                    int rotatedHeight, rotatedWidth;
                    
                    if(nOrientationForRotation == 3)
                        nOrientationForRotation = 1;
                    else if(nOrientationForRotation == 1)
                        nOrientationForRotation = 3;
                    else
                    {
                        //do nothing
                    }

					CLogPrinter_LOG(LIVE_INSET_LOG, "LIVE_INSET_LOG CVideoDecodingThread::DecodeAndSendToClient before rotation m_decodingHeight %d, m_decodingWidth %d, nOrientationForRotation %d", m_decodingHeight, m_decodingWidth, nOrientationForRotation);
                    
					int iLen = this->m_pColorConverter->RotateI420(m_DecodedFrame, m_decodingHeight, m_decodingWidth, m_RotatedFrame, rotatedHeight, rotatedWidth, nOrientationForRotation);
                    
                    memcpy(m_DecodedFrame, m_RotatedFrame, iLen);
                    m_decodingHeight = rotatedHeight;
                    m_decodingWidth = rotatedWidth;

					CLogPrinter_LOG(LIVE_INSET_LOG, "LIVE_INSET_LOG CVideoDecodingThread::DecodeAndSendToClient after rotation m_decodingHeight %d, m_decodingWidth %d, rotatedHeight %d, rotatedWidth %d, nOrientationForRotation %d", m_decodingHeight, m_decodingWidth, rotatedHeight, rotatedWidth, nOrientationForRotation);
                }
                
				CLogPrinter_LOG(LIVE_INSET_LOG, "LIVE_INSET_LOG CVideoDecodingThread::DecodeAndSendToClient betfore setting small frame m_decodingHeight %d, m_decodingWidth %d, iHeight %d, iWidth %d", m_decodingHeight, m_decodingWidth, iHeight, iWidth);

                this->m_pColorConverter->SetSmallFrame(m_DecodedFrame, m_decodingHeight, m_decodingWidth, m_decodedFrameSize, iHeight, iWidth, m_pVideoCallSession->GetOwnDeviceType() != DEVICE_TYPE_DESKTOP, min(LIBRARY_VERSION, iOpponentLibraryVersion));
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

				CLogPrinter_WriteLog(CLogPrinter::INFO, INSTENT_TEST_LOG_2, "CVideoDecodingThread::DecodeAndSendToClient() Merge_Two_Video iHeight " + m_Tools.getText(iHeight) + " iWidth " + m_Tools.getText(iWidth));

				if (m_pVideoCallSession->GetOponentDeviceType() != DEVICE_TYPE_DESKTOP)
				{
					m_pColorConverter->GetInsetLocation(iHeight, iWidth, iPosX, iPosY);
				}

				if (m_pVideoCallSession->GetScreenSplitType() == LIVE_CALL_SCREEN_SPLIT_TYPE_DIVIDE)
				{
					int upperOffset;

					if (m_nUpperOffsetOfInset > 5 && m_nUpperOffsetOfInset < 320)
						upperOffset = m_nUpperOffsetOfInset;
					else
						upperOffset = SPLIT_TYPE_DEVIDE_UPPER_OFFSET;

					CLogPrinter_LOG(CO_HOST_CALL_LOG, "CVideoDecodingThread::DecodeAndSendToClient nScreenSplitType %d", m_pVideoCallSession->GetScreenSplitType());

					if (m_pColorConverter->GetSmallFrameStatus() == true)
					{
						int nSmallFrameHeight;
						int nSmallFrameWidth;

						CLogPrinter_LOG(CO_HOST_CALL_LOG, "CVideoDecodingThread::DecodeAndSendToClient 1 nScreenSplitType %d", m_pVideoCallSession->GetScreenSplitType());

						this->m_pColorConverter->GetSmallFrame(m_ucaOpponentSmallFrame, nSmallFrameHeight, nSmallFrameWidth);

						if ((nSmallFrameHeight > nSmallFrameWidth) && (m_pVideoCallSession->GetOponentDeviceType() == DEVICE_TYPE_DESKTOP && m_PreviousDecodingWidth > m_PreviousDecodingHeight))
						{
							CLogPrinter_LOG(CO_HOST_CALL_LOG, "CVideoDecodingThread::DecodeAndSendToClient 2 nScreenSplitType %d", m_pVideoCallSession->GetScreenSplitType());

							this->m_pColorConverter->DownScaleYUV420_Dynamic_Version222(m_ucaOpponentSmallFrame, nSmallFrameHeight, nSmallFrameWidth, m_ucaTempFrame2, SPLIT_TYPE_DEVICE_DESKTOP_HEIGHT, SPLIT_TYPE_DEVICE_DESKTOP_MOBILE_WIDTH);

							this->m_pColorConverter->Merge_Two_Video2(false, m_DecodedFrame, SPLIT_TYPE_DEVICE_DESKTOP_WIDTH, 0, iHeight, iWidth, m_ucaTempFrame2, SPLIT_TYPE_DEVICE_DESKTOP_HEIGHT, SPLIT_TYPE_DEVICE_DESKTOP_MOBILE_WIDTH);
						}
						else
						{
							CLogPrinter_LOG(CO_HOST_CALL_LOG, "CVideoDecodingThread::DecodeAndSendToClient 3 nScreenSplitType %d", m_pVideoCallSession->GetScreenSplitType());

							if (nInsetHeight > 16 && (nInsetHeight < iHeight / 2))
							{
								int nHeightDiff = iHeight / 2 - nInsetHeight;
								int nSmallFrameModifiedHeight;
								int nSmallFrameModifiedWidth;

								this->m_pColorConverter->DownScaleYUV420_Dynamic_Version222(m_ucaOpponentSmallFrame, nSmallFrameHeight, nSmallFrameWidth, m_ucaTempTempFrame, iHeight / 2, iWidth / 2);

								this->m_pColorConverter->Crop_YUV420(m_ucaTempTempFrame, iHeight / 2, iWidth / 2, 0, 0, nHeightDiff / 2, nHeightDiff / 2, m_ucaTempFrame2, nSmallFrameModifiedHeight, nSmallFrameModifiedWidth);

								this->m_pColorConverter->Merge_Two_Video2(false, m_DecodedFrame, iWidth / 2, upperOffset, iHeight, iWidth, m_ucaTempFrame2, iHeight / 2 - nHeightDiff, iWidth / 2);
							}
							else
							{
								this->m_pColorConverter->DownScaleYUV420_Dynamic_Version222(m_ucaOpponentSmallFrame, nSmallFrameHeight, nSmallFrameWidth, m_ucaTempFrame2, iHeight / 2, iWidth / 2);

								this->m_pColorConverter->Merge_Two_Video2(false, m_DecodedFrame, iWidth / 2, upperOffset, iHeight, iWidth, m_ucaTempFrame2, iHeight / 2, iWidth / 2);
							}
						}
					}
				}
				else
				{
					CLogPrinter_LOG(CO_HOST_CALL_LOG, "CVideoDecodingThread::DecodeAndSendToClient nScreenSplitType %d", m_pVideoCallSession->GetScreenSplitType());

					this->m_pColorConverter->Merge_Two_Video(m_DecodedFrame, iPosX, iPosY, iHeight, iWidth);
				}
			}
			//TheKing-->Here

		}

		//QuickFix

		//printf("service type = %d, ownDeviceType = %d, OpponentDevice type = %d\n", m_pVideoCallSession->GetServiceType(), m_pVideoCallSession->GetOwnDeviceType(), m_pVideoCallSession->GetOponentDeviceType());

		/*if ((m_pVideoCallSession->GetServiceType() == SERVICE_TYPE_LIVE_STREAM || m_pVideoCallSession->GetServiceType() == SERVICE_TYPE_SELF_STREAM) && m_pVideoCallSession->GetOwnDeviceType() == DEVICE_TYPE_DESKTOP && m_pVideoCallSession->GetOponentDeviceType() != DEVICE_TYPE_DESKTOP)
		{
			int iHeight = m_pVideoCallSession->m_nVideoCallHeight;
			int iWidth = m_pVideoCallSession->m_nVideoCallWidth;

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
		}*/



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

		/*if ((m_pVideoCallSession->GetServiceType() == SERVICE_TYPE_LIVE_STREAM || m_pVideoCallSession->GetServiceType() == SERVICE_TYPE_SELF_STREAM) && (m_pVideoCallSession->GetOwnDeviceType() != DEVICE_TYPE_DESKTOP))
		{
			int iHeight = m_pVideoCallSession->m_nVideoCallHeight;
			int iWidth = m_pVideoCallSession->m_nVideoCallWidth;

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
		}*/

		memcpy(m_PreviousDecodedFrame, m_DecodedFrame, m_decodedFrameSize);
		m_previousDecodedFrameSize = m_decodedFrameSize;
		m_PreviousDecodingHeight = m_decodingHeight;
		m_PreviousDecodingWidth = m_decodingWidth;

		/*
		if (m_pVideoCallSession->GetServiceType() == SERVICE_TYPE_CALL && m_pVideoCallSession->GetOwnDeviceType() == DEVICE_TYPE_ANDROID && m_pVideoCallSession->GetOponentDeviceType() != DEVICE_TYPE_DESKTOP)
		{
		int iHeight = this->m_pColorConverter->GetHeight();
		int iWidth = this->m_pColorConverter->GetWidth();

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
		int iCroppedDataLen = this->m_pColorConverter->CropWithAspectRatio_YUVNV12_YUVNV21_RGB24(m_DecodedFrame, m_decodingHeight, m_decodingWidth, iScreenHeight, iScreenWidth, m_CropedFrame, iCropedHeight, iCropedWidth, YUVNV12);

		memcpy(m_DecodedFrame, m_CropedFrame, iCroppedDataLen);

		m_decodingHeight = iCropedHeight;
		m_decodingWidth = iCropedWidth;
		m_decodedFrameSize = iCroppedDataLen;
		}
		}
		*/

		if (m_pVideoCallSession->GetCalculationStatus() == true && m_pVideoCallSession->GetResolationCheck() == false)
		{
			m_Counter++;
			long long currentTimeStampForBrust = m_Tools.CurrentTimestamp();
			long long diff = currentTimeStampForBrust - m_pVideoCallSession->GetCalculationStartTime();
            
			CLogPrinter_WriteLog(CLogPrinter::INFO, INSTENT_TEST_LOG || CHECK_CAPABILITY_LOG, "Inside m_Counter = " + m_Tools.getText(m_Counter)
				+ ", CalculationStartTime = " + m_Tools.getText(m_pVideoCallSession->GetCalculationStartTime())
				+ ", CurrentTime = " + m_Tools.getText(currentTimeStampForBrust) + ", m_nCallFPS = " + m_Tools.getText(m_nCallFPS) + ", diff = " + m_Tools.getText(diff));

			if (m_Counter >= (m_nCallFPS - FPS_TOLERANCE_FOR_HIGH_RESOLUTION) && diff <= 1000)
			{
				//   m_pCommonElementsBucket->m_pEventNotifier->fireVideoEvent(m_FriendID, nFrameNumber, frameSize, m_RenderingFrame, videoHeight, videoWidth);
				m_pVideoCallSession->SetCalculationStartMechanism(false);
				m_pVideoCallSession->DecideHighResolatedVideo(true);

				CLogPrinter_WriteLog(CLogPrinter::INFO, CHECK_CAPABILITY_LOG, " CVideoDecodingThread::DecodeAndSendToClient() SUCCESSED for iVideoheight = " + m_Tools.IntegertoStringConvert(m_decodingHeight) + ", iVideoWidth = " + m_Tools.IntegertoStringConvert(m_decodingWidth));

				//printFile("%s", sss.c_str());
				//printfiledone();

			}
			else if (diff > 1000)
			{
				CLogPrinter_WriteLog(CLogPrinter::INFO, CHECK_CAPABILITY_LOG, " CVideoDecodingThread::DecodeAndSendToClient() FAILED for iVideoheight = " + m_Tools.IntegertoStringConvert(m_decodingHeight) + ", iVideoWidth = " + m_Tools.IntegertoStringConvert(m_decodingWidth));

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
