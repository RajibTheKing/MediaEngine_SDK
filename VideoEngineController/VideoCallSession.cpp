
#include "VideoCallSession.h"
#include "CommonElementsBucket.h"
#include "LogPrinter.h"
#include "Globals.h"
#include "Controller.h"
#include "InterfaceOfAudioVideoEngine.h"


//PairMap g_timeInt;

#if defined(TARGET_OS_IPHONE) || defined(TARGET_IPHONE_SIMULATOR)
#include <dispatch/dispatch.h>
#endif

#define MINIMUM_CAPTURE_INTERVAL_TO_UPDATE_FPS 10

extern long long g_llFirstFrameReceiveTime;
CVideoCallSession::CVideoCallSession(CController *pController, LongLong fname, CCommonElementsBucket* sharedObject, int nFPS, int *nrDeviceSupportedCallFPS, bool bIsCheckCall, CDeviceCapabilityCheckBuffer *deviceCheckCapabilityBuffer, int nOwnSupportedResolutionFPSLevel, int nServiceType, int nEntityType) :

m_pCommonElementsBucket(sharedObject),
m_ClientFPS(DEVICE_FPS_MAXIMUM),
m_ClientFPSDiffSum(0),
m_ClientFrameCounter(0),
m_EncodingFrameCounter(0),
m_pEncodedFramePacketizer(NULL),
m_ByteRcvInBandSlot(0),
m_SlotResetLeftRange(0),
m_SlotResetRightRange(nFPS),
m_pVideoEncoder(NULL),
m_bSkipFirstByteCalculation(true),
m_llTimeStampOfFirstPacketRcvd(-1),
m_nFirstFrameEncodingTimeDiff(-1),
m_llShiftedTime(-1),
mt_llCapturePrevTime(0),
m_bResolationCheck(false),
m_bShouldStartCalculation(false),
m_bCaclculationStartTime(0),
m_bHighResolutionSupportedForOwn(false),
m_bHighResolutionSupportedForOpponent(false),
m_bReinitialized(false),
m_bResolutionNegotiationDone(false),
m_pVersionController(NULL),
m_bIsCheckCall(bIsCheckCall),
m_nCallFPS(nFPS),
pnDeviceSupportedFPS(nrDeviceSupportedCallFPS),
m_nOwnVideoCallQualityLevel(nOwnSupportedResolutionFPSLevel),
m_pDeviceCheckCapabilityBuffer(deviceCheckCapabilityBuffer),
m_bVideoCallStarted(false),
m_nDeviceCheckFrameCounter(0),
m_nCapturedFrameCounter(0),
m_nServiceType(nServiceType),
m_nEntityType(nEntityType),
m_iRole(0),
m_bVideoEffectEnabled(true),
m_nOponentDeviceType(DEVICE_TYPE_UNKNOWN),
m_nOpponentVideoHeight(-1),
m_nOpponentVideoWidth(-1)

{

#if defined(TARGET_OS_IPHONE) || defined(TARGET_IPHONE_SIMULATOR)

	m_nOwnDeviceType = DEVICE_TYPE_IOS;

#elif defined(_DESKTOP_C_SHARP_)

	m_nOwnDeviceType = DEVICE_TYPE_DESKTOP;

#elif defined(TARGET_OS_WINDOWS_PHONE)

	m_nOwnDeviceType = DEVICE_TYPE_WINDOWS_PHONE;

#else

	m_nOwnDeviceType = DEVICE_TYPE_ANDROID;

#endif


    m_nOpponentVideoCallQualityLevel = VIDEO_CALL_TYPE_UNKNOWN;
    m_nCurrentVideoCallQualityLevel = VIDEO_CALL_TYPE_UNKNOWN;

	m_nDeviceHeight = pController->GetDeviceDisplayHeight();
	m_nDeviceWidth = pController->GetDeviceDisplayWidth();
    
    m_VideoFpsCalculator = new CAverageCalculator();
    m_bLiveVideoStreamRunning = false;
    
	if (m_nServiceType == SERVICE_TYPE_LIVE_STREAM || m_nServiceType == SERVICE_TYPE_SELF_STREAM || m_nServiceType == SERVICE_TYPE_CHANNEL)
    {
        m_bLiveVideoStreamRunning = true;
        m_pLiveVideoDecodingQueue = new LiveVideoDecodingQueue();
		m_pLiveReceiverVideo = new LiveReceiver(m_pCommonElementsBucket, NULL);
        m_pLiveReceiverVideo->SetVideoDecodingQueue(m_pLiveVideoDecodingQueue);
        
        
        
        m_nOpponentVideoCallQualityLevel = nOwnSupportedResolutionFPSLevel;
        m_nCurrentVideoCallQualityLevel = nOwnSupportedResolutionFPSLevel;
    }
    
    
    
    
    
    
	//CLogPrinter_WriteLog(CLogPrinter::INFO, INSTENT_TEST_LOG, "CVideoCallSession::CVideoCallSession 54");
    m_llClientFrameFPSTimeStamp = -1;
    m_pController = pController;
    
	m_miniPacketBandCounter = 0;

	//Resetting Global Variables.

	if (m_bIsCheckCall == LIVE_CALL_MOOD)
	{
		m_bResolationCheck = true;
	}

	m_pFPSController = new CFPSController(nFPS);

	CLogPrinter_Write(CLogPrinter::INFO, "CVideoCallSession::CVideoCallSession");
	m_pVideoCallSessionMutex.reset(new CLockHandler);
	m_lfriendID = fname;
	sessionMediaList.ClearAllFromVideoEncoderList();


	m_SendingBuffer = new CSendingBuffer();
	m_EncodingBuffer = new CEncodingBuffer();
	m_RenderingBuffer = new CRenderingBuffer();

	m_pVideoPacketQueue = new CVideoPacketQueue();
	m_pRetransVideoPacketQueue = new CVideoPacketQueue();
	m_pMiniPacketQueue = new CVideoPacketQueue();

	m_pEncodedFramePacketizer = new CEncodedFramePacketizer(sharedObject, m_SendingBuffer, this);
	m_pEncodedFrameDepacketizer = new CEncodedFrameDepacketizer(sharedObject, this);

	m_BitRateController = new BitRateController(m_nCallFPS, m_lfriendID);

	m_BitRateController->SetSharedObject(sharedObject);

	//CLogPrinter_WriteLog(CLogPrinter::INFO, INSTENT_TEST_LOG, "CVideoCallSession::CVideoCallSession 90");
}

CVideoCallSession::~CVideoCallSession()
{
	//CLogPrinter_WriteLog(CLogPrinter::INFO, INSTENT_TEST_LOG, "CVideoCallSession::~~~CVideoCallSession 95");
	m_pVideoEncodingThread->StopEncodingThread();
	m_pSendingThread->StopSendingThread();
	m_pVideoDepacketizationThread->StopDepacketizationThread();
	m_pVideoDecodingThread->StopDecodingThread();
	m_pVideoRenderingThread->StopRenderingThread();

	if (NULL != m_pVideoEncodingThread)
	{
		delete m_pVideoEncodingThread;
		m_pVideoEncodingThread = NULL;
	}

	if (NULL != m_pVideoDepacketizationThread)
	{
		delete m_pVideoDepacketizationThread;
		m_pVideoDepacketizationThread = NULL;
	}

	if (NULL != m_pVideoDecodingThread)
	{
		delete m_pVideoDecodingThread;
		m_pVideoDecodingThread = NULL;
	}

	if (NULL != m_pVideoRenderingThread)
	{
		delete m_pVideoRenderingThread;
		m_pVideoRenderingThread = NULL;
	}

	if (NULL != m_BitRateController)
	{
		delete m_BitRateController;
		m_BitRateController = NULL;
	}

	if (NULL != m_pVideoPacketQueue)
	{
		delete m_pVideoPacketQueue;
		m_pVideoPacketQueue = NULL;
	}

	if (NULL != m_pRetransVideoPacketQueue)
	{
		delete m_pRetransVideoPacketQueue;
		m_pRetransVideoPacketQueue = NULL;
	}

	if (NULL != m_pMiniPacketQueue)
	{
		delete m_pMiniPacketQueue;
		m_pMiniPacketQueue = NULL;
	}

	if (NULL != m_RenderingBuffer)
	{
		delete m_RenderingBuffer;
		m_RenderingBuffer = NULL;
	}

	if (NULL != m_EncodingBuffer)
	{
		delete m_EncodingBuffer;
		m_EncodingBuffer = NULL;
	}

	if (NULL != m_pVideoEncoder)
	{
		delete m_pVideoEncoder;
		m_pVideoEncoder = NULL;
	}

	if (NULL != m_pEncodedFramePacketizer)
	{
		delete m_pEncodedFramePacketizer;
		m_pEncodedFramePacketizer = NULL;
	}

	if (NULL != m_pEncodedFrameDepacketizer)
	{
		delete m_pEncodedFrameDepacketizer;
		m_pEncodedFrameDepacketizer = NULL;
	}

	if (NULL != m_pVideoDecoder)
	{
		delete m_pVideoDecoder;

		m_pVideoDecoder = NULL;
	}

	if (NULL != m_pColorConverter)
	{
		delete m_pColorConverter;

		m_pColorConverter = NULL;
	}

	if (NULL != m_pSendingThread)
	{
		delete m_pSendingThread;
		m_pSendingThread = NULL;
	}

	if (NULL != m_SendingBuffer)
	{
		delete m_SendingBuffer;
		m_SendingBuffer = NULL;
	}

	if(NULL != m_pFPSController)
	{
		delete m_pFPSController;
		m_pFPSController = NULL;
	}

	m_lfriendID = -1;
    
    if(NULL  != m_pVersionController)
    {
        delete m_pVersionController;
        m_pVersionController = NULL;    
    }
    
    if(NULL != m_VideoFpsCalculator)
    {
        delete m_VideoFpsCalculator;
        m_VideoFpsCalculator = NULL;
        
    }
    
	if (m_nServiceType == SERVICE_TYPE_LIVE_STREAM || m_nServiceType == SERVICE_TYPE_SELF_STREAM || m_nServiceType == SERVICE_TYPE_CHANNEL)
    {
        if(NULL != m_pLiveReceiverVideo)
        {
            delete m_pLiveReceiverVideo;
            m_pLiveReceiverVideo = NULL;
        }
        
        if(NULL != m_pLiveVideoDecodingQueue)
        {
            delete m_pLiveVideoDecodingQueue;
            
            m_pLiveVideoDecodingQueue = NULL;
        }
    }
    
    
	//CLogPrinter_WriteLog(CLogPrinter::INFO, INSTENT_TEST_LOG, "CVideoCallSession::~~~CVideoCallSession 220");
	SHARED_PTR_DELETE(m_pVideoCallSessionMutex);
}

LongLong CVideoCallSession::GetFriendID()
{
	return m_lfriendID;
}

void CVideoCallSession::InitializeVideoSession(LongLong lFriendID, int iVideoHeight, int iVideoWidth, int nServiceType, int iNetworkType)
{

	//CLogPrinter_WriteLog(CLogPrinter::INFO, INSTENT_TEST_LOG, "CVideoCallSession::InitializeVideoSession 232");
    m_nServiceType = nServiceType;
    
	m_nVideoCallHeight = iVideoHeight;
	m_nVideoCallWidth = iVideoWidth;

    m_pVersionController = new CVersionController();
    
    if(m_bLiveVideoStreamRunning == true)
    {
        //m_iOppVersion = VIDEO_VERSION_CODE;
        //m_iCurrentCallVersion = VIDEO_VERSION_CODE;
        //m_bFirstVideoPacketReceivedFlag = true;
        
        m_pVersionController->SetOpponentVersion(VIDEO_VERSION_CODE);
        m_pVersionController->SetCurrentCallVersion(VIDEO_VERSION_CODE);
        m_pVersionController->NotifyFirstVideoPacetReceived();
    }
    
    
    g_llFirstFrameReceiveTime = 0;

	//CLogPrinter_WriteLog(CLogPrinter::INFO, INSTENT_TEST_LOG, "CVideoCallSession::InitializeVideoSession 240");

	if (sessionMediaList.IsVideoEncoderExist(iVideoHeight, iVideoWidth))
	{
		return;
	}

	//CLogPrinter_WriteLog(CLogPrinter::INFO, INSTENT_TEST_LOG, "CVideoCallSession::InitializeVideoSession 247");

	this->m_pVideoEncoder = new CVideoEncoder(m_pCommonElementsBucket, m_lfriendID);

	if (m_nServiceType == SERVICE_TYPE_LIVE_STREAM || m_nServiceType == SERVICE_TYPE_SELF_STREAM || m_nServiceType == SERVICE_TYPE_CHANNEL)
		m_pVideoEncoder->CreateVideoEncoder(iVideoHeight, iVideoWidth, m_nCallFPS, m_nCallFPS / IFRAME_INTERVAL, m_bIsCheckCall, nServiceType);
	else
		m_pVideoEncoder->CreateVideoEncoder(iVideoHeight, iVideoWidth, m_nCallFPS, m_nCallFPS / 2 + 1, m_bIsCheckCall, nServiceType);

	m_pFPSController->SetEncoder(m_pVideoEncoder);
	m_BitRateController->SetEncoder(m_pVideoEncoder);

	this->m_pVideoDecoder = new CVideoDecoder(m_pCommonElementsBucket);

	m_pVideoDecoder->CreateVideoDecoder();

	this->m_pColorConverter = new CColorConverter(iVideoHeight, iVideoWidth, m_pCommonElementsBucket, m_lfriendID);

	this->m_pColorConverter->SetDeviceHeightWidth(m_nDeviceHeight, m_nDeviceWidth);

	//CLogPrinter_WriteLog(CLogPrinter::INFO, INSTENT_TEST_LOG, "CVideoCallSession::InitializeVideoSession 262");

	m_pSendingThread = new CSendingThread(m_pCommonElementsBucket, m_SendingBuffer, this, m_bIsCheckCall, m_lfriendID);
	m_pVideoEncodingThread = new CVideoEncodingThread(lFriendID, m_EncodingBuffer, m_pCommonElementsBucket, m_BitRateController, m_pColorConverter, m_pVideoEncoder, m_pEncodedFramePacketizer, this, m_nCallFPS, m_bIsCheckCall);
	m_pVideoRenderingThread = new CVideoRenderingThread(lFriendID, m_RenderingBuffer, m_pCommonElementsBucket, this, m_bIsCheckCall);
	m_pVideoDecodingThread = new CVideoDecodingThread(m_pEncodedFrameDepacketizer, lFriendID, m_pCommonElementsBucket, m_RenderingBuffer, m_pLiveVideoDecodingQueue, m_pVideoDecoder, m_pColorConverter, this, m_bIsCheckCall, m_nCallFPS);
	m_pVideoDepacketizationThread = new CVideoDepacketizationThread(lFriendID, m_pVideoPacketQueue, m_pRetransVideoPacketQueue, m_pMiniPacketQueue, m_BitRateController, m_pEncodedFrameDepacketizer, m_pCommonElementsBucket, &m_miniPacketBandCounter, m_pVersionController, this);

	//CLogPrinter_WriteLog(CLogPrinter::INFO, INSTENT_TEST_LOG, "CVideoCallSession::InitializeVideoSession 270");

	m_pCommonElementsBucket->m_pVideoEncoderList->AddToVideoEncoderList(lFriendID, m_pVideoEncoder);

    //CLogPrinter_WriteLog(CLogPrinter::INFO, INSTENT_TEST_LOG, "CVideoCallSession::InitializeVideoSession 274");
	m_ClientFrameCounter = 0;
	m_EncodingFrameCounter = 0;
	m_llFirstFrameCapturingTimeStamp = -1;


	m_BitRateController->SetOwnNetworkType(iNetworkType);
	//CLogPrinter_WriteLog(CLogPrinter::INFO, INSTENT_TEST_LOG, "CVideoCallSession::InitializeVideoSession 281");
	//CreateAndSendMiniPacket(iNetworkType, __NETWORK_INFO_PACKET_TYPE);

	//CLogPrinter_WriteLog(CLogPrinter::INFO, INSTENT_TEST_LOG, "CVideoCallSession::InitializeVideoSession 282");

	m_pSendingThread->StartSendingThread();

	//CLogPrinter_WriteLog(CLogPrinter::INFO, INSTENT_TEST_LOG, "CVideoCallSession::InitializeVideoSession 286");
	m_pVideoEncodingThread->StartEncodingThread();

	//CLogPrinter_WriteLog(CLogPrinter::INFO, INSTENT_TEST_LOG, "CVideoCallSession::InitializeVideoSession 289");
	m_pVideoRenderingThread->StartRenderingThread();	
	//CLogPrinter_WriteLog(CLogPrinter::INFO, INSTENT_TEST_LOG, "CVideoCallSession::InitializeVideoSession 291");
	m_pVideoDepacketizationThread->StartDepacketizationThread();
	//CLogPrinter_WriteLog(CLogPrinter::INFO, INSTENT_TEST_LOG, "CVideoCallSession::InitializeVideoSession 293");
	m_pVideoDecodingThread->StartDecodingThread();



	//CLogPrinter_WriteLog(CLogPrinter::INFO, INSTENT_TEST_LOG, "CVideoCallSession::InitializeVideoSession 298");
}

CVideoEncoder* CVideoCallSession::GetVideoEncoder()
{
	//	return sessionMediaList.GetFromVideoEncoderList(mediaName);

	return m_pVideoEncoder;
}

long long CVideoCallSession::GetFirstVideoPacketTime(){
	return m_llTimeStampOfFirstPacketRcvd;
}

void CVideoCallSession::SetFirstVideoPacketTime(long long llTimeStamp){
	m_llTimeStampOfFirstPacketRcvd = llTimeStamp;
}

void CVideoCallSession::SetFirstFrameEncodingTime(long long time)
{
	m_nFirstFrameEncodingTimeDiff = time;
}

long long CVideoCallSession::GetFirstFrameEncodingTime(){
	return m_nFirstFrameEncodingTimeDiff;
}

bool CVideoCallSession::PushPacketForMergingVector(int offset, unsigned char *in_data, unsigned int in_size, bool bSelfData, int numberOfFrames, int *frameSizes, std::vector< std::pair<int, int> > vMissingFrames)
{
	if (m_bLiveVideoStreamRunning)
	{
		m_pLiveReceiverVideo->PushVideoDataVector(offset, in_data, in_size, numberOfFrames, frameSizes, vMissingFrames);

		return true;
	}

	return true;
}

bool CVideoCallSession::PushPacketForMerging(unsigned char *in_data, unsigned int in_size, bool bSelfData, int numberOfFrames, int *frameSizes, int numberOfMissingFrames, int *missingFrames)
{
	if(m_bLiveVideoStreamRunning)
	{		
		m_cVH.setPacketHeader(in_data);

		m_cVH.ShowDetails("After Receiving ");

		if (m_nEntityType == ENTITY_TYPE_PUBLISHER_CALLER)
			m_pVideoPacketQueue->Queue(in_data, in_size);	
		else if (m_nEntityType != ENTITY_TYPE_PUBLISHER)
			m_pLiveReceiverVideo->PushVideoData(in_data, in_size, numberOfFrames, frameSizes, numberOfMissingFrames, missingFrames);
			
		return true;
	}


	//CLogPrinter_WriteLog(CLogPrinter::INFO, INSTENT_TEST_LOG, "CVideoCallSession::PushPacketForMerging 326");
	unsigned char uchPacketType = in_data[__PACKET_TYPE_INDEX];
	if(uchPacketType < __MIN_PACKET_TYPE || __MAX_PACKET_TYPE < uchPacketType)
		return false;


	if (__BITRATE_CONTROLL_PACKET_TYPE == uchPacketType || __NETWORK_INFO_PACKET_TYPE == uchPacketType) // It is a minipacket
	{
		//CLogPrinter_WriteLog(CLogPrinter::INFO, INSTENT_TEST_LOG, "CVideoCallSession::PushPacketForMerging 334");
		m_pMiniPacketQueue->Queue(in_data, in_size);
	}	
	else if (__VIDEO_PACKET_TYPE == uchPacketType)
	{
        /*if(bSelfData == false && m_bResolutionNegotiationDone == false)
        {
            OperationForResolutionControl(in_data,in_size);
        }*/
        m_PacketHeader.setPacketHeader(in_data);
        
		unsigned int unFrameNumber = (unsigned int)m_PacketHeader.getFrameNumber();
        
//		VLOG("#DR# --------------------------> FrameNumber : "+Tools::IntegertoStringConvert(unFrameNumber));
        //printf("PushPacketForMerging--> nFrameNumber = %d, m_nCallFPS = %d, m_PacketHeader.GetHeaderLength() = %d\n", unFrameNumber, m_nCallFPS, m_PacketHeader.GetHeaderLength());
        

		if (unFrameNumber >= m_SlotResetLeftRange && unFrameNumber < m_SlotResetRightRange)
		{
			m_ByteRcvInBandSlot += (in_size - m_PacketHeader.GetHeaderLength());
		}
		else
		{
			if (m_bSkipFirstByteCalculation == true)
			{
				m_bSkipFirstByteCalculation = false;
			}
			else
			{
				m_miniPacketBandCounter = m_SlotResetLeftRange / m_nCallFPS;
//				VLOG("#DR# -----------------+++++++++------> m_miniPacketBandCounter : "+Tools::IntegertoStringConvert(m_miniPacketBandCounter));
//                CLogPrinter_WriteLog(CLogPrinter::INFO, INSTENT_TEST_LOG, "ReceivingSide: SlotIndex = " + m_Tools.IntegertoStringConvert(m_miniPacketBandCounter) + ", ReceivedBytes = " + m_Tools.IntegertoStringConvert(m_ByteRcvInBandSlot));

				CreateAndSendMiniPacket(m_ByteRcvInBandSlot, __BITRATE_CONTROLL_PACKET_TYPE);
			}

			m_SlotResetLeftRange = unFrameNumber - (unFrameNumber % m_nCallFPS);
            
            
			m_SlotResetRightRange = m_SlotResetLeftRange + m_nCallFPS;

			m_ByteRcvInBandSlot = in_size - m_PacketHeader.GetHeaderLength();
		}

		m_pVideoPacketQueue->Queue(in_data, in_size);

		//CLogPrinter_WriteLog(CLogPrinter::INFO, INSTENT_TEST_LOG, "CVideoCallSession::PushPacketForMerging 378  in_size= " + m_Tools.IntegertoStringConvert(in_size));
	}
	else if (__NEGOTIATION_PACKET_TYPE == uchPacketType)
	{
		m_pVideoPacketQueue->Queue(in_data, in_size);
		//CLogPrinter_WriteLog(CLogPrinter::INFO, INSTENT_TEST_LOG, "CVideoCallSession::PushPacketForMerging 383");
	}
	else
	{
		//This condition will not be appeared
		return false;
	}

	return true;
}

map<int, long long> g_TimeTraceFromCaptureToSend;
int g_CapturingFrameCounter = 0;

int CVideoCallSession::PushIntoBufferForEncoding(unsigned char *in_data, unsigned int in_size, int device_orientation)
{
    //CLogPrinter_WriteLog(CLogPrinter::INFO, INSTENT_TEST_LOG, "CVideoCallSession::PushIntoBufferForEncoding 1");

	if ((m_nServiceType == SERVICE_TYPE_LIVE_STREAM || m_nServiceType == SERVICE_TYPE_SELF_STREAM || m_nServiceType == SERVICE_TYPE_CHANNEL) && m_nEntityType == ENTITY_TYPE_VIEWER)
		return 1;
    
    m_VideoFpsCalculator->CalculateFPS("PushIntoBufferForEncoding, VideoFPS--> ");
    /*if(m_bIsCheckCall==true)
    {
        m_nDeviceCheckFrameCounter++;
        if(m_nDeviceCheckFrameCounter>75) return m_nDeviceCheckFrameCounter;
    }*/

    //LOGE("CVideoCallSession::PushIntoBufferForEncoding called");
    
	m_nCapturedFrameCounter++;

	//CLogPrinter_WriteLog(CLogPrinter::INFO, INSTENT_TEST_LOG, "CVideoCallSession::PushIntoBufferForEncoding 407");
    
	if ( GetVersionController()->GetCurrentCallVersion() == -1 && m_bIsCheckCall == false)
    {
		//if( m_nCapturedFrameCounter < VIDEO_START_WITHOUT_VERSION_TIMEOUT_COUNTER )
        //{
        //      return 1;
        //}

        //LOGE("CVideoCallSession::PushIntoBufferForEncoding  GetVersionController()->GetCurrentCallVersion() == -1 && m_bIsCheckCall == false so returning" );

        return 1;
    }

	if (m_bVideoCallStarted == false && m_bIsCheckCall == false)
	{
	    //LOGE("CVideoCallSession::PushIntoBufferForEncoding  m_bVideoCallStarted == false && m_bIsCheckCall == false so returning");
        
        if(m_bLiveVideoStreamRunning == false)
            return 1;
	}
    
    //CLogPrinter_WriteLog(CLogPrinter::INFO, INSTENT_TEST_LOG, "CVideoCallSession::PushIntoBufferForEncoding 2");
    
	if (m_pVideoEncodingThread->IsThreadStarted() == false)
	{
	    //LOGE("CVideoCallSession::PushIntoBufferForEncoding m_pVideoEncodingThread->IsThreadStarted() == false so returning");

	    return 1;
	}
    
    //CLogPrinter_WriteLog(CLogPrinter::INFO, INSTENT_TEST_LOG, "CVideoCallSession::PushIntoBufferForEncoding 3");

#if defined(SOUL_SELF_DEVICE_CHECK)
	
	if (m_bIsCheckCall == true)
	{
	    //LOGE("CVideoCallSession::PushIntoBufferForEncoding m_bIsCheckCall == true so returning");

	    return 1;
	}
	
#endif
    
   // CLogPrinter_WriteLog(CLogPrinter::INFO, INSTENT_TEST_LOG, "CVideoCallSession::PushIntoBufferForEncoding 4");

    if(g_llFirstFrameReceiveTime == 0) g_llFirstFrameReceiveTime = m_Tools.CurrentTimestamp();
    
	
	CLogPrinter_Write(CLogPrinter::INFO, "CVideoCallSession::PushIntoBufferForEncoding");

	LongLong currentTimeStamp = m_Tools.CurrentTimestamp();

    //CLogPrinter_WriteLog(CLogPrinter::INFO, INSTENT_TEST_LOG, "CVideoCallSession::PushIntoBufferForEncoding 5");
    //Capturing fps calculation
    if(m_llClientFrameFPSTimeStamp==-1) m_llClientFrameFPSTimeStamp = currentTimeStamp;
    m_ClientFrameCounter++;
    if(currentTimeStamp - m_llClientFrameFPSTimeStamp >= 1000)
    {
        {//Block for Lock
            Locker lock(*m_pVideoCallSessionMutex);
            if(m_ClientFrameCounter > MINIMUM_CAPTURE_INTERVAL_TO_UPDATE_FPS)
            {
                //printf("RajibTheKing, setting m_ClientFrameCounter = %d\n", m_ClientFrameCounter);
                m_pFPSController->SetClientFPS(m_ClientFrameCounter);
            }
        }
        
        m_ClientFrameCounter = 0;
        m_llClientFrameFPSTimeStamp = currentTimeStamp;
    }
	
    
    
    /*if (m_ClientFrameCounter%m_nCallFPS)
	{
		m_ClientFPSDiffSum += currentTimeStamp - m_LastTimeStampClientFPS;

		{//Block for LOCK
			int  nApproximateAverageFrameInterval = m_ClientFPSDiffSum / m_ClientFrameCounter;
			if(nApproximateAverageFrameInterval > MINIMUM_CAPTURE_INTERVAL_TO_UPDATE_FPS)
            {
				Locker lock(*m_pVideoCallSessionMutex);
				m_pFPSController->SetClientFPS(1000 / nApproximateAverageFrameInterval);

			}
		}
	}*/
    
    
    
    

	m_LastTimeStampClientFPS = currentTimeStamp;

#if defined(TARGET_OS_IPHONE) || defined(TARGET_IPHONE_SIMULATOR)

	CLogPrinter_Write(CLogPrinter::INFO, "CVideoCallSession::PushIntoBufferForEncoding 2");
	//this->m_pColorConverter->ConvertNV12ToI420(m_EncodingFrame);

	CLogPrinter_Write(CLogPrinter::DEBUGS, "CVideoCallSession::PushIntoBufferForEncoding Converted to 420");

#endif

	if(-1 == m_llFirstFrameCapturingTimeStamp)
		m_llFirstFrameCapturingTimeStamp = currentTimeStamp;

	int nCaptureTimeDiff = currentTimeStamp - m_llFirstFrameCapturingTimeStamp;
    
    g_TimeTraceFromCaptureToSend[g_CapturingFrameCounter] = m_Tools.CurrentTimestamp();

    if(g_CapturingFrameCounter < 30)
        //printf("Frame %d --> Trying to Set --> %d..... Capture Time = %lld\n", g_CapturingFrameCounter, nCaptureTimeDiff, m_Tools.CurrentTimestamp());
    
    g_CapturingFrameCounter++;
    
	//CLogPrinter_WriteLog(CLogPrinter::INFO, INSTENT_TEST_LOG, "CVideoCallSession::PushIntoBufferForEncoding 504 -- size " + m_Tools.IntegertoStringConvert(in_size) + "  device_orient = "+ m_Tools.IntegertoStringConvert(device_orientation));

	int returnedValue = m_EncodingBuffer->Queue(in_data, in_size, nCaptureTimeDiff, device_orientation);
    
    //CLogPrinter_WriteLog(CLogPrinter::INFO, OPERATION_TIME_LOG || INSTENT_TEST_LOG, " nCaptureTimeDiff = " +  m_Tools.LongLongtoStringConvert(m_Tools.CurrentTimestamp() - mt_llCapturePrevTime));
    mt_llCapturePrevTime = m_Tools.CurrentTimestamp();
    
    
    
//	CLogPrinter_WriteInstentTestLog(CLogPrinter::INFO, "CVideoCallSession::PushIntoBufferForEncoding Queue packetSize " + Tools::IntegertoStringConvert(in_size));

	//CLogPrinter_WriteLog(CLogPrinter::INFO, INSTENT_TEST_LOG, "CVideoCallSession::PushIntoBufferForEncoding 513");

	return returnedValue;
}

CVideoDecoder* CVideoCallSession::GetVideoDecoder()
{
	//	return sessionMediaList.GetFromVideoEncoderList(mediaName);

	return m_pVideoDecoder;
}

CColorConverter* CVideoCallSession::GetColorConverter()
{
	return m_pColorConverter;
}

/*
void CVideoCallSession::ResetAllInMediaList()
{
sessionMediaList.ResetAllInVideoEncoderList();
}
*/

CEncodedFrameDepacketizer * CVideoCallSession::GetEncodedFrameDepacketizer()
{
	return m_pEncodedFrameDepacketizer;
}

void CVideoCallSession::CreateAndSendMiniPacket(int nByteReceivedOrNetworkType, int nMiniPacketType)
{
    if(m_bLiveVideoStreamRunning == true)
        return;
    
    if(m_bIsCheckCall != LIVE_CALL_MOOD) return;
    
	unsigned char uchVersion = (unsigned char)GetVersionController()->GetCurrentCallVersion();
    
	CVideoHeader PacketHeader;

	if (nMiniPacketType == __BITRATE_CONTROLL_PACKET_TYPE)
	{
		//PacketHeader.setPacketHeader(__BITRATE_CONTROLL_PACKET_TYPE, uchVersion, m_miniPacketBandCounter/*SlotID*/, 0, nMiniPacketType, nByteReceivedOrNetworkType/*Byte Received*/, 0, 0, 0, 0, 0);

		PacketHeader.setPacketHeader(__BITRATE_CONTROLL_PACKET_TYPE,			//packetType
									uchVersion,									//VersionCode
									VIDEO_HEADER_LENGTH,						//HeaderLength
									0,											//FPSByte
									m_miniPacketBandCounter,                    //FrameNumber           //SlotID
									0,											//NetworkType
									0,											//Device Orientation
									0,											//QualityLevel
									0,											//NumberofPacket
									nMiniPacketType,							//PacketNumber
									nByteReceivedOrNetworkType,					//TimeStamp             //Sending Received Byte
									0,											//PacketStartingIndex
									0,											//PacketDataLength
									0											//SenderDeviceType
									);
        
        printf("TheKing--> SlotID = %d, Received Byte = %d\n", m_miniPacketBandCounter, nByteReceivedOrNetworkType);
        PacketHeader.ShowDetails("BtratePacket SendingSide: ");
	}
	else if (nMiniPacketType == __NETWORK_INFO_PACKET_TYPE)
	{
		//PacketHeader.setPacketHeader(__NETWORK_INFO_PACKET_TYPE, uchVersion, m_miniPacketBandCounter/*SlotID*/, 0, nMiniPacketType, nByteReceivedOrNetworkType/*Network Type*/, 0, 0, 0, 0, 0);

		PacketHeader.setPacketHeader(__NETWORK_INFO_PACKET_TYPE,				//packetType
										uchVersion,								//VersionCode
										VIDEO_HEADER_LENGTH,					//HeaderLength
										0,										//FPSByte
										m_miniPacketBandCounter,                //FrameNumber
										0,										//NetworkType
										0,										//Device Orientation
										0,										//QualityLevel
										0,										//NumberofPacket
										nMiniPacketType,						//PacketNumber
										nByteReceivedOrNetworkType,				//TimeStamp
										0,										//PacketStartingIndex
										0,										//PacketDataLength
										0										//SenderDeviceType
										);
	}

	m_miniPacket[0] = (int)VIDEO_PACKET_MEDIA_TYPE;

	PacketHeader.GetHeaderInByteArray(m_miniPacket + 1);

#ifndef NO_CONNECTIVITY
	m_pCommonElementsBucket->SendFunctionPointer(m_lfriendID, MEDIA_TYPE_VIDEO, m_miniPacket, VIDEO_HEADER_LENGTH + 1, 0, std::vector< std::pair<int, int> >());
#else
	m_pCommonElementsBucket->m_pEventNotifier->fireAudioPacketEvent(200, VIDEO_HEADER_LENGTH + 1, m_miniPacket);
#endif
}

long long CVideoCallSession::GetShiftedTime()
{
	return m_llShiftedTime;
}
void CVideoCallSession::SetShiftedTime(long long llTime)
{
	m_llShiftedTime = llTime;
}

bool  CVideoCallSession::GetResolationCheck()
{
    
    return m_bResolationCheck;
}

void CVideoCallSession::SetCalculationStartMechanism(bool s7)
{
    m_bShouldStartCalculation = s7;
    if(m_bCaclculationStartTime == 0)
    {
        m_bCaclculationStartTime = m_Tools.CurrentTimestamp();
    }
}

bool CVideoCallSession::GetCalculationStatus()
{
    return m_bShouldStartCalculation;
}

long long CVideoCallSession::GetCalculationStartTime()
{
    return m_bCaclculationStartTime;
}

void CVideoCallSession::DecideHighResolatedVideo(bool bValue)
{
    m_bResolationCheck = true;
    
    if(bValue)
    {
        m_bHighResolutionSupportedForOwn = true;
//        CLogPrinter_WriteLog(CLogPrinter::INFO, INSTENT_TEST_LOG, "Decision Supported = " + m_Tools.IntegertoStringConvert(bValue));
        m_pDeviceCheckCapabilityBuffer->Queue(m_lfriendID, STOP_DEVICE_CHECK, DEVICE_CHECK_SUCCESS, m_nVideoCallHeight, m_nVideoCallWidth);
        
        
        
        
        //ReInitializeVideoLibrary(352, 288);
        //StopDeviceAbilityChecking();
        //m_pCommonElementsBucket->m_pEventNotifier->fireVideoNotificationEvent(m_lfriendID, m_pCommonElementsBucket->m_pEventNotifier->SET_CAMERA_RESOLUTION_640x480);
        
    }
    else
    {
        //Not Supported
        m_bHighResolutionSupportedForOwn = false;
//        CLogPrinter_WriteLog(CLogPrinter::INFO, INSTENT_TEST_LOG, "Decision NotSupported = " + m_Tools.IntegertoStringConvert(bValue));
        
        m_pDeviceCheckCapabilityBuffer->Queue(m_lfriendID, STOP_DEVICE_CHECK, DEVICE_CHECK_FAILED, m_nVideoCallHeight, m_nVideoCallWidth);
        
        
        
        
		//StopDeviceAbilityChecking();
        //m_pCommonElementsBucket->m_pEventNotifier->fireVideoNotificationEvent(m_lfriendID, m_pCommonElementsBucket->m_pEventNotifier->SET_CAMERA_RESOLUTION_352x288_25FPS);
        
        //ReInitializeVideoLibrary(352, 288);

		//if (m_nVideoCallWidth < 640)
			//*pnDeviceSupportedFPS = 15;


    }
}

bool CVideoCallSession::GetHighResolutionSupportStatus()
{
    return m_bHighResolutionSupportedForOwn;
}

void CVideoCallSession::SetOpponentHighResolutionSupportStatus(bool bValue)
{
    m_bHighResolutionSupportedForOpponent = bValue;
}

bool CVideoCallSession::GetOpponentHighResolutionSupportStatus()
{
    return m_bHighResolutionSupportedForOpponent;
}

bool CVideoCallSession::GetReinitializationStatus()
{
    return m_bReinitialized;
}


void CVideoCallSession::OperationForResolutionControl(unsigned char* in_data, int in_size)
{
    //Opponent resolution support checking
    CVideoHeader RcvPakcetHeader;
    int gotResSt = RcvPakcetHeader.GetOpponentResolution(in_data);
    
    if(gotResSt == 2)
    {
//        CLogPrinter_WriteLog(CLogPrinter::INFO, INSTENT_TEST_LOG, "Opponent High supported, flag =  "+ m_Tools.IntegertoStringConvert(gotResSt) );
        
        SetOpponentHighResolutionSupportStatus(true);
        m_bResolutionNegotiationDone = true;
        
        
    }
    else if(gotResSt == 1)
    {
//        CLogPrinter_WriteLog(CLogPrinter::INFO, INSTENT_TEST_LOG, "Opponent High NotSupported = "+ m_Tools.IntegertoStringConvert(gotResSt));
        
        
        SetOpponentHighResolutionSupportStatus(false);
        
        if(m_bHighResolutionSupportedForOwn == false || m_bHighResolutionSupportedForOpponent == false)
        {
            //printf("m_bReinitialized SET_CAMERA_RESOLUTION_352x288_OR_320x240  = %d\n", m_bReinitialized);
            
            //m_pCommonElementsBucket->m_pEventNotifier->fireVideoNotificationEvent(m_lfriendID, m_pCommonElementsBucket->m_pEventNotifier->SET_CAMERA_RESOLUTION_352x288_OR_320x240);
            
            //ReInitializeVideoLibrary(352, 288);
        }
        
        m_pVideoDecodingThread->Reset();
        m_bResolutionNegotiationDone = true;
        
    }
    else
    {
//        CLogPrinter_WriteLog(CLogPrinter::INFO, INSTENT_TEST_LOG, "Opponent not set = " + m_Tools.IntegertoStringConvert(gotResSt));
    }
    
    
    //End of  Opponent resolution support checking
}
CVersionController* CVideoCallSession::GetVersionController()
{
    return m_pVersionController;
    
}
bool CVideoCallSession::GetResolutionNegotiationStatus()
{
    return m_bResolutionNegotiationDone;
}

void CVideoCallSession::StopDeviceAbilityChecking()
{
	long long llReinitializationStartTime = m_Tools.CurrentTimestamp();

//	CLogPrinter_WriteLog(CLogPrinter::INFO, INSTENT_TEST_LOG, "Video call session destructor 1");
	m_pVideoEncodingThread->StopEncodingThread();
//	CLogPrinter_WriteLog(CLogPrinter::INFO, INSTENT_TEST_LOG, "Video call session destructor 2");
	m_pVideoDepacketizationThread->StopDepacketizationThread();
//	CLogPrinter_WriteLog(CLogPrinter::INFO, INSTENT_TEST_LOG, "Video call session destructor 4");
	m_pVideoDecodingThread->InstructionToStop();
//	CLogPrinter_WriteLog(CLogPrinter::INFO, INSTENT_TEST_LOG, "Video call session destructor 5");
	m_pVideoRenderingThread->StopRenderingThread();
//	CLogPrinter_WriteLog(CLogPrinter::INFO, INSTENT_TEST_LOG, "Video call session destructor 6");
}

int CVideoCallSession::GetOwnVideoCallQualityLevel(){
	return m_nOwnVideoCallQualityLevel;	
}

int CVideoCallSession::GetOpponentVideoCallQualityLevel(){
	return m_nOpponentVideoCallQualityLevel;
}

void CVideoCallSession::SetOwnVideoCallQualityLevel(int nVideoCallQualityLevel){
	m_nOwnVideoCallQualityLevel = nVideoCallQualityLevel;
}

void CVideoCallSession::SetOpponentVideoCallQualityLevel(int nVideoCallQualityLevel){
	m_nOpponentVideoCallQualityLevel = nVideoCallQualityLevel;
}

int CVideoCallSession::GetCurrentVideoCallQualityLevel(){
	return m_nCurrentVideoCallQualityLevel;
}

void CVideoCallSession::SetCurrentVideoCallQualityLevel(int nVideoCallQualityLevel)
{
	m_nCurrentVideoCallQualityLevel = nVideoCallQualityLevel;
    
    if(m_bIsCheckCall == true)
        return;

	if (m_nCurrentVideoCallQualityLevel == SUPPORTED_RESOLUTION_FPS_640_25)
	{
		m_nVideoCallHeight = m_pController->m_Quality[1].iHeight;
		m_nVideoCallWidth = m_pController->m_Quality[1].iWidth;
        
		m_nCallFPS = HIGH_QUALITY_FPS;
		m_SlotResetRightRange = HIGH_QUALITY_FPS;
	}
	else if (m_nCurrentVideoCallQualityLevel == SUPPORTED_RESOLUTION_FPS_352_25)
	{
        m_nVideoCallHeight = m_pController->m_Quality[0].iHeight;
        m_nVideoCallWidth = m_pController->m_Quality[0].iWidth;
		m_nCallFPS = HIGH_QUALITY_FPS;
		m_SlotResetRightRange = HIGH_QUALITY_FPS;
	}
	else if (m_nCurrentVideoCallQualityLevel == SUPPORTED_RESOLUTION_FPS_352_15)
	{
        m_nVideoCallHeight = m_pController->m_Quality[0].iHeight;
        m_nVideoCallWidth = m_pController->m_Quality[0].iWidth;
        
		m_nCallFPS = LOW_QUALITY_FPS;
		m_SlotResetRightRange = LOW_QUALITY_FPS;
	}
	else if (m_nCurrentVideoCallQualityLevel == RESOLUTION_FPS_SUPPORT_NOT_TESTED)
	{
        m_nVideoCallHeight = m_pController->m_Quality[0].iHeight;
        m_nVideoCallWidth = m_pController->m_Quality[0].iWidth;
		m_nCallFPS = LOW_QUALITY_FPS;
		m_SlotResetRightRange = LOW_QUALITY_FPS;
	}

	m_BitRateController->SetCallFPS(m_nCallFPS);
	m_pVideoEncodingThread->SetCallFPS(m_nCallFPS);
    m_pVideoEncodingThread->SetFrameNumber(m_nCallFPS);
	m_pVideoDecodingThread->SetCallFPS(m_nCallFPS);
    
    m_pFPSController->Reset(m_nCallFPS);

	this->m_pColorConverter->SetHeightWidth(m_nVideoCallHeight, m_nVideoCallWidth);

	if (m_nServiceType == SERVICE_TYPE_LIVE_STREAM || m_nServiceType == SERVICE_TYPE_SELF_STREAM || m_nServiceType == SERVICE_TYPE_CHANNEL)
		this->m_pVideoEncoder->SetHeightWidth(m_nVideoCallHeight, m_nVideoCallWidth, m_nCallFPS, m_nCallFPS / IFRAME_INTERVAL, m_bIsCheckCall, m_nServiceType);
    else
		this->m_pVideoEncoder->SetHeightWidth(m_nVideoCallHeight, m_nVideoCallWidth, m_nCallFPS, m_nCallFPS / 2 + 1, m_bIsCheckCall, m_nServiceType);



	m_pVideoEncodingThread->SetNotifierFlag(true);
}

BitRateController* CVideoCallSession::GetBitRateController(){
	return m_BitRateController;
}

int CVideoCallSession::SetEncoderHeightWidth(const LongLong& lFriendID, int height, int width)
{
	if(m_nVideoCallHeight != height || m_nVideoCallWidth != width)
	{
		m_nVideoCallHeight = height;
		m_nVideoCallWidth = width;

		this->m_pColorConverter->SetHeightWidth(height, width);

		if (m_nServiceType == SERVICE_TYPE_LIVE_STREAM || m_nServiceType == SERVICE_TYPE_SELF_STREAM || m_nServiceType == SERVICE_TYPE_CHANNEL)
			this->m_pVideoEncoder->SetHeightWidth(height, width, m_nCallFPS, m_nCallFPS / IFRAME_INTERVAL, m_bIsCheckCall, m_nServiceType);
		else
			this->m_pVideoEncoder->SetHeightWidth(height, width, m_nCallFPS, m_nCallFPS / 2 + 1, m_bIsCheckCall, m_nServiceType);

		return 1;
	}
	else
	{
		return -1;
	}
}

int CVideoCallSession::SetVideoEffect(int nEffectStatus)
{
	if (nEffectStatus == 1)
		m_bVideoEffectEnabled = true;
	else if (nEffectStatus == 0)
		m_bVideoEffectEnabled = false;

	this->m_pVideoEncodingThread->SetVideoEffect(nEffectStatus);
	return 1;
}

int CVideoCallSession::TestVideoEffect( int *param, int size)
{
	this->m_pVideoEncodingThread->TestVideoEffect(param, size);
	return 1;
}

void CVideoCallSession::SetOwnDeviceType(int deviceType)
{
	m_nOwnDeviceType = deviceType;
}

int CVideoCallSession::GetOwnDeviceType()
{
	return m_nOwnDeviceType;
}

void CVideoCallSession::SetOponentDeviceType(int deviceType)
{
	m_nOponentDeviceType = deviceType;
}

int CVideoCallSession::GetOponentDeviceType()
{
	return m_nOponentDeviceType;
}

int CVideoCallSession::SetDeviceHeightWidth(const LongLong& lFriendID, int height, int width)
{
	m_nDeviceHeight = height;
	m_nDeviceWidth = width;

	this->m_pColorConverter->SetDeviceHeightWidth(height, width);

	return 1;
}

void CVideoCallSession::InterruptOccured()
{
	m_pSendingThread->InterruptOccured();
}

void CVideoCallSession::InterruptOver()
{
	m_pSendingThread->InterruptOver();
}

void CVideoCallSession::ReInitializeVideoLibrary(int iHeight, int iWidth)
{
    return;
    
    //printf("Reinitializing........\n");
    long long llReinitializationStartTime = m_Tools.CurrentTimestamp();
    

    
//    CLogPrinter_WriteLog(CLogPrinter::INFO, INSTENT_TEST_LOG, "Video call session destructor 1");
    m_pVideoEncodingThread->StopEncodingThread();
//    CLogPrinter_WriteLog(CLogPrinter::INFO, INSTENT_TEST_LOG, "Video call session destructor 2");
    m_pVideoDepacketizationThread->StopDepacketizationThread();
//    CLogPrinter_WriteLog(CLogPrinter::INFO, INSTENT_TEST_LOG, "Video call session destructor 4");
    //m_pVideoDecodingThread->StopDecodingThread();
//    CLogPrinter_WriteLog(CLogPrinter::INFO, INSTENT_TEST_LOG, "Video call session destructor 5");
    m_pVideoRenderingThread->StopRenderingThread();
//    CLogPrinter_WriteLog(CLogPrinter::INFO, INSTENT_TEST_LOG, "Video call session destructor 6");
    
	if (m_nServiceType == SERVICE_TYPE_LIVE_STREAM || m_nServiceType == SERVICE_TYPE_SELF_STREAM || m_nServiceType == SERVICE_TYPE_CHANNEL)
		m_pVideoEncoder->CreateVideoEncoder(iHeight, iWidth, m_nCallFPS, m_nCallFPS / IFRAME_INTERVAL, m_bIsCheckCall, m_nServiceType);
    else
		m_pVideoEncoder->CreateVideoEncoder(iHeight, iWidth, m_nCallFPS, m_nCallFPS / 2 + 1, m_bIsCheckCall, m_nServiceType);



	m_pColorConverter->SetHeightWidth(iHeight, iWidth);

	m_SendingBuffer->ResetBuffer();
	//g_FPSController.Reset(m_nCallFPS;);
	m_EncodingBuffer->ResetBuffer();
	//m_BitRateController
	m_RenderingBuffer->ResetBuffer();
	m_pVideoPacketQueue->ResetBuffer();
	m_pMiniPacketQueue->ResetBuffer();

	m_BitRateController = new BitRateController(m_nCallFPS, m_lfriendID);
    m_BitRateController->SetEncoder(m_pVideoEncoder);
    m_BitRateController->SetSharedObject(m_pCommonElementsBucket);
    
    m_pVideoEncodingThread->ResetVideoEncodingThread(m_BitRateController);
    
//  m_pSendingThread = new CSendingThread(m_pCommonElementsBucket, m_SendingBuffer, &g_FPSController, this);
//	m_pVideoEncodingThread = new CVideoEncodingThread(m_lfriendID, m_EncodingBuffer, m_BitRateController, m_pColorConverter, m_pVideoEncoder, m_pEncodedFramePacketizer, this);
//	m_pVideoRenderingThread = new CVideoRenderingThread(m_lfriendID, m_RenderingBuffer, m_pCommonElementsBucket, this);
//   m_pVideoDecodingThread = new CVideoDecodingThread(m_pEncodedFrameDepacketizer, m_RenderingBuffer, m_pVideoDecoder, m_pColorConverter, &g_FPSController, this);
    
	m_pVideoDepacketizationThread = new CVideoDepacketizationThread(m_lfriendID, m_pVideoPacketQueue, m_pRetransVideoPacketQueue, m_pMiniPacketQueue, m_BitRateController, m_pEncodedFrameDepacketizer, m_pCommonElementsBucket, &m_miniPacketBandCounter, m_pVersionController, this);
    

    
    g_CapturingFrameCounter = 0;
    
    m_pVideoEncodingThread->StartEncodingThread();
    m_pVideoRenderingThread->StartRenderingThread();
    m_pVideoDepacketizationThread->StartDepacketizationThread();
    //m_pVideoDecodingThread->StartDecodingThread();
    m_pVideoDecodingThread->Reset();
    
    m_bReinitialized = true;
    
    //printf("TheKing--> Reinitialization time = %lld\n",m_Tools.CurrentTimestamp() - llReinitializationStartTime);
    
    
}

CFPSController* CVideoCallSession::GetFPSController()
{
	return m_pFPSController;
}

bool CVideoCallSession::isLiveVideoStreamRunning()
{
    return m_bLiveVideoStreamRunning;
}
int CVideoCallSession::GetServiceType()
{
    return m_nServiceType;  
}

int CVideoCallSession::GetEntityType()
{
	return m_nEntityType;
}

void CVideoCallSession::StartCallInLive()
{
	if (m_iRole != 0)
		return;
	else
	{
		if (m_nEntityType == ENTITY_TYPE_PUBLISHER)
		{
			SetFirstVideoPacketTime(-1);
			SetShiftedTime(-1);

			m_nEntityType = ENTITY_TYPE_PUBLISHER_CALLER;
		}
		else if (m_nEntityType == ENTITY_TYPE_VIEWER)
		{
			m_pVideoDecodingThread->ResetForViewerCallerCallStartEnd();

			m_nEntityType = ENTITY_TYPE_VIEWER_CALLEE;
		}

		m_iRole = 1;
	}
}

void CVideoCallSession::EndCallInLive()
{
	if (m_iRole != 1)
		return;
	else
	{
		if (m_nEntityType == ENTITY_TYPE_PUBLISHER_CALLER)
		{
			//m_pVideoDepacketizationThread->ResetForPublisherCallerCallEnd();

			SetFirstVideoPacketTime(-1);
			SetShiftedTime(-1);

			m_pVideoDecodingThread->ResetForPublisherCallerCallEnd();

			m_pColorConverter->ClearSmallScreen();

			//m_pCommonElementsBucket->m_pEventNotifier->fireVideoNotificationEvent(m_lfriendID, m_pCommonElementsBucket->m_pEventNotifier->LIVE_CALL_INSET_OFF);

			m_nEntityType = ENTITY_TYPE_PUBLISHER;
		}
		else if (m_nEntityType == ENTITY_TYPE_VIEWER_CALLEE)
		{
			m_pVideoDecodingThread->ResetForViewerCallerCallStartEnd();
			m_pVideoEncodingThread->ResetForViewerCallerCallEnd();

			m_pColorConverter->ClearSmallScreen();

			//m_pCommonElementsBucket->m_pEventNotifier->fireVideoNotificationEvent(m_lfriendID, m_pCommonElementsBucket->m_pEventNotifier->LIVE_CALL_INSET_OFF);

			m_nEntityType = ENTITY_TYPE_VIEWER;
		}

		m_iRole = 0;
	}
}

void CVideoCallSession::SetOpponentVideoHeightWidth(int iHeight, int iWidth)
{
    m_nOpponentVideoHeight = iHeight;
    m_nOpponentVideoWidth = iWidth;
}

int CVideoCallSession::GetOpponentVideoHeight()
{
    return m_nOpponentVideoHeight;
}

int CVideoCallSession::GetOpponentVideoWidth()
{
    return m_nOpponentVideoWidth;
}
