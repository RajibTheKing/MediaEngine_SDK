
#include "VideoCallSession.h"
#include "VideoEncodingThreadOfCall.h"

#include "LogPrinter.h"
#include "VideoCallSession.h"
#include "CommonElementsBucket.h"

#if defined(TARGET_OS_IPHONE) || defined(TARGET_IPHONE_SIMULATOR)
#include <dispatch/dispatch.h>
#endif

namespace MediaSDK
{

	CVideoEncodingThreadOfCall::CVideoEncodingThreadOfCall(long long llFriendID, CEncodingBuffer *pEncodingBuffer, CCommonElementsBucket *commonElementsBucket, BitRateController *pBitRateController, IDRFrameIntervalController *pIdrFrameController, CColorConverter *pColorConverter, CVideoEncoder *pVideoEncoder, CEncodedFramePacketizer *pEncodedFramePacketizer, CVideoCallSession *pVideoCallSession, int nFPS, bool bIsCheckCall, bool bSelfViewOnly) :

		m_pVideoCallSession(pVideoCallSession),
		m_iFrameNumber(nFPS),
		m_llFriendID(llFriendID),
		m_pEncodingBuffer(pEncodingBuffer),
		m_pBitRateController(pBitRateController),
		m_pIdrFrameIntervalController(pIdrFrameController),
		m_pColorConverter(pColorConverter),
		m_pVideoEncoder(pVideoEncoder),
		m_pEncodedFramePacketizer(pEncodedFramePacketizer),
		mt_nTotalEncodingTimePerFrameRate(0),
		m_bIsThisThreadStarted(false),
		m_FPS_TimeDiff(0),
		m_FpsCounter(0),
		m_nCallFPS(nFPS),
		m_bNotifyToClientVideoQuality(false),
		m_pCommonElementBucket(commonElementsBucket),
		m_bVideoEffectEnabled(true),
m_VideoBeautificationer(NULL),
		m_bSelfViewOnly(bSelfViewOnly)
	{
		m_pCalculatorEncodeTime = new CAverageCalculator();
		m_pCalculateEncodingTimeDiff = new CAverageCalculator();

	//m_VideoBeautificationer = new CVideoBeautificationer(m_pVideoCallSession->m_nVideoCallHeight, m_pVideoCallSession->m_nVideoCallWidth);
		//m_VideoBeautificationer->GenerateUVIndex(this->m_pColorConverter->GetHeight(), this->m_pColorConverter->GetWidth(), 11);

		m_VideoEffects = new CVideoEffects();

		m_pVideoCallSession = pVideoCallSession;
		m_bIsCheckCall = bIsCheckCall;

		if (m_bIsCheckCall == DEVICE_ABILITY_CHECK_MOOD)
		{
			for (int k = 0; k < 3; k++)
			{
				memset(m_ucaDummmyFrame[k], 0, sizeof(m_ucaDummmyFrame[k]));

				for (int i = 0; i < m_pVideoCallSession->m_nVideoCallHeight; i++)
				{
					int color = rand() % 255;
					for (int j = 0; j < m_pVideoCallSession->m_nVideoCallWidth; j++)
					{
						m_ucaDummmyFrame[k][i * m_pVideoCallSession->m_nVideoCallHeight + j] = color;
					}

				}
			}
		}

		m_filterToApply = 0;
	}

	CVideoEncodingThreadOfCall::~CVideoEncodingThreadOfCall()
	{
		if (NULL != m_pCalculatorEncodeTime)
		{
			delete m_pCalculatorEncodeTime;
			m_pCalculatorEncodeTime = NULL;
		}

		if (NULL != m_pCalculateEncodingTimeDiff)
		{
			delete m_pCalculateEncodingTimeDiff;
			m_pCalculateEncodingTimeDiff = NULL;
		}

		if (NULL != m_VideoBeautificationer)
		{
			delete m_VideoBeautificationer;
			m_VideoBeautificationer = NULL;
		}

		if (NULL != m_VideoEffects)
		{
			delete m_VideoEffects;
			m_VideoEffects = NULL;
		}
	}

	void CVideoEncodingThreadOfCall::SetCallFPS(int nFPS)
	{
		m_nCallFPS = nFPS;
	}

	void CVideoEncodingThreadOfCall::ResetVideoEncodingThread(BitRateController *pBitRateController)
	{
		m_iFrameNumber = 0;
		m_pBitRateController = pBitRateController;
	}

	void CVideoEncodingThreadOfCall::StopEncodingThread()
	{
		CLogPrinter_WriteLog(CLogPrinter::INFO, THREAD_LOG, "CVideoEncodingThreadOfCall::StopEncodingThread called");

		//if (pInternalThread.get())
		{

			bEncodingThreadRunning = false;

			while (!bEncodingThreadClosed)
			{
				m_Tools.SOSleep(5);
			}
		}

		//pInternalThread.reset();

		CLogPrinter_WriteLog(CLogPrinter::INFO, THREAD_LOG, "CVideoEncodingThreadOfCall::StopEncodingThread Encoding Thread STOPPED");
	}

	void CVideoEncodingThreadOfCall::StartEncodingThread()
	{
		CLogPrinter_WriteLog(CLogPrinter::INFO, THREAD_LOG, "CVideoEncodingThreadOfCall::StartEncodingThread called");

		if (pEncodingThread.get())
		{
			pEncodingThread.reset();

			return;
		}

		bEncodingThreadRunning = true;
		bEncodingThreadClosed = false;

#if defined(TARGET_OS_IPHONE) || defined(TARGET_IPHONE_SIMULATOR)

		dispatch_queue_t EncodeThreadQ = dispatch_queue_create("EncodeThreadQ", DISPATCH_QUEUE_CONCURRENT);
		dispatch_async(EncodeThreadQ, ^{
			this->EncodingThreadProcedure();
		});

#else

		std::thread myThread(CreateVideoEncodingThread, this);
		myThread.detach();

#endif

		CLogPrinter_WriteLog(CLogPrinter::INFO, THREAD_LOG, "CVideoEncodingThreadOfCall::StartEncodingThread Encoding Thread started");

		return;
	}

	void *CVideoEncodingThreadOfCall::CreateVideoEncodingThread(void* param)
	{
		CVideoEncodingThreadOfCall *pThis = (CVideoEncodingThreadOfCall*)param;
		pThis->EncodingThreadProcedure();

		return NULL;
	}

	void CVideoEncodingThreadOfCall::SetOrientationType(int nOrientationType)
	{
		m_nOrientationType = nOrientationType;
	}

	bool CVideoEncodingThreadOfCall::IsThreadStarted()
	{
		return m_bIsThisThreadStarted;
	}

	void CVideoEncodingThreadOfCall::SetNotifierFlag(bool flag)
	{
		m_bNotifyToClientVideoQuality = flag;
	}

	void CVideoEncodingThreadOfCall::SetFrameNumber(int nFrameNumber)
	{
		m_iFrameNumber = nFrameNumber;
	}

	int CVideoEncodingThreadOfCall::SetVideoEffect(int nEffectStatus)
	{
		if (nEffectStatus == 1)
			m_bVideoEffectEnabled = true;
		else if (nEffectStatus == 0)
			m_bVideoEffectEnabled = false;

		m_filterToApply = nEffectStatus;

		return 1;
	}


	long long g_PrevEncodeTimeOfCall = 0;

	void CVideoEncodingThreadOfCall::EncodingThreadProcedure()
	{
		CLogPrinter_WriteLog(CLogPrinter::INFO, THREAD_LOG, "CVideoEncodingThreadOfCall::EncodingThreadProcedure() started EncodingThreadProcedure method");

		Tools toolsObject;
        toolsObject.SetThreadName("EncodingCall");
		int nEncodingFrameSize, nENCODEDFrameSize, nCaptureTimeDifference, nDevice_orientation;
		long long llCalculatingTime;
		int sumOfEncodingTimediff = 0;
		int sumOfZeroLengthEncodingTimediff = 0;
		int countZeroLengthFrame = 0;
		bool bIsBitrateInitialized = false;
		long long llPacketizePrevTime = 0;

		int sum = 0;
		int sum2 = 0;
		int sum3 = 0;
		int countNumber = 1;

		bool bNeedIDR = false;

		int iGotHeight;
		int iGotWidth;

		/*for(int i = 0; i < 200; i++)
		{
		if (m_pBitRateController->IsNetworkTypeMiniPacketReceived())
		{
		CLogPrinter_WriteSpecific5(CLogPrinter::INFO, "CVideoEncodingThreadOfCall::EncodingThreadProcedure() m_pBitRateController->m_iNetworkType after waiting = " + toolsObject.IntegertoStringConvert(m_pBitRateController->GetOpponentNetworkType()));
		break;
		}

		toolsObject.SOSleep(10);
		}*/

		if (m_bIsCheckCall)
		{
			m_pBitRateController->SetInitialBitrate();
			bIsBitrateInitialized = true;
		}

		while (bEncodingThreadRunning)
		{
			//CLogPrinter_WriteLog(CLogPrinter::INFO, THREAD_LOG ,"CVideoEncodingThreadOfCall::EncodingThreadProcedure() RUNNING EncodingThreadProcedure method");

			bNeedIDR = false;

			m_bIsThisThreadStarted = true;

			if (m_bNotifyToClientVideoQuality == true && m_bIsCheckCall == false)
			{
				m_bNotifyToClientVideoQuality = false;

				if (m_pVideoCallSession->GetCurrentVideoCallQualityLevel() == RESOLUTION_FPS_SUPPORT_NOT_TESTED || m_pVideoCallSession->GetCurrentVideoCallQualityLevel() == SUPPORTED_RESOLUTION_FPS_352_15)
					m_pCommonElementBucket->m_pEventNotifier->fireVideoNotificationEvent(m_pVideoCallSession->GetFriendID(), m_pCommonElementBucket->m_pEventNotifier->SET_CAMERA_RESOLUTION_352x288);
				else if (m_pVideoCallSession->GetCurrentVideoCallQualityLevel() == SUPPORTED_RESOLUTION_FPS_352_25)
					m_pCommonElementBucket->m_pEventNotifier->fireVideoNotificationEvent(m_pVideoCallSession->GetFriendID(), m_pCommonElementBucket->m_pEventNotifier->SET_CAMERA_RESOLUTION_352x288);
				else if (m_pVideoCallSession->GetCurrentVideoCallQualityLevel() == SUPPORTED_RESOLUTION_FPS_640_25)
					m_pCommonElementBucket->m_pEventNotifier->fireVideoNotificationEvent(m_pVideoCallSession->GetFriendID(), m_pCommonElementBucket->m_pEventNotifier->SET_CAMERA_RESOLUTION_640x480);

				m_pVideoCallSession->m_bVideoCallStarted = true;
			}


			//printf("TheVersion--> CurrentCallVersion = %d\n", m_pVideoCallSession->GetVersionController()->GetCurrentCallVersion());

			if (m_pVideoCallSession->GetVersionController()->GetCurrentCallVersion() == -1 && m_bIsCheckCall == false)
			{
				m_pEncodedFramePacketizer->Packetize(m_llFriendID, m_ucaEncodedFrame, /*SIZE*/ 0, /*m_iFrameNumber*/0, /*nCaptureTimeDifference*/0, 0, BLANK_DATA_MOOD, 0, 0);

				CLogPrinter_WriteLog(CLogPrinter::INFO, THREAD_LOG, "CVideoEncodingThreadOfCall::EncodingThreadProcedure() Negotiation uncomplete");

				toolsObject.SOSleep(20);
				continue;
			}

			if (m_pEncodingBuffer->GetQueueSize() == 0)
			{
				CLogPrinter_WriteLog(CLogPrinter::INFO, THREAD_LOG, "CVideoEncodingThreadOfCall::EncodingThreadProcedure() got NOTHING for encoding");

				//			CLogPrinter_WriteLog(CLogPrinter::INFO, INSTENT_TEST_LOG, " fahad Encode time buffer size 0");
				if (!m_pVideoCallSession->GetVersionController()->IsFirstVideoPacetReceived() && m_bIsCheckCall == false) {
					//			toolsObject.SOSleep(10000);
					//				VLOG("--------------------------------------------------------------> NOT RECEIVED");
					m_pEncodedFramePacketizer->Packetize(m_llFriendID, m_ucaEncodedFrame,
						2, /*m_iFrameNumber*/
						0, /*nCaptureTimeDifference*/0, 0,
						BLANK_DATA_MOOD, 0, 0);

				}

				toolsObject.SOSleep(10);
			}
			else
			{
				CLogPrinter_WriteLog(CLogPrinter::INFO, THREAD_LOG, "CVideoEncodingThreadOfCall::EncodingThreadProcedure() GOT packet for Encoding");
				int timeDiff;

				if (!bIsBitrateInitialized)
				{
					m_pBitRateController->SetInitialBitrate();
					bIsBitrateInitialized = true;
				}

				//CLogPrinter_WriteLog(CLogPrinter::INFO, INSTENT_TEST_LOG, " fahad Encode time ");

				nEncodingFrameSize = m_pEncodingBuffer->DeQueue(m_ucaEncodingFrame, iGotHeight, iGotWidth, timeDiff, nCaptureTimeDifference, nDevice_orientation);

				//LOGEF("Current bitrate %d", m_pVideoEncoder->GetBitrate());

				if (g_PrevEncodeTimeOfCall != 0)
					m_pCalculateEncodingTimeDiff->UpdateData(m_Tools.CurrentTimestamp() - g_PrevEncodeTimeOfCall);

				//printf("TheVampireEngg --> EncodingTime Diff = %lld, Average = %lf\n", m_Tools.CurrentTimestamp() - g_PrevEncodeTimeOfCall, m_CalculateEncodingTimeDiff.GetAverage());
				g_PrevEncodeTimeOfCall = m_Tools.CurrentTimestamp();

				long long startTime = m_Tools.CurrentTimestamp();


				CLogPrinter_WriteLog(CLogPrinter::INFO, QUEUE_TIME_LOG, " &*&*&* m_pEncodingBuffer ->" + toolsObject.IntegertoStringConvert(timeDiff));

				if (!m_pVideoCallSession->GetFPSController()->IsProcessableFrame() && m_bIsCheckCall == false)
				{
					CLogPrinter_WriteLog(CLogPrinter::INFO, THREAD_LOG, "CVideoEncodingThreadOfCall::EncodingThreadProcedure() not processable for FPS");

					toolsObject.SOSleep(10);

					continue;
				}
				//			CLogPrinter_WriteLog(CLogPrinter::INFO, INSTENT_TEST_LOG ," Client FPS: " + Tools::DoubleToString(m_pVideoCallSession->GetFPSController()->GetClientFPS()));

				m_pBitRateController->UpdateBitrate();

				llCalculatingTime = CLogPrinter_WriteLog(CLogPrinter::INFO, OPERATION_TIME_LOG);

#if defined(TARGET_OS_IPHONE) || defined(TARGET_IPHONE_SIMULATOR)

				this->m_pColorConverter->ConvertNV12ToI420(m_ucaEncodingFrame, iGotHeight, iGotWidth);

#elif defined(DESKTOP_C_SHARP)

				if (nEncodingFrameSize == iGotWidth * iGotHeight * 2)
				{
					nEncodingFrameSize = this->m_pColorConverter->ConvertYUY2ToI420(m_ucaEncodingFrame, m_ucaConvertedEncodingFrame, iGotHeight, iGotWidth);
				}
				else if (nEncodingFrameSize == iGotWidth * iGotHeight * 3)
				{
					nEncodingFrameSize = this->m_pColorConverter->ConvertRGB24ToI420(m_ucaEncodingFrame, m_ucaConvertedEncodingFrame, iGotHeight, iGotWidth);
				}

#elif defined(TARGET_OS_WINDOWS_PHONE)

				if (m_nOrientationType == ORIENTATION_90_MIRRORED)
				{
					this->m_pColorConverter->mirrorRotateAndConvertNV12ToI420(m_ucaEncodingFrame, m_ucaConvertedEncodingFrame, iGotHeight, iGotWidth);
				}
				else if (m_nOrientationType == ORIENTATION_0_MIRRORED)
				{
					this->m_pColorConverter->mirrorRotateAndConvertNV12ToI420ForBackCam(m_ucaEncodingFrame, m_ucaConvertedEncodingFrame, iGotHeight, iGotWidth);
				}

#else

				if (m_nOrientationType == ORIENTATION_90_MIRRORED)
				{
					this->m_pColorConverter->mirrorRotateAndConvertNV21ToI420(m_ucaEncodingFrame, m_ucaConvertedEncodingFrame, iGotHeight, iGotWidth);
				}
				else if (m_nOrientationType == ORIENTATION_0_MIRRORED)
				{
					this->m_pColorConverter->mirrorRotateAndConvertNV21ToI420ForBackCam90(m_ucaEncodingFrame, m_ucaConvertedEncodingFrame, iGotHeight, iGotWidth);
				}
				else if (m_nOrientationType == ORIENTATION_BACK_270_MIRRORED)
				{
					this->m_pColorConverter->mirrorRotateAndConvertNV21ToI420ForBackCam270(m_ucaEncodingFrame, m_ucaConvertedEncodingFrame, iGotHeight, iGotWidth);
				}

#endif
				int nServiceType = m_pVideoCallSession->GetServiceType();

				/*	
				if (m_VideoBeautificationer == NULL)
				{
					m_VideoBeautificationer = new CVideoBeautificationer(iGotHeight, iGotWidth);
				}

				bool doSharp = false;

				if (m_pVideoEncoder->GetBitrate() >= MIN_BITRATE_FOR_SHARPING)
					doSharp = true;

#if defined(TARGET_OS_IPHONE) || defined(TARGET_IPHONE_SIMULATOR)

				if (m_pVideoCallSession->GetOwnVideoCallQualityLevel() != SUPPORTED_RESOLUTION_FPS_352_15)
				{
					pair<int, int> resultPair = m_VideoBeautificationer->BeautificationFilter(m_ucaEncodingFrame, nEncodingFrameSize, iGotHeight, iGotWidth, iGotHeight - 5, iGotWidth - 5, doSharp);

				}
				else
				{
					pair<int, int> resultPair = m_VideoBeautificationer->BeautificationFilter2(m_ucaConvertedEncodingFrame, nEncodingFrameSize, iGotHeight, iGotWidth);
				}
#else

#ifndef TARGET_OS_WINDOWS_PHONE

				if (m_pVideoCallSession->GetOwnVideoCallQualityLevel() != SUPPORTED_RESOLUTION_FPS_352_15 || m_pVideoCallSession->GetOwnDeviceType() == DEVICE_TYPE_DESKTOP)
				{
					if (m_pVideoCallSession->GetOwnDeviceType() == DEVICE_TYPE_DESKTOP)
					{
						pair<int, int> resultPair = m_VideoBeautificationer->BeautificationFilter(m_ucaConvertedEncodingFrame, nEncodingFrameSize, iGotHeight, iGotWidth, doSharp);
					}
					else
						pair<int, int> resultPair = m_VideoBeautificationer->BeautificationFilter(m_ucaConvertedEncodingFrame, nEncodingFrameSize, iGotHeight, iGotWidth, iGotHeight - 5, iGotWidth - 5, doSharp);
				}
				else
				{
					pair<int, int> resultPair = m_VideoBeautificationer->BeautificationFilter(m_ucaConvertedEncodingFrame, nEncodingFrameSize, iGotHeight, iGotWidth, iGotHeight - 5, iGotWidth - 5, doSharp);
				}
#endif
				//m_pCommonElementBucket->m_pEventNotifier->fireVideoNotificationEvent(resultPair.first, resultPair.second);
#endif
				*/

				/*if(m_bIsCheckCall == true)
				{
				memset(m_ucaEncodingFrame, 0, sizeof(m_ucaEncodingFrame));

				for(int i=0;i<iGotHeight;i++)
				{
				int color = rand()%255;
				for(int j = 0; j < iGotWidth; j ++)
				{
				m_ucaEncodingFrame[i * iGotHeight + j ] = color;
				}

				}
				}*/

				if (m_bSelfViewOnly == false)
				{
					CLogPrinter_WriteLog(CLogPrinter::INFO, OPERATION_TIME_LOG, " Conversion ", llCalculatingTime);

					llCalculatingTime = CLogPrinter_WriteLog(CLogPrinter::INFO, OPERATION_TIME_LOG);

					if (m_pVideoCallSession->isDynamicIDR_Mechanism_Enable())
					{
						bNeedIDR = m_pIdrFrameIntervalController->NeedToGenerateIFrame(m_pVideoCallSession->GetServiceType());
					}

#if defined(TARGET_OS_IPHONE) || defined(TARGET_IPHONE_SIMULATOR)
					llCalculatingTime = m_Tools.CurrentTimestamp();

					if (m_bIsCheckCall)
						nENCODEDFrameSize = m_pVideoEncoder->EncodeVideoFrame(m_ucaDummmyFrame[m_iFrameNumber % 2], nEncodingFrameSize, m_ucaEncodedFrame, false);
					else
						nENCODEDFrameSize = m_pVideoEncoder->EncodeVideoFrame(m_ucaEncodingFrame, nEncodingFrameSize, m_ucaEncodedFrame, bNeedIDR);

					//printf("The encoder returned , nENCODEDFrameSize = %d, frameNumber = %d\n", nENCODEDFrameSize, m_iFrameNumber);


#else
					long long timeStampForEncoding = m_Tools.CurrentTimestamp();

					if (m_bIsCheckCall)
						nENCODEDFrameSize = m_pVideoEncoder->EncodeVideoFrame(m_ucaDummmyFrame[m_iFrameNumber % 3], nEncodingFrameSize, m_ucaEncodedFrame, false);
					else
						nENCODEDFrameSize = m_pVideoEncoder->EncodeVideoFrame(m_ucaConvertedEncodingFrame, nEncodingFrameSize, m_ucaEncodedFrame, bNeedIDR);

					//VLOG("#EN# Encoding Frame: " + m_Tools.IntegertoStringConvert(m_iFrameNumber));



					int timediff = (int)(m_Tools.CurrentTimestamp() - timeStampForEncoding);
					sumOfEncodingTimediff += timeDiff;
					if (nENCODEDFrameSize == 0)
					{
						sumOfZeroLengthEncodingTimediff += timeDiff;
						countZeroLengthFrame++;
					}
					if (m_iFrameNumber % m_nCallFPS == 0)
					{
						CLogPrinter_WriteLog(CLogPrinter::INFO, INSTENT_TEST_LOG, " m_iFrameNumber " + m_Tools.IntegertoStringConvert(m_iFrameNumber%m_nCallFPS) + " Encode time " + m_Tools.IntegertoStringConvert(timeDiff) +
							" sumOfEncodingTimediff " + m_Tools.IntegertoStringConvert(sumOfEncodingTimediff) + " ---  nENCODEDFrameSize  "
							+ m_Tools.IntegertoStringConvert(nENCODEDFrameSize) + " ---  countZeroLengthFrame  " + m_Tools.IntegertoStringConvert(countZeroLengthFrame) +
							" --- ***** afterFrameDropFps  " + m_Tools.IntegertoStringConvert(m_nCallFPS - countZeroLengthFrame));
						sumOfEncodingTimediff = 0;
						countZeroLengthFrame = 0;
					}

#endif

					m_pCalculatorEncodeTime->UpdateData(m_Tools.CurrentTimestamp() - llCalculatingTime);

					//            CLogPrinter_WriteLog(CLogPrinter::INFO, OPERATION_TIME_LOG || INSTENT_TEST_LOG, "AverageVideoEncoding Time = " + m_Tools.DoubleToString(m_pCalculatorEncodeTime->GetAverage()));
					//            CLogPrinter_WriteLog(CLogPrinter::INFO, OPERATION_TIME_LOG || INSTENT_TEST_LOG, "VideoEncoding Time = " + m_Tools.LongLongtoStringConvert(m_Tools.CurrentTimestamp() - llCalculatingTime));

					m_pBitRateController->NotifyEncodedFrame(nENCODEDFrameSize);

					if (m_pVideoCallSession->isDynamicIDR_Mechanism_Enable())
					{
						m_pIdrFrameIntervalController->NotifyEncodedFrame(m_ucaEncodedFrame, nENCODEDFrameSize, m_iFrameNumber);
					}

					//llCalculatingTime = CLogPrinter_WriteLog(CLogPrinter::INFO, OPERATION_TIME_LOG, "" ,true);


					if (m_FPS_TimeDiff == 0)
						m_FPS_TimeDiff = m_Tools.CurrentTimestamp();

					if (m_Tools.CurrentTimestamp() - m_FPS_TimeDiff < 1000)
					{
						m_FpsCounter++;
					}
					else
					{
						m_FPS_TimeDiff = m_Tools.CurrentTimestamp();

						//printf("Current Encoding FPS = %d\n", m_FpsCounter);
						if (m_FpsCounter >(m_nCallFPS - FPS_TOLERANCE_FOR_FPS))
						{
							//kaj korte hobe
						}
						m_FpsCounter = 0;
					}

					CLogPrinter_WriteLog(CLogPrinter::INFO, THREAD_LOG, "CVideoEncodingThreadOfCall::EncodingThreadProcedure() Sending for packetization nENCODEDFrameSize " + m_Tools.getText(nENCODEDFrameSize));
				}


				//if (nENCODEDFrameSize > 0)
			{

				if (m_bSelfViewOnly == false)
				{
					m_pEncodedFramePacketizer->Packetize(m_llFriendID, m_ucaEncodedFrame, nENCODEDFrameSize, m_iFrameNumber, nCaptureTimeDifference, nDevice_orientation, VIDEO_DATA_MOOD, 0, 0);
				}

				//CLogPrinter_WriteLog(CLogPrinter::INFO, OPERATION_TIME_LOG, " Packetize ",true, llCalculatingTime);

				++m_iFrameNumber;
			}

			if (countNumber == 1000)
			{
				countNumber = 1;
				sum = 0;
			}

			sum += (int)(m_Tools.CurrentTimestamp() - startTime);

#ifdef __ANDROID__

			LOGSS("Average %d %d\n", sum / countNumber, countNumber);

#endif

			countNumber++;

			toolsObject.SOSleep(0);
			}
		}

		bEncodingThreadClosed = true;

		CLogPrinter_WriteLog(CLogPrinter::INFO, THREAD_LOG, "CVideoEncodingThreadOfCall::EncodingThreadProcedure() stopped EncodingThreadProcedure method.");
	}

	void CVideoEncodingThreadOfCall::TestVideoEffect(int *param, int size)
	{
		m_VideoBeautificationer->TestVideoEffect(param, size);
	}



} //namespace MediaSDK

