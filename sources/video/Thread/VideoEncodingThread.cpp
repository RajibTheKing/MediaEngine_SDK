#include "VideoCallSession.h"
#include "VideoEncodingThread.h"

#include "LogPrinter.h"
#include "VideoCallSession.h"
#include "CommonElementsBucket.h"
#include "InterfaceOfAudioVideoEngine.h"

#if defined(TARGET_OS_IPHONE) || defined(TARGET_IPHONE_SIMULATOR)
#include <dispatch/dispatch.h>
#endif

namespace MediaSDK
{

CVideoEncodingThread::CVideoEncodingThread(long long llFriendID, CEncodingBuffer *pEncodingBuffer, CCommonElementsBucket *commonElementsBucket, BitRateController *pBitRateController, IDRFrameIntervalController *pIdrFrameController, CColorConverter *pColorConverter, CVideoEncoder *pVideoEncoder, CEncodedFramePacketizer *pEncodedFramePacketizer, CVideoCallSession *pVideoCallSession, int nFPS, bool bIsCheckCall, bool bSelfViewOnly) :

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
m_bResetForViewerCallerCallEnd(false),
m_bVideoEffectEnabled(true),
m_bSelfViewOnly(bSelfViewOnly),
m_VideoBeautificationer(NULL),
m_bNewSessionStarted(true)
{
    m_pCalculatorEncodeTime = new CAverageCalculator();
    m_pCalculateEncodingTimeDiff = new CAverageCalculator();

	//m_VideoBeautificationer = new CVideoBeautificationer(m_pVideoCallSession->m_nVideoCallHeight, m_pVideoCallSession->m_nVideoCallWidth);
	//m_VideoBeautificationer->GenerateUVIndex(this->m_pColorConverter->GetHeight(), this->m_pColorConverter->GetWidth(), 11);

	m_VideoEffects = new CVideoEffects();

	m_EncodingFailedCounter = 0;
    
    m_pVideoCallSession = pVideoCallSession;
    m_bIsCheckCall = bIsCheckCall;
    
	if (m_bIsCheckCall == DEVICE_ABILITY_CHECK_MOOD)
     {
         for(int k=0;k<3;k++)
         {
             memset(m_ucaDummmyFrame[k], 0, sizeof(m_ucaDummmyFrame[k]));
             
             for(int i=0;i<m_pVideoCallSession->getVideoCallHeight();i++)
             {
                 int color = rand()%255;
                 for(int j = 0; j <  m_pVideoCallSession->getVideoCallWidth(); j ++)
                 {
                     m_ucaDummmyFrame[k][i * m_pVideoCallSession->getVideoCallHeight() + j ] = color;
                 }
                 
             }
         }  
     }

	m_filterToApply = 0;
    m_iNumberOfEncodeFailPerFPS = 0;

	//pFile = fopen("/sdcard/encodingThreadEncodedFile.h264", "w");
}

CVideoEncodingThread::~CVideoEncodingThread()
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

	//fclose(pFile);
}

void CVideoEncodingThread::SetCallFPS(int nFPS)
{
	m_nCallFPS = nFPS;
}

void CVideoEncodingThread::ResetVideoEncodingThread(BitRateController *pBitRateController)
{
    m_iFrameNumber = 0;
    m_pBitRateController = pBitRateController; 
}

void CVideoEncodingThread::StopEncodingThread()
{
	CLogPrinter_WriteLog(CLogPrinter::INFO, THREAD_LOG, "CVideoEncodingThread::StopEncodingThread called");

	//if (pInternalThread.get())
	{

		bEncodingThreadRunning = false;

		while (!bEncodingThreadClosed)
		{
			m_Tools.SOSleep(5);
		}
	}

	//pInternalThread.reset();

	CLogPrinter_WriteLog(CLogPrinter::INFO, THREAD_LOG, "CVideoEncodingThread::StopEncodingThread Encoding Thread STOPPED");
}

void CVideoEncodingThread::StartEncodingThread()
{
	CLogPrinter_WriteLog(CLogPrinter::INFO, THREAD_LOG ,"CVideoEncodingThread::StartEncodingThread called");

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

	CLogPrinter_WriteLog(CLogPrinter::INFO, THREAD_LOG ,"CVideoEncodingThread::StartEncodingThread Encoding Thread started");

	return;
}

void *CVideoEncodingThread::CreateVideoEncodingThread(void* param)
{
	CVideoEncodingThread *pThis = (CVideoEncodingThread*)param;
	pThis->EncodingThreadProcedure();

	return NULL;
}

void CVideoEncodingThread::ResetForViewerCallerCallEnd()
{
	m_bResetForViewerCallerCallEnd = true;

	while (m_bResetForViewerCallerCallEnd)
	{
		m_Tools.SOSleep(5);
	}
}

void CVideoEncodingThread::ResetForPublisherCallerInAudioOnly()
{
	m_ResetForPublisherCallerInAudioOnly = true;

	while (m_ResetForPublisherCallerInAudioOnly)
	{
		m_Tools.SOSleep(5);
	}
}

void CVideoEncodingThread::SetOrientationType(int nOrientationType)
{
	m_nOrientationType = nOrientationType;
}

bool CVideoEncodingThread::IsThreadStarted()
{
	return m_bIsThisThreadStarted;
}

void CVideoEncodingThread::SetNotifierFlag(bool flag)
{
	m_bNotifyToClientVideoQuality = flag;
}

void CVideoEncodingThread::SetFrameNumber(int nFrameNumber)
{
    m_iFrameNumber = nFrameNumber;
}

int CVideoEncodingThread::SetVideoEffect(int nEffectStatus)
{
	if (nEffectStatus != 0)
		m_bVideoEffectEnabled = true;
	else if (nEffectStatus == 0)
		m_bVideoEffectEnabled = false;

	m_filterToApply = nEffectStatus;

	return 1;
}


long long g_PrevEncodeTime = 0;

void CVideoEncodingThread::EncodingThreadProcedure()
{
	CLogPrinter_WriteLog(CLogPrinter::INFO, THREAD_LOG ,"CVideoEncodingThread::EncodingThreadProcedure() started EncodingThreadProcedure method");

	Tools toolsObject;
    toolsObject.SetThreadName("EncodingCommon");
	int nEncodingFrameSize, nENCODEDFrameSize = 0, nCaptureTimeDifference, nDevice_orientation;
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
	int dummyTimeStampCounter = 0;

	bool bNeedIDR = false;

	int iGotHeight;
	int iGotWidth;
    
    int iNumberOfEncodeFailCounter = 0;
#if defined(DESKTOP_C_SHARP)

	MakeBlackScreen(m_ucaDummmyStillFrame,  m_pVideoCallSession->getVideoCallHeight(),  m_pVideoCallSession->getVideoCallWidth(), RGB24);

#else

	MakeBlackScreen(m_ucaDummmyStillFrame,  m_pVideoCallSession->getVideoCallHeight(),  m_pVideoCallSession->getVideoCallWidth(), YUV420);

#endif

	/*for(int i = 0; i < 200; i++)
	{
		if (m_pBitRateController->IsNetworkTypeMiniPacketReceived())
		{
			CLogPrinter_WriteSpecific5(CLogPrinter::INFO, "CVideoEncodingThread::EncodingThreadProcedure() m_pBitRateController->m_iNetworkType after waiting = " + toolsObject.IntegertoStringConvert(m_pBitRateController->GetOpponentNetworkType()));
			break;
		}

		toolsObject.SOSleep(10);
	}*/
	if (m_bIsCheckCall && (m_pVideoCallSession->GetServiceType() == SERVICE_TYPE_CALL || m_pVideoCallSession->GetServiceType() == SERVICE_TYPE_SELF_CALL)) 
	{
		m_pBitRateController->SetInitialBitrate();
		bIsBitrateInitialized = true;
	}

	while (bEncodingThreadRunning)
	{
		//CLogPrinter_WriteLog(CLogPrinter::INFO, THREAD_LOG ,"CVideoEncodingThread::EncodingThreadProcedure() RUNNING EncodingThreadProcedure method");

		bNeedIDR = false;

		if (m_bResetForViewerCallerCallEnd == true)
		{
			m_pEncodingBuffer->ResetBuffer();

			m_iFrameNumber = m_nCallFPS;
			bNeedIDR = true;

			m_bResetForViewerCallerCallEnd = false;
		}

		if (m_ResetForPublisherCallerInAudioOnly == true)
		{
			dummyTimeStampCounter = 0;

			m_ResetForPublisherCallerInAudioOnly = false;
		}

		m_bIsThisThreadStarted = true;

		if (m_bNotifyToClientVideoQuality == true && m_bIsCheckCall == false)
		{
			m_bNotifyToClientVideoQuality = false;

			if (m_pVideoCallSession->GetCurrentVideoCallQualityLevel() == RESOLUTION_FPS_SUPPORT_NOT_TESTED || m_pVideoCallSession->GetCurrentVideoCallQualityLevel() == SUPPORTED_RESOLUTION_FPS_352_15)
				m_pCommonElementBucket->m_pEventNotifier->fireVideoNotificationEvent(m_pVideoCallSession->GetFriendID(), m_pCommonElementBucket->m_pEventNotifier->SET_CAMERA_RESOLUTION_352x288);
			else if (m_pVideoCallSession->GetCurrentVideoCallQualityLevel() == SUPPORTED_RESOLUTION_FPS_352_25)
			{

#if defined(TARGET_OS_IPHONE) || defined(TARGET_IPHONE_SIMULATOR) || defined(__ANDROID__)

				m_pCommonElementBucket->m_pEventNotifier->fireVideoNotificationEvent(m_pVideoCallSession->GetFriendID(), m_pCommonElementBucket->m_pEventNotifier->SET_CAMERA_RESOLUTION_640x480);

#else
				m_pCommonElementBucket->m_pEventNotifier->fireVideoNotificationEvent(m_pVideoCallSession->GetFriendID(), m_pCommonElementBucket->m_pEventNotifier->SET_CAMERA_RESOLUTION_352x288);
#endif
			}
			else if (m_pVideoCallSession->GetCurrentVideoCallQualityLevel() == SUPPORTED_RESOLUTION_FPS_640_25)
				m_pCommonElementBucket->m_pEventNotifier->fireVideoNotificationEvent(m_pVideoCallSession->GetFriendID(), m_pCommonElementBucket->m_pEventNotifier->SET_CAMERA_RESOLUTION_640x480);

			m_pVideoCallSession->m_bVideoCallStarted = true;
		}

        
        //printf("TheVersion--> CurrentCallVersion = %d\n", m_pVideoCallSession->GetVersionController()->GetCurrentCallVersion());

        if( m_pVideoCallSession->GetVersionController()->GetCurrentCallVersion() == -1  && m_bIsCheckCall == false)
        {
			m_pEncodedFramePacketizer->Packetize(m_llFriendID, m_ucaEncodedFrame, /*SIZE*/ 0, /*m_iFrameNumber*/0, /*nCaptureTimeDifference*/0, 0, BLANK_DATA_MOOD, 0, 0);

			CLogPrinter_WriteLog(CLogPrinter::INFO, THREAD_LOG, "CVideoEncodingThread::EncodingThreadProcedure() Negotiation uncomplete");

			CLogPrinter_LOG(VIDEO_CALL_NEG_LOG, "CVideoEncodingThread::EncodingThreadProcedure Sending negotiation m_bIsCheckCall %d GetCurrentCallVersion %d", m_bIsCheckCall, m_pVideoCallSession->GetVersionController()->GetCurrentCallVersion());

            toolsObject.SOSleep(20);
            continue;
        }
        
		if (m_pEncodingBuffer->GetQueueSize() == 0)
		{
			CLogPrinter_WriteLog(CLogPrinter::INFO, THREAD_LOG, "CVideoEncodingThread::EncodingThreadProcedure() got NOTHING for encoding");

//			CLogPrinter_WriteLog(CLogPrinter::INFO, INSTENT_TEST_LOG, " fahad Encode time buffer size 0");
			if( !m_pVideoCallSession->GetVersionController()->IsFirstVideoPacetReceived() && m_bIsCheckCall == false) {
//			toolsObject.SOSleep(10000);
//				VLOG("--------------------------------------------------------------> NOT RECEIVED");
				m_pEncodedFramePacketizer->Packetize(m_llFriendID, m_ucaEncodedFrame,
													 2, /*m_iFrameNumber*/
													 0, /*nCaptureTimeDifference*/0, 0,
													 BLANK_DATA_MOOD, 0, 0);

			}
			
			if (m_pVideoCallSession->GetEntityType() == ENTITY_TYPE_PUBLISHER_CALLER && m_pVideoCallSession->GetAudioOnlyLiveStatus() == true && (m_pVideoCallSession->GetCallInLiveType() == CALL_IN_LIVE_TYPE_AUDIO_VIDEO || m_pVideoCallSession->GetCallInLiveType() == CALL_IN_LIVE_TYPE_VIDEO_ONLY))
			{
				dummyTimeStampCounter++;

				if (dummyTimeStampCounter % 4 == 0)
				{

#if defined(DESKTOP_C_SHARP)

					m_pEncodingBuffer->Queue(m_ucaDummmyStillFrame,  m_pVideoCallSession->m_nVideoCallWidth *  m_pVideoCallSession->m_nVideoCallHeight * 3,  m_pVideoCallSession->m_nVideoCallHeight, m_pVideoCallSession->m_nVideoCallWidth,  dummyTimeStampCounter * 10, 0);
#else
					m_pEncodingBuffer->Queue(m_ucaDummmyStillFrame,  m_pVideoCallSession->m_nVideoCallWidth *  m_pVideoCallSession->m_nVideoCallHeight * 3 / 2,  m_pVideoCallSession->m_nVideoCallHeight, m_pVideoCallSession->m_nVideoCallWidth, dummyTimeStampCounter * 10, 0);
#endif
				}
			}
			
			toolsObject.SOSleep(10);
		}
		else
		{
            CLogPrinter_WriteLog(CLogPrinter::INFO, THREAD_LOG ,"CVideoEncodingThread::EncodingThreadProcedure() GOT packet for Encoding");
			int timeDiff;

			if (!bIsBitrateInitialized && (m_pVideoCallSession->GetServiceType() == SERVICE_TYPE_CALL || m_pVideoCallSession->GetServiceType() == SERVICE_TYPE_SELF_CALL))
			{
				m_pBitRateController->SetInitialBitrate();
				bIsBitrateInitialized = true;
			}

			//CLogPrinter_WriteLog(CLogPrinter::INFO, INSTENT_TEST_LOG, " fahad Encode time ");

			nEncodingFrameSize = m_pEncodingBuffer->DeQueue(m_ucaEncodingFrame, iGotHeight, iGotWidth, timeDiff, nCaptureTimeDifference, nDevice_orientation);
			/*
			if (m_iFrameNumber % 3 == 0)
			{
				m_iFrameNumber++;

				continue;
			}
			*/

			//LOGEF("Current bitrate %d", m_pVideoEncoder->GetBitrate());
            
            if(g_PrevEncodeTime!=0)
                m_pCalculateEncodingTimeDiff->UpdateData(m_Tools.CurrentTimestamp() - g_PrevEncodeTime);
            
            //printf("TheVampireEngg --> EncodingTime Diff = %lld, Average = %lf\n", m_Tools.CurrentTimestamp() - g_PrevEncodeTime, m_CalculateEncodingTimeDiff.GetAverage());
            g_PrevEncodeTime = m_Tools.CurrentTimestamp();

			long long startTime = m_Tools.CurrentTimestamp();
            
            
			CLogPrinter_WriteLog(CLogPrinter::INFO, QUEUE_TIME_LOG ," &*&*&* m_pEncodingBuffer ->" + toolsObject.IntegertoStringConvert(timeDiff));

			if (! m_pVideoCallSession->GetFPSController()->IsProcessableFrame() && m_bIsCheckCall == false)
			{
				CLogPrinter_WriteLog(CLogPrinter::INFO, THREAD_LOG ,"CVideoEncodingThread::EncodingThreadProcedure() not processable for FPS");

				toolsObject.SOSleep(10);

				continue;
			}
//			CLogPrinter_WriteLog(CLogPrinter::INFO, INSTENT_TEST_LOG ," Client FPS: " + Tools::DoubleToString(m_pVideoCallSession->GetFPSController()->GetClientFPS()));

			if (m_pVideoCallSession->GetServiceType() == SERVICE_TYPE_CALL || m_pVideoCallSession->GetServiceType() == SERVICE_TYPE_SELF_CALL)
			{
				m_pBitRateController->UpdateBitrate();
			}

			llCalculatingTime = CLogPrinter_WriteLog(CLogPrinter::INFO, OPERATION_TIME_LOG);

#if defined(TARGET_OS_IPHONE) || defined(TARGET_IPHONE_SIMULATOR)
            
            long long convertStartTime = m_Tools.CurrentTimestamp();
			this->m_pColorConverter->ConvertNV12ToI420(m_ucaEncodingFrame, iGotHeight, iGotWidth);
            //printf("TheKing--> convertTimeDiff = %lld\n", m_Tools.CurrentTimestamp() - convertStartTime);

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
			if ((nServiceType == SERVICE_TYPE_LIVE_STREAM || nServiceType == SERVICE_TYPE_SELF_STREAM || nServiceType == SERVICE_TYPE_CHANNEL) && (m_pVideoCallSession->GetOwnDeviceType() != DEVICE_TYPE_DESKTOP || (m_nOrientationType == ORIENTATION_90_MIRRORED && iGotWidth < iGotHeight)))
			{
				int iChangedGotHeight, iChangedGotWidth;
#if defined(TARGET_OS_IPHONE) || defined(TARGET_IPHONE_SIMULATOR)
				nEncodingFrameSize =  m_pColorConverter->CropWithAspectRatio_YUVNV12_YUVNV21_RGB24(m_ucaEncodingFrame, iGotHeight, iGotWidth, DEVICE_SCREEN_HEIGHT, DEVICE_SCREEN_WIDTH, m_ucaCropedFrame, iChangedGotHeight, iChangedGotWidth, YUVYV12);
            	memcpy(m_ucaEncodingFrame, m_ucaCropedFrame, nEncodingFrameSize);
#else

				nEncodingFrameSize = m_pColorConverter->CropWithAspectRatio_YUVNV12_YUVNV21_RGB24(m_ucaConvertedEncodingFrame, iGotHeight, iGotWidth, DEVICE_SCREEN_HEIGHT, DEVICE_SCREEN_WIDTH, m_ucaCropedFrame, iChangedGotHeight, iChangedGotWidth, YUVYV12);
				memcpy(m_ucaConvertedEncodingFrame, m_ucaCropedFrame, nEncodingFrameSize);
#endif
				iGotHeight = iChangedGotHeight;
				iGotWidth = iChangedGotWidth;
			}
            else
			{
				/**Do Nothing**/
			}

            
			if (m_VideoBeautificationer == NULL)
			{
				m_VideoBeautificationer = new CVideoBeautificationer(iGotHeight, iGotWidth, CHANNEL_TYPE_NOT_CHANNEL);
			}
            
            if (m_pVideoCallSession->GetEntityType() == ENTITY_TYPE_PUBLISHER_CALLER && m_pVideoCallSession->GetAudioOnlyLiveStatus() == true && (m_pVideoCallSession->GetCallInLiveType() == CALL_IN_LIVE_TYPE_AUDIO_VIDEO || m_pVideoCallSession->GetCallInLiveType() == CALL_IN_LIVE_TYPE_VIDEO_ONLY))
			{
				MakeBlackScreen(m_ucaConvertedEncodingFrame, iGotHeight, iGotWidth, YUV420);
			}
			
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

			int iSmallWidth = m_pColorConverter->GetSmallFrameWidth();
			int iSmallHeight = m_pColorConverter->GetSmallFrameHeight();

			int nCallScreenSplitType = m_pVideoCallSession->GetScreenSplitType();

			if (m_pVideoCallSession->GetServiceType() == SERVICE_TYPE_LIVE_STREAM || m_pVideoCallSession->GetServiceType() == SERVICE_TYPE_SELF_STREAM || m_pVideoCallSession->GetServiceType() == SERVICE_TYPE_CHANNEL)
			{
				int iWidth = iGotWidth;
				int iHeight = iGotHeight;

				int newHeight;
				int newWidth;

				m_pColorConverter->CalculateAspectRatioWithScreenAndModifyHeightWidth(iHeight, iWidth, DEVICE_SCREEN_HEIGHT, DEVICE_SCREEN_WIDTH, newHeight, newWidth);

				if (m_bVideoEffectEnabled == true && !(m_pVideoCallSession->GetEntityType() == ENTITY_TYPE_PUBLISHER_CALLER && m_pVideoCallSession->GetAudioOnlyLiveStatus() == true))
				{

#if defined(TARGET_OS_IPHONE) || defined(TARGET_IPHONE_SIMULATOR)

					if (m_pVideoCallSession->GetOwnVideoCallQualityLevel() != SUPPORTED_RESOLUTION_FPS_352_15)
					{
						pair<int, int> resultPair = m_VideoBeautificationer->BeautificationFilterNew(m_ucaEncodingFrame, nEncodingFrameSize, iGotHeight, iGotWidth, newHeight, newWidth, true);
					
					}
					else
					{
						pair<int, int> resultPair = m_VideoBeautificationer->BeautificationFilter2(m_ucaConvertedEncodingFrame, nEncodingFrameSize, iGotHeight, iGotWidth);

						//m_VideoBeautificationer->MakeFrameBlurAndStore(m_ucaConvertedEncodingFrame, iHeight, iWidth);
						//m_VideoBeautificationer->MakeFrameBeautiful(m_ucaConvertedEncodingFrame);
					}
#else
					

					//LOGE("setVideoEffect -------------->>             m_VideoEffectParam[0] = %d, m_VideoEffectParam[1] = %d,  m_VideoEffectParam[2] = %d", m_VideoEffectParam[0], m_VideoEffectParam[1], m_VideoEffectParam[2]);

#ifndef TARGET_OS_WINDOWS_PHONE

					if (m_pVideoCallSession->GetOwnVideoCallQualityLevel() != SUPPORTED_RESOLUTION_FPS_352_15 || m_pVideoCallSession->GetOwnDeviceType() == DEVICE_TYPE_DESKTOP)
					{
						if (m_pVideoCallSession->GetOwnDeviceType() == DEVICE_TYPE_DESKTOP)
						{
							pair<int, int> resultPair = m_VideoBeautificationer->BeautificationFilter(m_ucaConvertedEncodingFrame, nEncodingFrameSize, iGotHeight, iGotWidth, iGotHeight, iGotWidth, true);
						}
						else
						{
						//	if (13 == m_filterToApply)
								pair<int, int> resultPair = m_VideoBeautificationer->BeautificationFilterNew(m_ucaConvertedEncodingFrame, nEncodingFrameSize, iGotHeight, iGotWidth, newHeight, newWidth, true);
						//	else
						//		pair<int, int> resultPair = m_VideoBeautificationer->BeautificationFilter(m_ucaConvertedEncodingFrame, nEncodingFrameSize, iGotHeight, iGotWidth, newHeight, newWidth, true);

						}
					}
					else
					{
						pair<int, int> resultPair = m_VideoBeautificationer->BeautificationFilter2(m_ucaConvertedEncodingFrame, nEncodingFrameSize, iGotHeight, iGotWidth);

						//m_VideoBeautificationer->MakeFrameBlurAndStore(m_ucaConvertedEncodingFrame, iHeight, iWidth);
						//m_VideoBeautificationer->MakeFrameBeautiful(m_ucaConvertedEncodingFrame);
					}
#endif
					//m_pCommonElementBucket->m_pEventNotifier->fireVideoNotificationEvent(resultPair.first, resultPair.second);
#endif
				}

				if (m_nOrientationType == ORIENTATION_90_MIRRORED)
				{
					//CLogPrinter_WriteLog(CLogPrinter::INFO, INSTENT_TEST_LOG_2, "CVideoEncodingThread::EncodingThreadProcedure 1");

#if defined(TARGET_OS_IPHONE) || defined(TARGET_IPHONE_SIMULATOR)

					this->m_pColorConverter->mirrorYUVI420(m_ucaEncodingFrame, m_ucaMirroredFrame, iHeight, iWidth);
#else
					this->m_pColorConverter->mirrorYUVI420(m_ucaConvertedEncodingFrame, m_ucaMirroredFrame, iHeight, iWidth);
#endif
				}
				else
				{
					//CLogPrinter_WriteLog(CLogPrinter::INFO, INSTENT_TEST_LOG_2, "CVideoEncodingThread::EncodingThreadProcedure 2");

#if defined(TARGET_OS_IPHONE) || defined(TARGET_IPHONE_SIMULATOR)

					memcpy(m_ucaMirroredFrame, m_ucaEncodingFrame, (iWidth*iHeight*3) /2);
#else
					memcpy(m_ucaMirroredFrame, m_ucaConvertedEncodingFrame, (iWidth*iHeight*3) /2);
#endif
				}

				if (m_pVideoCallSession->GetEntityType() == ENTITY_TYPE_VIEWER_CALLEE)
				{
					CLogPrinter_WriteLog(CLogPrinter::INFO, INSTENT_TEST_LOG_2, "CVideoEncodingThread::EncodingThreadProcedure() SetSmallFrame iHeight " + m_Tools.getText(iHeight) + " iWidth " + m_Tools.getText(iWidth));
                    int iOpponentVideoHeight = m_pVideoCallSession->GetOpponentVideoHeight();
                    int iOpponentVideoWidth = m_pVideoCallSession->GetOpponentVideoWidth();
                    int iOpponentLibraryVersion = m_pVideoCallSession->getOpponentLibraryVersion();
                    
                    if(iOpponentVideoHeight !=-1 && iOpponentVideoWidth !=  -1)
                    {
                        if(m_pVideoCallSession->GetOwnDeviceType() != DEVICE_TYPE_DESKTOP && m_pVideoCallSession->GetOponentDeviceType() != DEVICE_TYPE_DESKTOP)
                        {
                            int rotatedHeight, rotatedWidth;
                            m_pVideoCallSession->GetColorConverter()->RotateI420(m_ucaMirroredFrame, iHeight, iWidth, m_ucaRotatedFrame, rotatedHeight, rotatedWidth, nDevice_orientation);
                            m_pVideoCallSession->GetColorConverter()->SetSmallFrame(m_ucaRotatedFrame, rotatedHeight, rotatedWidth, nEncodingFrameSize, iOpponentVideoHeight, iOpponentVideoWidth, m_pVideoCallSession->GetOponentDeviceType() != DEVICE_TYPE_DESKTOP, min(iOpponentLibraryVersion, LIBRARY_VERSION));
                        }
                        else
                        {
                            m_pVideoCallSession->GetColorConverter()->SetSmallFrame(m_ucaMirroredFrame, iHeight, iWidth, nEncodingFrameSize, iOpponentVideoHeight, iOpponentVideoWidth, m_pVideoCallSession->GetOponentDeviceType() != DEVICE_TYPE_DESKTOP, min(iOpponentLibraryVersion, LIBRARY_VERSION));
                        }
                        
                    }
				}

				CLogPrinter_LOG(LIVE_INSET_LOG, "LIVE_INSET_LOG CVideoEncodingThread::EncodingThreadProcedure 001 GetEntityType %d, GetSmallFrameStatus %d, GetScreenSplitType %d GetCallInLiveType %d", m_pVideoCallSession->GetEntityType(), m_pColorConverter->GetSmallFrameStatus(), nCallScreenSplitType, m_pVideoCallSession->GetCallInLiveType());

				if (m_pVideoCallSession->GetEntityType() == ENTITY_TYPE_PUBLISHER_CALLER && (m_pColorConverter->GetSmallFrameStatus() == true || (nCallScreenSplitType == LIVE_CALL_SCREEN_SPLIT_TYPE_DIVIDE && m_pVideoCallSession->GetCallInLiveType() == CALL_IN_LIVE_TYPE_AUDIO_ONLY)))
				{
					int iInsetLowerPadding = (int)((iGotHeight * 10) / 100);

					iSmallWidth = m_pColorConverter->GetSmallFrameWidth();
					iSmallHeight = m_pColorConverter->GetSmallFrameHeight();

					int nSmallFrameHeight;
					int nSmallFrameWidth;

					this->m_pColorConverter->GetSmallFrame(m_ucaOpponentSmallFrame, nSmallFrameHeight, nSmallFrameWidth);

					CLogPrinter_LOG(LIVE_INSET_LOG, "LIVE_INSET_LOG CVideoEncodingThread::EncodingThreadProcedure 000 iInsetLowerPadding %d, iSmallWidth %d, iSmallHeight %d", iInsetLowerPadding, iSmallWidth, iSmallHeight);

					int iPosX = iWidth - iSmallWidth;
					int iPosY = iHeight - iSmallHeight - iInsetLowerPadding;

					int upperOffset = SPLIT_TYPE_DEVIDE_UPPER_OFFSET;

					bool audioOnlyCall = m_pVideoCallSession->GetCallInLiveType() == CALL_IN_LIVE_TYPE_AUDIO_ONLY;

					CLogPrinter_LOG(LIVE_INSET_LOG, "LIVE_INSET_LOG CVideoEncodingThread::EncodingThreadProcedure 1 iPosX %d, iPosY %d audioOnlyCall %d GetCallInLiveType %d", iPosX, iPosY, audioOnlyCall, m_pVideoCallSession->GetCallInLiveType());

					CLogPrinter_WriteLog(CLogPrinter::INFO, INSTENT_TEST_LOG_2, "CVideoEncodingThread::EncodingThreadProcedure() Merge_Two_Video iHeight " + m_Tools.getText(iHeight) + " iWidth " + m_Tools.getText(iWidth));

					CLogPrinter_LOG(CO_HOST_CALL_LOG, "CVideoEncodingThread::EncodingThreadProcedure() nScreenSplitType %d", nCallScreenSplitType);

					if (nCallScreenSplitType == LIVE_CALL_SCREEN_SPLIT_TYPE_DIVIDE)
					{
						if ((m_pVideoCallSession->GetOwnDeviceType() == DEVICE_TYPE_DESKTOP && iGotWidth > iGotHeight) && (nSmallFrameHeight > nSmallFrameWidth))
						{
							CLogPrinter_LOG(CO_HOST_CALL_LOG, "CVideoEncodingThread::EncodingThreadProcedure() 1 nScreenSplitType %d", nCallScreenSplitType);

#if defined(TARGET_OS_IPHONE) || defined(TARGET_IPHONE_SIMULATOR)

							this->m_pColorConverter->DownScaleYUV420_Dynamic_Version222(m_ucaEncodingFrame, iHeight, iWidth, m_ucaTempFrame, SPLIT_TYPE_DEVICE_DESKTOP_HEIGHT, SPLIT_TYPE_DEVICE_DESKTOP_WIDTH);
#else
							this->m_pColorConverter->DownScaleYUV420_Dynamic_Version222(m_ucaConvertedEncodingFrame, iHeight, iWidth, m_ucaTempFrame, SPLIT_TYPE_DEVICE_DESKTOP_HEIGHT, SPLIT_TYPE_DEVICE_DESKTOP_WIDTH);
#endif
							this->m_pColorConverter->DownScaleYUV420_Dynamic_Version222(m_ucaMirroredFrame, iHeight, iWidth, m_ucaTempFrame1, SPLIT_TYPE_DEVICE_DESKTOP_HEIGHT, SPLIT_TYPE_DEVICE_DESKTOP_WIDTH);
							
							if(m_pVideoCallSession->GetCallInLiveType() != CALL_IN_LIVE_TYPE_AUDIO_ONLY)
								this->m_pColorConverter->DownScaleYUV420_Dynamic_Version222(m_ucaOpponentSmallFrame, nSmallFrameHeight, nSmallFrameWidth, m_ucaTempFrame2, SPLIT_TYPE_DEVICE_DESKTOP_HEIGHT, SPLIT_TYPE_DEVICE_DESKTOP_MOBILE_WIDTH);
						}
						else
						{
							CLogPrinter_LOG(CO_HOST_CALL_LOG, "CVideoEncodingThread::EncodingThreadProcedure() 2 nScreenSplitType %d", nCallScreenSplitType);

#if defined(TARGET_OS_IPHONE) || defined(TARGET_IPHONE_SIMULATOR)

							this->m_pColorConverter->DownScaleYUV420_Dynamic_Version222(m_ucaEncodingFrame, iHeight, iWidth, m_ucaTempFrame, iHeight / 2, iWidth / 2);
#else
							this->m_pColorConverter->DownScaleYUV420_Dynamic_Version222(m_ucaConvertedEncodingFrame, iHeight, iWidth, m_ucaTempFrame, iHeight / 2, iWidth / 2);
#endif
							this->m_pColorConverter->DownScaleYUV420_Dynamic_Version222(m_ucaMirroredFrame, iHeight, iWidth, m_ucaTempFrame1, iHeight / 2, iWidth / 2);
							
							if (m_pVideoCallSession->GetCallInLiveType() != CALL_IN_LIVE_TYPE_AUDIO_ONLY)
								this->m_pColorConverter->DownScaleYUV420_Dynamic_Version222(m_ucaOpponentSmallFrame, nSmallFrameHeight, nSmallFrameWidth, m_ucaTempFrame2, iHeight / 2, iWidth / 2);
						}
					}	
					
					if (m_pVideoCallSession->GetOwnDeviceType() != DEVICE_TYPE_DESKTOP)
					{
						CLogPrinter_LOG(LIVE_INSET_LOG, "CVideoEncodingThread::EncodingThreadProcedure() zzz iGotWidth %d iGotHeight %d nSmallFrameHeight %d nSmallFrameWidth %d", iGotWidth, iGotHeight, nSmallFrameHeight, nSmallFrameWidth);

						m_pColorConverter->GetInsetLocation(iHeight, iWidth, iPosX, iPosY);
					}

					if (nCallScreenSplitType == LIVE_CALL_SCREEN_SPLIT_TYPE_DIVIDE)
					{
						CLogPrinter_LOG(LIVE_INSET_LOG, "CVideoEncodingThread::EncodingThreadProcedure() qqq iGotWidth %d iGotHeight %d nSmallFrameHeight %d nSmallFrameWidth %d", iGotWidth, iGotHeight, nSmallFrameHeight, nSmallFrameWidth);

						if ((m_pVideoCallSession->GetOwnDeviceType() == DEVICE_TYPE_DESKTOP && iGotWidth > iGotHeight) && (nSmallFrameHeight > nSmallFrameWidth))
						{
							CLogPrinter_LOG(LIVE_INSET_LOG, "CVideoEncodingThread::EncodingThreadProcedure() www iGotWidth %d iGotHeight %d nSmallFrameHeight %d nSmallFrameWidth %d", iGotWidth, iGotHeight, nSmallFrameHeight, nSmallFrameWidth);

							this->m_pColorConverter->Merge_Two_Video2(audioOnlyCall, m_ucaMirroredFrame, 0, 0, iHeight, iWidth, m_ucaTempFrame1, SPLIT_TYPE_DEVICE_DESKTOP_HEIGHT, SPLIT_TYPE_DEVICE_DESKTOP_WIDTH);

							if(audioOnlyCall)
								this->m_pColorConverter->Merge_Two_Video3(audioOnlyCall, m_ucaMirroredFrame, SPLIT_TYPE_DEVICE_DESKTOP_WIDTH, 0, iHeight, iWidth, SPLIT_TYPE_DEVICE_DESKTOP_HEIGHT, SPLIT_TYPE_DEVICE_DESKTOP_MOBILE_WIDTH);
							else
								this->m_pColorConverter->Merge_Two_Video2(audioOnlyCall, m_ucaMirroredFrame, SPLIT_TYPE_DEVICE_DESKTOP_WIDTH, 0, iHeight, iWidth, m_ucaTempFrame2, SPLIT_TYPE_DEVICE_DESKTOP_HEIGHT, SPLIT_TYPE_DEVICE_DESKTOP_MOBILE_WIDTH);

							this->m_pColorConverter->Merge_Two_VideoI420(audioOnlyCall, m_ucaMirroredFrame, 0, SPLIT_TYPE_DEVICE_DESKTOP_HEIGHT, iHeight, iWidth, iHeight - SPLIT_TYPE_DEVICE_DESKTOP_HEIGHT, iWidth);
						}
						else
						{
							CLogPrinter_LOG(LIVE_INSET_LOG, "CVideoEncodingThread::EncodingThreadProcedure() eee iGotWidth %d iGotHeight %d nSmallFrameHeight %d nSmallFrameWidth %d", iGotWidth, iGotHeight, nSmallFrameHeight, nSmallFrameWidth);

							this->m_pColorConverter->Merge_Two_VideoI420(audioOnlyCall, m_ucaMirroredFrame, 0, 0, iHeight, iWidth, upperOffset, iWidth);
							this->m_pColorConverter->Merge_Two_Video2(audioOnlyCall, m_ucaMirroredFrame, 0, upperOffset, iHeight, iWidth, m_ucaTempFrame1, iHeight / 2, iWidth / 2);

							if (audioOnlyCall)
								this->m_pColorConverter->Merge_Two_Video3(audioOnlyCall, m_ucaMirroredFrame, iWidth / 2, upperOffset, iHeight, iWidth, iHeight / 2, iWidth / 2);
							else
								this->m_pColorConverter->Merge_Two_Video2(audioOnlyCall, m_ucaMirroredFrame, iWidth / 2, upperOffset, iHeight, iWidth, m_ucaTempFrame2, iHeight / 2, iWidth / 2);

							this->m_pColorConverter->Merge_Two_VideoI420(audioOnlyCall, m_ucaMirroredFrame, 0, iHeight / 2 + upperOffset, iHeight, iWidth, iHeight / 2 - upperOffset, iWidth);
						}		
					}
					else
					{
						this->m_pColorConverter->Merge_Two_Video(m_ucaMirroredFrame, iPosX, iPosY, iHeight, iWidth);
					}

#if defined(TARGET_OS_IPHONE) || defined(TARGET_IPHONE_SIMULATOR)

					if (nCallScreenSplitType == LIVE_CALL_SCREEN_SPLIT_TYPE_DIVIDE)
					{
						this->m_pColorConverter->Merge_Two_VideoI420(audioOnlyCall, m_ucaEncodingFrame, 0, 0, iHeight, iWidth, upperOffset, iWidth);
						this->m_pColorConverter->Merge_Two_Video2(audioOnlyCall, m_ucaEncodingFrame, 0, upperOffset, iHeight, iWidth, m_ucaTempFrame, iHeight / 2, iWidth / 2);

						if (audioOnlyCall)
							this->m_pColorConverter->Merge_Two_Video3(audioOnlyCall, m_ucaEncodingFrame, iWidth / 2, upperOffset, iHeight, iWidth, iHeight / 2, iWidth / 2);
						else
							this->m_pColorConverter->Merge_Two_Video2(audioOnlyCall, m_ucaEncodingFrame, iWidth / 2, upperOffset, iHeight, iWidth, m_ucaTempFrame2, iHeight / 2, iWidth / 2);

						this->m_pColorConverter->Merge_Two_VideoI420(audioOnlyCall, m_ucaEncodingFrame, 0, iHeight / 2 + upperOffset, iHeight, iWidth, iHeight / 2 - upperOffset, iWidth);
					}
					else
					{
						this->m_pColorConverter->Merge_Two_Video(m_ucaEncodingFrame, iPosX, iPosY, iHeight, iWidth);
					}
#else
					if (nCallScreenSplitType == LIVE_CALL_SCREEN_SPLIT_TYPE_DIVIDE)
					{
						if ((m_pVideoCallSession->GetOwnDeviceType() == DEVICE_TYPE_DESKTOP && iGotWidth > iGotHeight) && (nSmallFrameHeight > nSmallFrameWidth))
						{
							this->m_pColorConverter->Merge_Two_Video2(audioOnlyCall, m_ucaConvertedEncodingFrame, 0, 0, iHeight, iWidth, m_ucaTempFrame, SPLIT_TYPE_DEVICE_DESKTOP_HEIGHT, SPLIT_TYPE_DEVICE_DESKTOP_WIDTH);

							if (audioOnlyCall)
								this->m_pColorConverter->Merge_Two_Video3(audioOnlyCall, m_ucaConvertedEncodingFrame, SPLIT_TYPE_DEVICE_DESKTOP_WIDTH, 0, iHeight, iWidth, SPLIT_TYPE_DEVICE_DESKTOP_HEIGHT, SPLIT_TYPE_DEVICE_DESKTOP_MOBILE_WIDTH);
							else
								this->m_pColorConverter->Merge_Two_Video2(audioOnlyCall, m_ucaConvertedEncodingFrame, SPLIT_TYPE_DEVICE_DESKTOP_WIDTH, 0, iHeight, iWidth, m_ucaTempFrame2, SPLIT_TYPE_DEVICE_DESKTOP_HEIGHT, SPLIT_TYPE_DEVICE_DESKTOP_MOBILE_WIDTH);
							
							this->m_pColorConverter->Merge_Two_VideoI420(audioOnlyCall, m_ucaConvertedEncodingFrame, 0, SPLIT_TYPE_DEVICE_DESKTOP_HEIGHT, iHeight, iWidth, iHeight - SPLIT_TYPE_DEVICE_DESKTOP_HEIGHT, iWidth);
						}
						else
						{
							this->m_pColorConverter->Merge_Two_VideoI420(audioOnlyCall, m_ucaConvertedEncodingFrame, 0, 0, iHeight, iWidth, upperOffset, iWidth);
							this->m_pColorConverter->Merge_Two_Video2(audioOnlyCall, m_ucaConvertedEncodingFrame, 0, upperOffset, iHeight, iWidth, m_ucaTempFrame, iHeight / 2, iWidth / 2);
							
							if (audioOnlyCall)
								this->m_pColorConverter->Merge_Two_Video3(audioOnlyCall, m_ucaConvertedEncodingFrame, iWidth / 2, upperOffset, iHeight, iWidth, iHeight / 2, iWidth / 2);
							else
								this->m_pColorConverter->Merge_Two_Video2(audioOnlyCall, m_ucaConvertedEncodingFrame, iWidth / 2, upperOffset, iHeight, iWidth, m_ucaTempFrame2, iHeight / 2, iWidth / 2);

							this->m_pColorConverter->Merge_Two_VideoI420(audioOnlyCall, m_ucaConvertedEncodingFrame, 0, iHeight / 2 + upperOffset, iHeight, iWidth, iHeight / 2 - upperOffset, iWidth);
						}		
					}
					else
					{
						this->m_pColorConverter->Merge_Two_Video(m_ucaConvertedEncodingFrame, iPosX, iPosY, iHeight, iWidth);
					}
#endif			
				}
                
			}
			/*	
			else
			{

				bool doSharp = false;

				if(m_pVideoEncoder->GetBitrate() >= MIN_BITRATE_FOR_SHARPING)
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
			}
			*/

			if (m_bSelfViewOnly == false)
			{
				CLogPrinter_WriteLog(CLogPrinter::INFO, OPERATION_TIME_LOG, " Conversion ", llCalculatingTime);

				llCalculatingTime = CLogPrinter_WriteLog(CLogPrinter::INFO, OPERATION_TIME_LOG);

            if(m_pVideoCallSession->isDynamicIDR_Mechanism_Enable())
            {
                bNeedIDR = m_pIdrFrameIntervalController->NeedToGenerateIFrame(m_pVideoCallSession->GetServiceType());
            }
			
			if (m_pVideoCallSession->GetServiceType() == SERVICE_TYPE_LIVE_STREAM || m_pVideoCallSession->GetServiceType() == SERVICE_TYPE_SELF_STREAM || m_pVideoCallSession->GetServiceType() == SERVICE_TYPE_CHANNEL)
			{
				if (m_bNewSessionStarted)
				{
					m_llLastTime = Tools::CurrentTimestamp();

					m_bNewSessionStarted = false;
				}

				long long currentTime = Tools::CurrentTimestamp();

				if (currentTime - m_llLastTime > 200)
				{
					bNeedIDR = true;
					m_llLastTime = currentTime;
				}
			}
			
            
#if defined(TARGET_OS_IPHONE) || defined(TARGET_IPHONE_SIMULATOR)
				llCalculatingTime = m_Tools.CurrentTimestamp();

            if(m_bIsCheckCall)
                nENCODEDFrameSize = m_pVideoEncoder->EncodeVideoFrame(m_ucaDummmyFrame[m_iFrameNumber%2], nEncodingFrameSize, m_ucaEncodedFrame, false);
            else
                nENCODEDFrameSize = m_pVideoEncoder->EncodeVideoFrame(m_ucaEncodingFrame, nEncodingFrameSize, m_ucaEncodedFrame, bNeedIDR);

				//printf("The encoder returned , nENCODEDFrameSize = %d, frameNumber = %d\n", nENCODEDFrameSize, m_iFrameNumber);


#else
			CLogPrinter_LOG(BITRATE_INFO_LOG, "CVideoEncodingThread::EncodingThreadProcedure() dfps bitrate %d maxBitrate %d", m_pVideoEncoder->GetBitrate(), m_pVideoEncoder->GetMaxBitrate());

				long long timeStampForEncoding = m_Tools.CurrentTimestamp();

				if (m_pVideoCallSession->GetOwnDeviceType() == DEVICE_TYPE_DESKTOP && m_nOrientationType != ORIENTATION_90_MIRRORED)
				{
					if (iGotHeight == m_pVideoCallSession->m_nVideoCallHeight && iGotWidth == m_pVideoCallSession->m_nVideoCallWidth)
					{
						if (m_bIsCheckCall)
							nENCODEDFrameSize = m_pVideoEncoder->EncodeVideoFrame(m_ucaDummmyFrame[m_iFrameNumber % 3], nEncodingFrameSize, m_ucaEncodedFrame, false);
						else
							nENCODEDFrameSize = m_pVideoEncoder->EncodeVideoFrame(m_ucaConvertedEncodingFrame, nEncodingFrameSize, m_ucaEncodedFrame, bNeedIDR);
					}
					else
						nENCODEDFrameSize = 0;
				}
				else
				{
					if (m_bIsCheckCall)
						nENCODEDFrameSize = m_pVideoEncoder->EncodeVideoFrame(m_ucaDummmyFrame[m_iFrameNumber % 3], nEncodingFrameSize, m_ucaEncodedFrame, false);
					else
						nENCODEDFrameSize = m_pVideoEncoder->EncodeVideoFrame(m_ucaConvertedEncodingFrame, nEncodingFrameSize, m_ucaEncodedFrame, bNeedIDR);
				}	

				/*
				if (!m_bIsCheckCall)
				{
					fwrite(m_ucaEncodedFrame, sizeof(char), nENCODEDFrameSize, pFile);
					fflush(pFile);
				}
				*/

				//VLOG("#EN# Encoding Frame: " + m_Tools.IntegertoStringConvert(m_iFrameNumber));

				if (nENCODEDFrameSize == 0)
					m_EncodingFailedCounter++;

				CLogPrinter_LOG(ENCODING_FAIL_LOG, "CVideoEncodingThread::EncodingThreadProcedure nENCODEDFrameSize %d m_iFrameNumber %d m_EncodingFailedCounter %d bitrate %d GetScreenSplitType() %d", nENCODEDFrameSize, m_iFrameNumber, m_EncodingFailedCounter, m_pVideoEncoder->GetBitrate(), nCallScreenSplitType);


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
            
            if(m_pVideoCallSession->isDynamicIDR_Mechanism_Enable())
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

				CLogPrinter_WriteLog(CLogPrinter::INFO, THREAD_LOG, "CVideoEncodingThread::EncodingThreadProcedure() Sending for packetization nENCODEDFrameSize " + m_Tools.getText(nENCODEDFrameSize));
			}

			
            
            if(m_iFrameNumber % m_nCallFPS == 0)
            {
                m_iNumberOfEncodeFailPerFPS = iNumberOfEncodeFailCounter;
                iNumberOfEncodeFailCounter = 0;
            }
            
            if(nENCODEDFrameSize <= 0)
            {
                iNumberOfEncodeFailCounter++;
            }

			if ((m_pVideoCallSession->GetServiceType() == SERVICE_TYPE_LIVE_STREAM || m_pVideoCallSession->GetServiceType() == SERVICE_TYPE_SELF_STREAM || m_pVideoCallSession->GetServiceType() == SERVICE_TYPE_CHANNEL) && (m_pVideoCallSession->GetEntityType() == ENTITY_TYPE_PUBLISHER || m_pVideoCallSession->GetEntityType() == ENTITY_TYPE_PUBLISHER_CALLER))
			{

#if defined(DESKTOP_C_SHARP)

				if (nCallScreenSplitType == LIVE_CALL_SCREEN_SPLIT_TYPE_DIVIDE)
				{
					int nSmallFrameHeight = m_pColorConverter->GetOpponentHeightInSplitCall();
					int nSmallFrameWidth = m_pColorConverter->GetOpponentWidthInSplitCall();

					if ((m_pVideoCallSession->GetOwnDeviceType() == DEVICE_TYPE_DESKTOP && iGotWidth > iGotHeight) && (nSmallFrameHeight > nSmallFrameWidth))
					{
						iSmallHeight = SPLIT_TYPE_DEVICE_DESKTOP_HEIGHT;
						iSmallWidth = SPLIT_TYPE_DEVICE_DESKTOP_MOBILE_WIDTH;
					}
					else
					{
						iSmallHeight = iGotHeight / 2;
						iSmallWidth = iGotWidth / 2;
					}
				}
#else
				if (nCallScreenSplitType == LIVE_CALL_SCREEN_SPLIT_TYPE_DIVIDE)
				{
					iSmallHeight = iGotHeight / 2;
					iSmallWidth = iGotWidth / 2;
				}
				else
				{
					iSmallHeight = m_pColorConverter->GetSmallFrameHeight();
					iSmallWidth = m_pColorConverter->GetSmallFrameWidth();
				}
#endif
			}
            
			//if (nENCODEDFrameSize > 0)
			{

				if (m_bSelfViewOnly == false)
				{
					if ((m_pVideoCallSession->GetServiceType() == SERVICE_TYPE_LIVE_STREAM || m_pVideoCallSession->GetServiceType() == SERVICE_TYPE_SELF_STREAM || m_pVideoCallSession->GetServiceType() == SERVICE_TYPE_CHANNEL) && (m_pVideoCallSession->GetEntityType() == ENTITY_TYPE_PUBLISHER || m_pVideoCallSession->GetEntityType() == ENTITY_TYPE_PUBLISHER_CALLER))
					{
						if (nCallScreenSplitType == LIVE_CALL_SCREEN_SPLIT_TYPE_DIVIDE)
						{
							m_pEncodedFramePacketizer->Packetize(m_llFriendID, m_ucaEncodedFrame, nENCODEDFrameSize, m_iFrameNumber, nCaptureTimeDifference, nDevice_orientation, VIDEO_DATA_MOOD, iSmallHeight, iSmallWidth);
						}
						else
							m_pEncodedFramePacketizer->Packetize(m_llFriendID, m_ucaEncodedFrame, nENCODEDFrameSize, m_iFrameNumber, nCaptureTimeDifference, nDevice_orientation, VIDEO_DATA_MOOD, 0, 0);
					}
					else
						m_pEncodedFramePacketizer->Packetize(m_llFriendID, m_ucaEncodedFrame, nENCODEDFrameSize, m_iFrameNumber, nCaptureTimeDifference, nDevice_orientation, VIDEO_DATA_MOOD, 0, 0);
				}	

				if ((m_pVideoCallSession->GetServiceType() == SERVICE_TYPE_LIVE_STREAM || m_pVideoCallSession->GetServiceType() == SERVICE_TYPE_SELF_STREAM || m_pVideoCallSession->GetServiceType() == SERVICE_TYPE_CHANNEL) && (m_pVideoCallSession->GetEntityType() == ENTITY_TYPE_PUBLISHER || m_pVideoCallSession->GetEntityType() == ENTITY_TYPE_PUBLISHER_CALLER))
				{

#if defined(TARGET_OS_IPHONE) || defined(TARGET_IPHONE_SIMULATOR)

					this->m_pColorConverter->ConvertI420ToNV12(m_ucaMirroredFrame, iGotHeight, iGotWidth);

#elif defined(DESKTOP_C_SHARP)

					int m_decodedFrameSize;
					/*
					if (m_pVideoCallSession->GetEntityType() == ENTITY_TYPE_PUBLISHER_CALLER)
					{
						if (this->m_pColorConverter->GetSmallFrameStatus())
						{
							this->m_pColorConverter->GetSmallFrame(m_pSmallFrame);

							m_decodedFrameSize = this->m_pColorConverter->ConverterYUV420ToRGB24(m_pSmallFrame, m_RenderingRGBFrame, m_pColorConverter->GetSmallFrameHeight(), m_pColorConverter->GetSmallFrameWidth());
						}
					}
					else*/
					{
						m_decodedFrameSize = this->m_pColorConverter->ConverterYUV420ToRGB24(m_ucaMirroredFrame, m_RenderingRGBFrame, iGotHeight, iGotWidth);
					}
					 
#elif defined(TARGET_OS_WINDOWS_PHONE)

					this->m_pColorConverter->ConvertI420ToYV12(m_ucaMirroredFrame, iGotHeight, iGotWidth);
#else
					this->m_pColorConverter->ConvertI420ToNV21(m_ucaMirroredFrame, iGotHeight, iGotWidth);
#endif
                    
                    int iHeight = iGotHeight;
                    int iWidth = iGotWidth;
                    
                    int iScreenHeight = this->m_pColorConverter->GetScreenHeight();
                    int iScreenWidth = this->m_pColorConverter->GetScreenWidth();
                    
                    int iCropedHeight = 0;
                    int iCropedWidth = 0;
                    
                    int iCroppedDataLen;

#if defined(DESKTOP_C_SHARP)
					/*
					if (m_pVideoCallSession->GetEntityType() == ENTITY_TYPE_PUBLISHER_CALLER)
					{
						if (this->m_pColorConverter->GetSmallFrameStatus())
						{
							m_pCommonElementBucket->m_pEventNotifier->fireVideoEvent(m_llFriendID, SERVICE_TYPE_LIVE_STREAM, m_iFrameNumber, m_decodedFrameSize, m_RenderingRGBFrame, m_pColorConverter->GetSmallFrameHeight(), m_pColorConverter->GetSmallFrameWidth(), 0, 0, nDevice_orientation);
						}
					}
					else*/
					{
                        //iCroppedDataLen = this->m_pColorConverter->CropWithAspectRatio_YUVNV12_YUVNV21_RGB24(m_RenderingRGBFrame, iHeight, iWidth, iScreenHeight, iScreenWidth, m_ucaCropedFrame, iCropedHeight, iCropedWidth, RGB24);

                        //if(iScreenWidth == -1 || iScreenHeight == -1)
						m_pCommonElementBucket->m_pEventNotifier->fireVideoEvent(m_llFriendID, m_pVideoCallSession->GetServiceType(), m_iFrameNumber, m_decodedFrameSize, m_RenderingRGBFrame, iGotHeight, iGotWidth, iSmallHeight, iSmallWidth, nDevice_orientation);
                        //else
						//	m_pCommonElementBucket->m_pEventNotifier->fireVideoEvent(m_llFriendID, SERVICE_TYPE_LIVE_STREAM, m_iFrameNumber, iCroppedDataLen, m_ucaCropedFrame, iCropedHeight, iCropedWidth, iSmallHeight, iSmallWidth, nDevice_orientation);
                    }
                    
#elif defined(TARGET_OS_IPHONE) || defined(TARGET_IPHONE_SIMULATOR) || defined (__ANDROID__) || defined (TARGET_OS_WINDOWS_PHONE)
                    
                    int nColorFormatType = -1;
                    int nOwnDeviceType = m_pVideoCallSession->GetOwnDeviceType();
                    
                    if(nOwnDeviceType == DEVICE_TYPE_IOS)
                    {
                        nColorFormatType = YUVNV12;
                    }
                    else if(nOwnDeviceType == DEVICE_TYPE_ANDROID)
                    {
                        nColorFormatType = YUVNV21;
                    }
                    else if(nOwnDeviceType == DEVICE_TYPE_WINDOWS_PHONE)
                    {
                        nColorFormatType = YUVYV12;
                    }
                    
                    
                    long long cropStartTime = m_Tools.CurrentTimestamp();
                    iCroppedDataLen = this->m_pColorConverter->CropWithAspectRatio_YUVNV12_YUVNV21_RGB24(m_ucaMirroredFrame, iHeight, iWidth, iScreenHeight, iScreenWidth, m_ucaCropedFrame, iCropedHeight, iCropedWidth, nColorFormatType);
                    //printf("TheKing--> CropTimeDiff = %lld\n", m_Tools.CurrentTimestamp() - cropStartTime);
                        
                    //printf("iScreen, H:W = %d:%d,   iCroped H:W = %d:%d, iCroppedLen = %d\n",iScreenHeight, iScreenWidth, iCropedHeight, iCropedWidth, iCroppedDataLen); 
                    
                    if(iScreenWidth == -1 || iScreenHeight == -1)
						m_pCommonElementBucket->m_pEventNotifier->fireVideoEvent(m_llFriendID, m_pVideoCallSession->GetServiceType(), m_iFrameNumber, ((iGotHeight * iGotWidth * 3) / 2), m_ucaMirroredFrame, iGotHeight, iGotWidth, iSmallHeight, iSmallWidth, nDevice_orientation);
                    else
						m_pCommonElementBucket->m_pEventNotifier->fireVideoEvent(m_llFriendID, m_pVideoCallSession->GetServiceType(), m_iFrameNumber, iCroppedDataLen, m_ucaCropedFrame, iCropedHeight, iCropedWidth, iSmallHeight, iSmallWidth, nDevice_orientation);
                    
#else
					m_pCommonElementBucket->m_pEventNotifier->fireVideoEvent(m_llFriendID, m_pVideoCallSession->GetServiceType(), m_iFrameNumber, ((iGotHeight * iGotWidth * 3) / 2), m_ucaMirroredFrame, iGotHeight, iGotWidth, nDevice_orientation);
#endif
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

			CLogPrinter_LOG(PUBLISHER_TIME_LOG, "Average %d count %d\n", sum / countNumber, countNumber);

			countNumber++;
		
			toolsObject.SOSleep(0);
		}
	}

	bEncodingThreadClosed = true;

	CLogPrinter_WriteLog(CLogPrinter::INFO, THREAD_LOG ,"CVideoEncodingThread::EncodingThreadProcedure() stopped EncodingThreadProcedure method.");
}

void CVideoEncodingThread::MakeBlackScreen(unsigned char *pData, int iHeight, int iWidth, int colorFormat)
{ 
    if(colorFormat == YUV420)
    {
        int yPlaneLength = iHeight * iWidth;
        int uvPlaneLength = yPlaneLength>>1;
        
        memset(pData, 0, yPlaneLength);
        memset(pData + yPlaneLength, 128, uvPlaneLength);
    }
    else if(colorFormat == RGB24)
    {
        memset(pData, 0, iHeight * iWidth * 3);
    }
    else
    {
        //This color format type is not handled
    }
    
    return;
    
}
    
void CVideoEncodingThread::SetBeautification(bool bIsEnable)
{
    
#if defined(DESKTOP_C_SHARP)
    m_bVideoEffectEnabled = bIsEnable;
#endif
    
}


void CVideoEncodingThread::TestVideoEffect(int *param, int size)
{
	m_VideoBeautificationer->TestVideoEffect(param, size);
}



CVideoBeautificationer* CVideoEncodingThread::getVideoBeautificationar()
{
    return m_VideoBeautificationer;
}











//			encodingTimeStamp = toolsObject.CurrentTimestamp();			// called before encoding

/*
encodingTime = toolsObject.CurrentTimestamp() - encodingTimeStamp;

encodeTimeStampFor15 += encodingTime;

countFrameSize = countFrameSize + encodedFrameSize;

if (countFrame >= 15)
{
encodingTimeFahadTest = toolsObject.CurrentTimestamp() - encodingTimeFahadTest;
CLogPrinter_WriteSpecific3(CLogPrinter::DEBUGS, "CVideoEncodingThread::EncodingThreadProcedure() Encoded " + Tools::IntegertoStringConvert(countFrame) + " frames Size: " + Tools::IntegertoStringConvert(countFrameSize * 8) + " encodeTimeStampFor15 : " + Tools::IntegertoStringConvert(encodeTimeStampFor15) + " Full_Lop: " + Tools::IntegertoStringConvert(encodingTimeFahadTest));
encodingTimeFahadTest = toolsObject.CurrentTimestamp();
countFrame = 0;
countFrameSize = 0;
encodeTimeStampFor15 = 0;
}

countFrame++;

dbTotalEncodingTime += encodingTime;
++iEncodedFrameCounter;
nMaxEncodingTime = max(nMaxEncodingTime, encodingTime);
*/

} //namespace MediaSDK
