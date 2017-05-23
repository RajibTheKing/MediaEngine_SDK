
#include "VideoDecodingThreadOfCall.h"
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

	CVideoDecodingThreadOfCall::CVideoDecodingThreadOfCall(CEncodedFrameDepacketizer *encodedFrameDepacketizer, long long llFriendID, CCommonElementsBucket *pCommonElementBucket, CRenderingBuffer *renderingBuffer,
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

	CVideoDecodingThreadOfCall::~CVideoDecodingThreadOfCall()
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

	void CVideoDecodingThreadOfCall::SetCallFPS(int nFPS)
	{
		m_nCallFPS = nFPS;
	}

	void CVideoDecodingThreadOfCall::InstructionToStop()
	{
		bDecodingThreadRunning = false;
	}

	void CVideoDecodingThreadOfCall::StopDecodingThread()
	{
		CLogPrinter_WriteLog(CLogPrinter::INFO, THREAD_LOG, "CVideoDecodingThreadOfCall::StopDecodingThread called");

		//if (pDepacketizationThread.get())
		{
			bDecodingThreadRunning = false;

			while (!bDecodingThreadClosed)
			{
				m_Tools.SOSleep(5);
			}
		}
		//pDepacketizationThread.reset();

		CLogPrinter_WriteLog(CLogPrinter::INFO, THREAD_LOG, "CVideoDecodingThreadOfCall::StopDecodingThread Decoding Thread STOPPED");
	}
	void CVideoDecodingThreadOfCall::Reset()
	{
		m_dbAverageDecodingTime = 0;
		m_dbTotalDecodingTime = 0;
		//int m_nOponnentFPS, m_nMaxProcessableByMine;
		m_iDecodedFrameCounter = 0;
		m_nMaxDecodingTime = 0;
		m_FpsCounter = 0;
	}
	void CVideoDecodingThreadOfCall::StartDecodingThread()
	{
		CLogPrinter_WriteLog(CLogPrinter::INFO, THREAD_LOG, "CVideoDecodingThreadOfCall::StartDecodingThread called");

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

		CLogPrinter_WriteLog(CLogPrinter::INFO, THREAD_LOG, "CVideoDecodingThreadOfCall::StartDecodingThread Decoding Thread started");

		return;
	}

	void *CVideoDecodingThreadOfCall::CreateDecodingThread(void* param)
	{
		CVideoDecodingThreadOfCall *pThis = (CVideoDecodingThreadOfCall*)param;
		pThis->DecodingThreadProcedure();

		return NULL;
	}

	void CVideoDecodingThreadOfCall::ResetForPublisherCallerCallEnd()
	{
		m_bResetForPublisherCallerCallEnd = true;

		while (m_bResetForPublisherCallerCallEnd)
		{
			m_Tools.SOSleep(5);
		}
	}

	void CVideoDecodingThreadOfCall::ResetForViewerCallerCallStartEnd()
	{
		m_bResetForViewerCallerCallStartEnd = true;

		while (m_bResetForViewerCallerCallStartEnd)
		{
			m_Tools.SOSleep(5);
		}
	}

	void CVideoDecodingThreadOfCall::DecodingThreadProcedure()
	{
		CLogPrinter_WriteLog(CLogPrinter::INFO, THREAD_LOG, "CVideoDecodingThreadOfCall::DecodingThreadProcedure() started DecodingThreadProcedure method");

		Tools toolsObject;

		int nFrameNumber, nFrameLength, nEncodingTime, nOrientation;
		unsigned int nTimeStampDiff = 0;
		long long currentTime;

		int nExpectedTime;

		int nDecodingStatus, fps = -1;

		int nOponnentFPS, nMaxProcessableByMine;
		nExpectedTime = -1;
		long long maxDecodingTime = 0, framCounter = 0, decodingTime, nBeforeDecodingTime;
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
			if (m_bResetForPublisherCallerCallEnd == true)
			{
				m_pEncodedFrameDepacketizer->ResetEncodedFrameDepacketizer();

				m_bResetForPublisherCallerCallEnd = false;
			}

			if (-1 == m_pVideoCallSession->GetShiftedTime())
			{
				CLogPrinter_WriteLog(CLogPrinter::INFO, THREAD_LOG, "CVideoDecodingThreadOfCall::DecodingThreadProcedure() Not started transmission");
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
					m_Tools.IntegertoStringConvert(
					nFrameNumber) + "  Len: " +
					m_Tools.IntegertoStringConvert(
					nFrameLength) +
					"  E.Time: " +
					m_Tools.IntegertoStringConvert(
					nEncodingTime)
					+ "  Exp E.Time: " +
					m_Tools.IntegertoStringConvert(
					nExpectedTime) + " -> " +
					m_Tools.IntegertoStringConvert(
					nExpectedTime -
					nEncodingTime) + "Orientation = " +
					m_Tools.IntegertoStringConvert(nOrientation));
				CLogPrinter_WriteLog(CLogPrinter::DEBUGS, DEPACKETIZATION_LOG, "#$ Cur: " + m_Tools.LongLongToString(currentTime) + " diff: " + m_Tools.LongLongToString(currentTime - g_ArribalTime[nFrameNumber]));


			}


			if (-1 == nFrameLength)
			{
				CLogPrinter_WriteLog(CLogPrinter::INFO, THREAD_LOG, "CVideoDecodingThreadOfCall::DecodingThreadProcedure() NOTHING for decoding method");

				toolsObject.SOSleep(10);
			}
			else
			{
				CLogPrinter_WriteLog(CLogPrinter::INFO, THREAD_LOG, "CVideoDecodingThreadOfCall::DecodingThreadProcedure() GOT FRAME FOR DDDDDDDDDDDDDdecoding method");

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

					CLogPrinter_WriteSpecific(CLogPrinter::DEBUGS, "CVideoDecodingThreadOfCall::DecodingThreadProcedure() Force:: Frame: " + m_Tools.IntegertoStringConvert(nFrameNumber) + "  FPS: " + m_Tools.IntegertoStringConvert(nOponnentFPS) + " ~" + toolsObject.IntegertoStringConvert(nMaxProcessableByMine));
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


				nDecodingStatus = DecodeAndSendToClient(m_PacketizedFrame, nFrameLength, nFrameNumber, nEncodingTime, nOrientation, 0, 0);
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
					CLogPrinter_WriteSpecific(CLogPrinter::DEBUGS, "CVideoDecodingThreadOfCall::DecodingThreadProcedure() Force:: AVG Decoding Time:" + m_Tools.DoubleToString(dbAverageDecodingTime) + "  Max Decoding-time: " + m_Tools.IntegertoStringConvert(nMaxDecodingTime) + "  MaxOwnProcessable: " + m_Tools.IntegertoStringConvert(fps));
				}

				toolsObject.SOSleep(1);
			}
		}

		bDecodingThreadClosed = true;

		CLogPrinter_WriteLog(CLogPrinter::INFO, THREAD_LOG, "CVideoDecodingThreadOfCall::DecodingThreadProcedure() stopped DecodingThreadProcedure method.");
	}

	int nIDR_Frame_GapOfCall = -1;
	int CVideoDecodingThreadOfCall::DecodeAndSendToClient(unsigned char *in_data, unsigned int frameSize, int nFramNumber, unsigned int nTimeStampDiff, int nOrientation, int nInsetHeight, int nInsetWidth)
	{
		long long currentTimeStamp = CLogPrinter_WriteLog(CLogPrinter::INFO, OPERATION_TIME_LOG);

		long long decTime = m_Tools.CurrentTimestamp();

		int nalType = m_Tools.GetEncodedFrameType(in_data);

		if (nalType == SPS_DATA)
		{
			printf("TheKing--> IDR FRAME Recieved, nFrameNumber = %d, IDR_FRAME_GAP = %d\n", nFramNumber, nFramNumber - nIDR_Frame_GapOfCall);
			nIDR_Frame_GapOfCall = nFramNumber;
		}

#if defined(TARGET_OS_WINDOWS_PHONE)

		m_decodedFrameSize = m_pVideoDecoder->DecodeVideoFrame(in_data, frameSize, m_TempDecodedFrame, m_decodingHeight, m_decodingWidth);

#else

		m_decodedFrameSize = m_pVideoDecoder->DecodeVideoFrame(in_data, frameSize, m_DecodedFrame, m_decodingHeight, m_decodingWidth);

#endif

		CLogPrinter_WriteFileLog(CLogPrinter::INFO, WRITE_TO_LOG_FILE, "CVideoDecodingThreadOfCall::DecodeAndSendToClient() Decoded Frame m_decodedFrameSize " + m_Tools.getText(m_decodedFrameSize));

		CLogPrinter_WriteLog(CLogPrinter::INFO, THREAD_LOG, "CVideoDecodingThreadOfCall::DecodeAndSendToClient() Decoded Frame m_decodedFrameSize " + m_Tools.getText(m_decodedFrameSize));

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

				CLogPrinter_WriteLog(CLogPrinter::INFO, CHECK_CAPABILITY_LOG, " CVideoDecodingThreadOfCall::DecodeAndSendToClient() SUCCESSED for iVideoheight = " + m_Tools.IntegertoStringConvert(m_decodingHeight) + ", iVideoWidth = " + m_Tools.IntegertoStringConvert(m_decodingWidth));

				//printFile("%s", sss.c_str());
				//printfiledone();

			}
			else if (diff > 1000)
			{
				CLogPrinter_WriteLog(CLogPrinter::INFO, CHECK_CAPABILITY_LOG, " CVideoDecodingThreadOfCall::DecodeAndSendToClient() FAILED for iVideoheight = " + m_Tools.IntegertoStringConvert(m_decodingHeight) + ", iVideoWidth = " + m_Tools.IntegertoStringConvert(m_decodingWidth));

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
