// test comment
#include "VideoCallSession.h"
#include "CommonElementsBucket.h"
#include "LogPrinter.h"

#include "Controller.h"
#include "InterfaceOfAudioVideoEngine.h"

//PairMap g_timeInt;

#if defined(TARGET_OS_IPHONE) || defined(TARGET_IPHONE_SIMULATOR)
#include <dispatch/dispatch.h>
#endif

#define MINIMUM_CAPTURE_INTERVAL_TO_UPDATE_FPS 10

namespace MediaSDK
{

CVideoCallSession::CVideoCallSession(CController *pController, long long fname, CCommonElementsBucket* sharedObject, int nFPS, int *nrDeviceSupportedCallFPS, bool bIsCheckCall, CDeviceCapabilityCheckBuffer *deviceCheckCapabilityBuffer, int nOwnSupportedResolutionFPSLevel, int nServiceType, int nEntityType, bool bAudioOnlyLive, bool bSelfViewOnly) :

m_pCommonElementsBucket(sharedObject),
m_ClientFPS(DEVICE_FPS_MAXIMUM),
m_ClientFPSDiffSum(0),
m_ClientFrameCounter(0),
m_EncodingFrameCounter(0),
m_pEncodedFramePacketizer(NULL),
m_ByteRcvInBandSlot(0),
m_llSlotResetLeftRange(0),
m_llSlotResetRightRange(nFPS),
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
m_bLiveVideoQuality(VIDEO_QUALITY_HIGH),
m_nCapturedFrameCounter(0),
m_nServiceType(nServiceType),
m_nEntityType(nEntityType),
m_bFrameReduce(false),
m_nReduceCheckNumber(30),
m_iRole(0),
m_bVideoEffectEnabled(true),
m_nOponentDeviceType(DEVICE_TYPE_UNKNOWN),
m_nOpponentVideoHeight(-1),
m_nOpponentVideoWidth(-1),
m_bAudioOnlyLive(bAudioOnlyLive),
m_bVideoOnlyLive(false),
m_nCallInLiveType(CALL_IN_LIVE_TYPE_AUDIO_VIDEO),
m_bSelfViewOnly(bSelfViewOnly),
m_nFrameCount(0),

#ifdef OLD_SENDING_THREAD

m_pSendingThread(NULL),

#else

m_pSendingThreadOfCall(NULL),
m_pSendingThreadOfLive(NULL),

#endif

#ifdef OLD_ENCODING_THREAD

m_pVideoEncodingThread(NULL),

#else

m_pVideoEncodingThreadOfCall(NULL),
m_pVideoEncodingThreadOfLive(NULL),

#endif

#ifdef OLD_RENDERING_THREAD

m_pVideoRenderingThread(NULL),

#else

m_pRenderingThreadOfCall(NULL),
m_pRenderingThreadOfLive(NULL),
m_pRenderingThreadOfChannel(NULL),

#endif

#ifdef OLD_DECODING_THREAD

m_pVideoDecodingThread(NULL),

#else

m_pVideoDecodingThreadOfCall(NULL),
m_pVideoDecodingThreadOfLive(NULL),
m_pVideoDecodingThreadOfChannel(NULL),

#endif

m_nPublisherInsetNumber(0)

{

#if defined(TARGET_OS_IPHONE) || defined(TARGET_IPHONE_SIMULATOR)

	m_nOwnDeviceType = DEVICE_TYPE_IOS;

#elif defined(DESKTOP_C_SHARP)

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
		m_pLiveReceiverVideo = new LiveReceiver(m_pCommonElementsBucket);
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
    m_pIdrFrameIntervalController = new IDRFrameIntervalController();

	m_BitRateController->SetSharedObject(sharedObject);
    
    m_bDynamic_IDR_Sending_Mechanism = true;
    
	//CLogPrinter_WriteLog(CLogPrinter::INFO, INSTENT_TEST_LOG, "CVideoCallSession::CVideoCallSession 90");
}

CVideoCallSession::~CVideoCallSession()
{
	//CLogPrinter_WriteLog(CLogPrinter::INFO, INSTENT_TEST_LOG, "CVideoCallSession::~~~CVideoCallSession 95");

#ifdef	OLD_ENCODING_THREAD

	if (m_pVideoEncodingThread != NULL)
		m_pVideoEncodingThread->StopEncodingThread();

#else

	if (m_pVideoEncodingThreadOfCall != NULL)
		m_pVideoEncodingThreadOfCall->StopEncodingThread();
	else if (m_pVideoEncodingThreadOfLive != NULL)
		m_pVideoEncodingThreadOfLive->StopEncodingThread();

#endif

#ifdef	OLD_SENDING_THREAD

	if (m_pSendingThread != NULL)
		m_pSendingThread->StopSendingThread();

#else

	if (m_pSendingThreadOfCall != NULL)
		m_pSendingThreadOfCall->StopSendingThread();
	else if(m_pSendingThreadOfLive != NULL)
		m_pSendingThreadOfLive->StopSendingThread();

#endif

	m_pVideoDepacketizationThread->StopDepacketizationThread();

#ifdef	OLD_DECODING_THREAD

	if (m_pVideoDecodingThread != NULL)
		m_pVideoDecodingThread->StopDecodingThread();
	
#else

	if (m_pVideoDecodingThreadOfCall != NULL)
		m_pVideoDecodingThreadOfCall->StopDecodingThread();
	else if (m_pVideoDecodingThreadOfLive != NULL)
		m_pVideoDecodingThreadOfLive->StopDecodingThread();
	else if (m_pVideoDecodingThreadOfChannel != NULL)
		m_pVideoDecodingThreadOfChannel->StopDecodingThread();

#endif


#ifdef OLD_RENDERING_THREAD

	if (m_pVideoRenderingThread != NULL)
		m_pVideoRenderingThread->StopRenderingThread();
	
#else

	if (m_pRenderingThreadOfCall != NULL)
		m_pRenderingThreadOfCall->StopRenderingThread();
	else if (m_pRenderingThreadOfLive != NULL)
		m_pRenderingThreadOfLive->StopRenderingThread();
	else if (m_pRenderingThreadOfChannel != NULL)
		m_pRenderingThreadOfChannel->StopRenderingThread();

#endif

#ifdef	OLD_ENCODING_THREAD

	if (NULL != m_pVideoEncodingThread)
	{
		delete m_pVideoEncodingThread;
		m_pVideoEncodingThread = NULL;
	}

#else

	if (NULL != m_pVideoEncodingThreadOfCall)
	{
		delete m_pVideoEncodingThreadOfCall;
		m_pVideoEncodingThreadOfCall = NULL;
	}

	if (NULL != m_pVideoEncodingThreadOfLive)
	{
		delete m_pVideoEncodingThreadOfLive;
		m_pVideoEncodingThreadOfLive = NULL;
	}

#endif

	if (NULL != m_pVideoDepacketizationThread)
	{
		delete m_pVideoDepacketizationThread;
		m_pVideoDepacketizationThread = NULL;
	}

#ifdef	OLD_DECODING_THREAD

	if (NULL != m_pVideoDecodingThread)
	{
		delete m_pVideoDecodingThread;
		m_pVideoDecodingThread = NULL;
	}

#else

	if (NULL != m_pVideoDecodingThreadOfCall)
	{
		delete m_pVideoDecodingThreadOfCall;
		m_pVideoDecodingThreadOfCall = NULL;
	}

	if (NULL != m_pVideoDecodingThreadOfLive)
	{
		delete m_pVideoDecodingThreadOfLive;
		m_pVideoDecodingThreadOfLive = NULL;
	}

	if (NULL != m_pVideoDecodingThreadOfChannel)
	{
		delete m_pVideoDecodingThreadOfChannel;
		m_pVideoDecodingThreadOfChannel = NULL;
	}

#endif


#ifdef OLD_RENDERING_THREAD

	if (NULL != m_pVideoRenderingThread)
	{
		delete m_pVideoRenderingThread;
		m_pVideoRenderingThread = NULL;
	}

#else

	if (NULL != m_pRenderingThreadOfCall)
	{
		delete m_pRenderingThreadOfCall;
		m_pRenderingThreadOfCall = NULL;
	}

	if (NULL != m_pRenderingThreadOfLive)
	{
		delete m_pRenderingThreadOfLive;
		m_pRenderingThreadOfLive = NULL;
	}

	if (NULL != m_pRenderingThreadOfChannel)
	{
		delete m_pRenderingThreadOfChannel;
		m_pRenderingThreadOfChannel = NULL;
	}

#endif

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

#ifdef OLD_SENDING_THREAD

	if (NULL != m_pSendingThread)
	{
		delete m_pSendingThread;
		m_pSendingThread = NULL;
	}

#else

	if (NULL != m_pSendingThreadOfCall)
	{
		delete m_pSendingThreadOfCall;
		m_pSendingThreadOfCall = NULL;
	}

	if (NULL != m_pSendingThreadOfLive)
	{
		delete m_pSendingThreadOfLive;
		m_pSendingThreadOfLive = NULL;
	}

#endif

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

long long CVideoCallSession::GetFriendID()
{
	return m_lfriendID;
}

void CVideoCallSession::InitializeVideoSession(long long lFriendID, int iVideoHeight, int iVideoWidth, int nServiceType, int iNetworkType)
{

	//CLogPrinter_WriteLog(CLogPrinter::INFO, INSTENT_TEST_LOG, "CVideoCallSession::InitializeVideoSession 232");
    m_nServiceType = nServiceType;
    
    iVideoHeight /= 4;
    iVideoWidth /= 4;
	m_nVideoCallHeight = iVideoHeight;
	m_nVideoCallWidth = iVideoWidth;
    
    printf("Tahmid---> H:W = %d:%d\n",m_nVideoCallHeight,m_nVideoCallWidth);

    
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

	//CLogPrinter_WriteLog(CLogPrinter::INFO, INSTENT_TEST_LOG, "CVideoCallSession::InitializeVideoSession 240");

	if (sessionMediaList.IsVideoEncoderExist(iVideoHeight, iVideoWidth))
	{
		return;
	}

	//CLogPrinter_WriteLog(CLogPrinter::INFO, INSTENT_TEST_LOG, "CVideoCallSession::InitializeVideoSession 247");

	this->m_pVideoEncoder = new CVideoEncoder(m_pCommonElementsBucket, m_lfriendID);

	if (m_nServiceType == SERVICE_TYPE_LIVE_STREAM || m_nServiceType == SERVICE_TYPE_SELF_STREAM || m_nServiceType == SERVICE_TYPE_CHANNEL)
		m_pVideoEncoder->CreateVideoEncoder(iVideoHeight, iVideoWidth, m_nCallFPS, IFRAME_INTERVAL, m_bIsCheckCall, nServiceType);
	else
    {
        if(m_bDynamic_IDR_Sending_Mechanism == true)
        {
            m_pVideoEncoder->CreateVideoEncoder(iVideoHeight, iVideoWidth, m_nCallFPS, m_nCallFPS * 2, m_bIsCheckCall, nServiceType);
        }
        else
        {
            m_pVideoEncoder->CreateVideoEncoder(iVideoHeight, iVideoWidth, m_nCallFPS, m_nCallFPS / 2 + 1, m_bIsCheckCall, nServiceType);
        }
    }


	if (m_bAudioOnlyLive == true)
	{
		m_pVideoEncoder->SetBitrate(BITRATE_FOR_INSET_STREAM);
		m_pVideoEncoder->SetMaxBitrate(BITRATE_FOR_INSET_STREAM);
	}

	m_pFPSController->SetEncoder(m_pVideoEncoder);
	m_BitRateController->SetEncoder(m_pVideoEncoder);

	this->m_pVideoDecoder = new CVideoDecoder(m_pCommonElementsBucket);

	m_pVideoDecoder->CreateVideoDecoder();

	this->m_pColorConverter = new CColorConverter(iVideoHeight, iVideoWidth, m_pCommonElementsBucket, m_lfriendID);

	this->m_pColorConverter->SetDeviceHeightWidth(m_nDeviceHeight, m_nDeviceWidth);

	//CLogPrinter_WriteLog(CLogPrinter::INFO, INSTENT_TEST_LOG, "CVideoCallSession::InitializeVideoSession 262");

	// SendingThreads

#ifdef OLD_SENDING_THREAD

	m_pSendingThread = new CSendingThread(m_pCommonElementsBucket, m_SendingBuffer, this, m_bIsCheckCall, m_lfriendID, m_bAudioOnlyLive);	// previous

#else

	if (nServiceType == SERVICE_TYPE_CALL || nServiceType == SERVICE_TYPE_SELF_CALL)
		m_pSendingThreadOfCall = new CSendingThreadOfCall(m_pCommonElementsBucket, m_SendingBuffer, this, m_bIsCheckCall, m_lfriendID, m_bAudioOnlyLive);
	else if (nServiceType == SERVICE_TYPE_LIVE_STREAM || nServiceType == SERVICE_TYPE_SELF_STREAM)
		m_pSendingThreadOfLive = new CSendingThreadOfLive(m_pCommonElementsBucket, m_SendingBuffer, this, m_bIsCheckCall, m_lfriendID, m_bAudioOnlyLive);

#endif

	// EncodingThreads

#ifdef	OLD_ENCODING_THREAD

	m_pVideoEncodingThread = new CVideoEncodingThread(lFriendID, m_EncodingBuffer, m_pCommonElementsBucket, m_BitRateController, m_pIdrFrameIntervalController, m_pColorConverter, m_pVideoEncoder, m_pEncodedFramePacketizer, this, m_nCallFPS, m_bIsCheckCall, m_bSelfViewOnly); // previous

#else

	if (nServiceType == SERVICE_TYPE_CALL || nServiceType == SERVICE_TYPE_SELF_CALL)
		m_pVideoEncodingThreadOfCall = new CVideoEncodingThreadOfCall(lFriendID, m_EncodingBuffer, m_pCommonElementsBucket, m_BitRateController, m_pIdrFrameIntervalController, m_pColorConverter, m_pVideoEncoder, m_pEncodedFramePacketizer, this, m_nCallFPS, m_bIsCheckCall, m_bSelfViewOnly);
	else if (nServiceType == SERVICE_TYPE_LIVE_STREAM || nServiceType == SERVICE_TYPE_SELF_STREAM)
		m_pVideoEncodingThreadOfLive = new CVideoEncodingThreadOfLive(lFriendID, m_EncodingBuffer, m_pCommonElementsBucket, m_BitRateController, m_pIdrFrameIntervalController, m_pColorConverter, m_pVideoEncoder, m_pEncodedFramePacketizer, this, m_nCallFPS, m_bIsCheckCall, m_bSelfViewOnly);

#endif

	// RenderingThreads
	
#ifdef OLD_RENDERING_THREAD

	m_pVideoRenderingThread = new CVideoRenderingThread(lFriendID, m_RenderingBuffer, m_pCommonElementsBucket, this, m_bIsCheckCall); // previous

#else
	
	if (nServiceType == SERVICE_TYPE_CALL || nServiceType == SERVICE_TYPE_SELF_CALL)
		m_pRenderingThreadOfCall = new CRenderingThreadOfCall(lFriendID, m_RenderingBuffer, m_pCommonElementsBucket, this, m_bIsCheckCall);
	else if (nServiceType == SERVICE_TYPE_LIVE_STREAM || nServiceType == SERVICE_TYPE_SELF_STREAM)
		m_pRenderingThreadOfLive = new CRenderingThreadOfLive(lFriendID, m_RenderingBuffer, m_pCommonElementsBucket, this, m_bIsCheckCall);
	else if (nServiceType == SERVICE_TYPE_CHANNEL)
		m_pRenderingThreadOfChannel = new CRenderingThreadOfChannel(lFriendID, m_RenderingBuffer, m_pCommonElementsBucket, this, m_bIsCheckCall);

#endif

	// DecodingThreads

#ifdef OLD_DECODING_THREAD

	m_pVideoDecodingThread = new CVideoDecodingThread(m_pEncodedFrameDepacketizer, lFriendID, m_pCommonElementsBucket, m_RenderingBuffer, m_pLiveVideoDecodingQueue, m_pVideoDecoder, m_pColorConverter, this, m_bIsCheckCall, m_nCallFPS); // previous
	
#else

	if (nServiceType == SERVICE_TYPE_CALL || nServiceType == SERVICE_TYPE_SELF_CALL)
		m_pVideoDecodingThreadOfCall = new CVideoDecodingThreadOfCall(m_pEncodedFrameDepacketizer, lFriendID, m_pCommonElementsBucket, m_RenderingBuffer, m_pLiveVideoDecodingQueue, m_pVideoDecoder, m_pColorConverter, this, m_bIsCheckCall, m_nCallFPS);
	else if (nServiceType == SERVICE_TYPE_LIVE_STREAM || nServiceType == SERVICE_TYPE_SELF_STREAM)
		m_pVideoDecodingThreadOfLive = new CVideoDecodingThreadOfLive(m_pEncodedFrameDepacketizer, lFriendID, m_pCommonElementsBucket, m_RenderingBuffer, m_pLiveVideoDecodingQueue, m_pVideoDecoder, m_pColorConverter, this, m_bIsCheckCall, m_nCallFPS);
	else if (nServiceType == SERVICE_TYPE_CHANNEL)
		m_pVideoDecodingThreadOfChannel = new CVideoDecodingThreadOfChannel(m_pEncodedFrameDepacketizer, lFriendID, m_pCommonElementsBucket, m_RenderingBuffer, m_pLiveVideoDecodingQueue, m_pVideoDecoder, m_pColorConverter, this, m_bIsCheckCall, m_nCallFPS);

#endif

	m_pVideoDepacketizationThread = new CVideoDepacketizationThread(lFriendID, m_pVideoPacketQueue, m_pRetransVideoPacketQueue, m_pMiniPacketQueue, m_BitRateController, m_pIdrFrameIntervalController, m_pEncodedFrameDepacketizer, m_pCommonElementsBucket, &m_miniPacketBandCounter, m_pVersionController, this);

	//CLogPrinter_WriteLog(CLogPrinter::INFO, INSTENT_TEST_LOG, "CVideoCallSession::InitializeVideoSession 270");

	m_pCommonElementsBucket->m_pVideoEncoderList->AddToVideoEncoderList(lFriendID, m_pVideoEncoder);

    //CLogPrinter_WriteLog(CLogPrinter::INFO, INSTENT_TEST_LOG, "CVideoCallSession::InitializeVideoSession 274");
	m_ClientFrameCounter = 0;
	m_EncodingFrameCounter = 0;
	m_llFirstFrameCapturingTimeStamp = -1;


	m_BitRateController->SetOwnNetworkType(iNetworkType);
	//CLogPrinter_WriteLog(CLogPrinter::INFO, INSTENT_TEST_LOG, "CVideoCallSession::InitializeVideoSession 281");
	//CreateAndSendMiniPacket(iNetworkType, NETWORK_INFO_PACKET_TYPE);

	//CLogPrinter_WriteLog(CLogPrinter::INFO, INSTENT_TEST_LOG, "CVideoCallSession::InitializeVideoSession 282");

#ifdef OLD_SENDING_THREAD

	m_pSendingThread->StartSendingThread();

#else

	if (nServiceType == SERVICE_TYPE_CALL || nServiceType == SERVICE_TYPE_SELF_CALL)
		m_pSendingThreadOfCall->StartSendingThread();
	else if (nServiceType == SERVICE_TYPE_LIVE_STREAM || nServiceType == SERVICE_TYPE_SELF_STREAM)
		m_pSendingThreadOfLive->StartSendingThread();

#endif

	//CLogPrinter_WriteLog(CLogPrinter::INFO, INSTENT_TEST_LOG, "CVideoCallSession::InitializeVideoSession 286");

#ifdef	OLD_ENCODING_THREAD

	m_pVideoEncodingThread->StartEncodingThread();

#else

	if (nServiceType == SERVICE_TYPE_CALL || nServiceType == SERVICE_TYPE_SELF_CALL)
		m_pVideoEncodingThreadOfCall->StartEncodingThread();
	else if (nServiceType == SERVICE_TYPE_LIVE_STREAM || nServiceType == SERVICE_TYPE_SELF_STREAM)
		m_pVideoEncodingThreadOfLive->StartEncodingThread();

#endif

#ifdef OLD_RENDERING_THREAD

	m_pVideoRenderingThread->StartRenderingThread();

#else

	if (nServiceType == SERVICE_TYPE_CALL || nServiceType == SERVICE_TYPE_SELF_CALL)
		m_pRenderingThreadOfCall->StartRenderingThread();
	else if (nServiceType == SERVICE_TYPE_LIVE_STREAM || nServiceType == SERVICE_TYPE_SELF_STREAM)
		m_pRenderingThreadOfLive->StartRenderingThread();
	else if (nServiceType == SERVICE_TYPE_CHANNEL)
		m_pRenderingThreadOfChannel->StartRenderingThread();

#endif

	m_pVideoDepacketizationThread->StartDepacketizationThread();

#ifdef OLD_DECODING_THREAD

	m_pVideoDecodingThread->StartDecodingThread();

#else

	if (nServiceType == SERVICE_TYPE_CALL || nServiceType == SERVICE_TYPE_SELF_CALL)
		m_pVideoDecodingThreadOfCall->StartDecodingThread();
	else if (nServiceType == SERVICE_TYPE_LIVE_STREAM || nServiceType == SERVICE_TYPE_SELF_STREAM)
		m_pVideoDecodingThreadOfLive->StartDecodingThread();
	else if (nServiceType == SERVICE_TYPE_CHANNEL)
		m_pVideoDecodingThreadOfChannel->StartDecodingThread();

#endif

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
		if (m_nEntityType != ENTITY_TYPE_PUBLISHER)
			m_pLiveReceiverVideo->PushVideoDataVector(offset, in_data, in_size, numberOfFrames, frameSizes, vMissingFrames, m_nServiceType);

		return true;
	}

	return true;
}

bool CVideoCallSession::PushPacketForMerging(unsigned char *in_data, unsigned int in_size, bool bSelfData, int numberOfFrames, int *frameSizes, int numberOfMissingFrames, int *missingFrames)
{
	//CLogPrinter_WriteLog(CLogPrinter::INFO, INSTENT_TEST_LOG, "CVideoCallSession::PushPacketForMerging 326");
	unsigned char uchPacketType = in_data[PACKET_TYPE_INDEX];
	if(uchPacketType < MIN_PACKET_TYPE || MAX_PACKET_TYPE < uchPacketType)
		return false;


	if (BITRATE_CONTROLL_PACKET_TYPE == uchPacketType || NETWORK_INFO_PACKET_TYPE == uchPacketType || IDR_FRAME_CONTROL_INFO_TYPE == uchPacketType) // It is a minipacket
	{
		//CLogPrinter_WriteLog(CLogPrinter::INFO, INSTENT_TEST_LOG, "CVideoCallSession::PushPacketForMerging 334");
		m_pMiniPacketQueue->Queue(in_data, in_size);
	}	
	else if (VIDEO_PACKET_TYPE == uchPacketType)
	{
        /*if(bSelfData == false && m_bResolutionNegotiationDone == false)
        {
            OperationForResolutionControl(in_data,in_size);
        }*/
        m_PacketHeader.SetPacketHeader(in_data);
        
		long long llFrameNumber = m_PacketHeader.GetFrameNumber();
        
//		VLOG("#DR# --------------------------> FrameNumber : "+Tools::IntegertoStringConvert(unFrameNumber));
        //printf("PushPacketForMerging--> nFrameNumber = %d, m_nCallFPS = %d, m_PacketHeader.GetHeaderLength() = %d\n", unFrameNumber, m_nCallFPS, m_PacketHeader.GetHeaderLength());
        

		if (llFrameNumber >= m_llSlotResetLeftRange && llFrameNumber < m_llSlotResetRightRange)
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
				m_miniPacketBandCounter = (unsigned int)(m_llSlotResetLeftRange / m_nCallFPS);
//				VLOG("#DR# -----------------+++++++++------> m_miniPacketBandCounter : "+Tools::IntegertoStringConvert(m_miniPacketBandCounter));
//                CLogPrinter_WriteLog(CLogPrinter::INFO, INSTENT_TEST_LOG, "ReceivingSide: SlotIndex = " + m_Tools.IntegertoStringConvert(m_miniPacketBandCounter) + ", ReceivedBytes = " + m_Tools.IntegertoStringConvert(m_ByteRcvInBandSlot));

				CreateAndSendMiniPacket(m_ByteRcvInBandSlot, BITRATE_CONTROLL_PACKET_TYPE);
			}

			m_llSlotResetLeftRange = llFrameNumber - (llFrameNumber % m_nCallFPS);
            
            
			m_llSlotResetRightRange = m_llSlotResetLeftRange + m_nCallFPS;

			m_ByteRcvInBandSlot = in_size - m_PacketHeader.GetHeaderLength();
		}

		m_pVideoPacketQueue->Queue(in_data, in_size);

		//CLogPrinter_WriteLog(CLogPrinter::INFO, INSTENT_TEST_LOG, "CVideoCallSession::PushPacketForMerging 378  in_size= " + m_Tools.IntegertoStringConvert(in_size));
	}
	else if (NEGOTIATION_PACKET_TYPE == uchPacketType)
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
	LOGE_MAIN("CVideoCallSession::PushIntoBufferForEncoding starts in_size = %d", in_size);
    //CLogPrinter_WriteLog(CLogPrinter::INFO, INSTENT_TEST_LOG, "CVideoCallSession::PushIntoBufferForEncoding 1");

	if ((m_nServiceType == SERVICE_TYPE_LIVE_STREAM || m_nServiceType == SERVICE_TYPE_SELF_STREAM || m_nServiceType == SERVICE_TYPE_CHANNEL) && (m_nEntityType == ENTITY_TYPE_PUBLISHER || m_nEntityType == ENTITY_TYPE_PUBLISHER_CALLER) && m_bAudioOnlyLive == true)
		return -10;

	if ((m_nServiceType == SERVICE_TYPE_LIVE_STREAM || m_nServiceType == SERVICE_TYPE_SELF_STREAM || m_nServiceType == SERVICE_TYPE_CHANNEL) && m_nEntityType == ENTITY_TYPE_VIEWER_CALLEE && m_nCallInLiveType == CALL_IN_LIVE_TYPE_AUDIO_ONLY)
		return -5;

	if ((m_nServiceType == SERVICE_TYPE_LIVE_STREAM || m_nServiceType == SERVICE_TYPE_SELF_STREAM || m_nServiceType == SERVICE_TYPE_CHANNEL) && m_nEntityType == ENTITY_TYPE_VIEWER)
		return 1;

	m_nCapturedFrameCounter++;

	if (m_bFrameReduce == true && m_nCapturedFrameCounter % m_nReduceCheckNumber == 0)
		return 1;
    
    m_VideoFpsCalculator->CalculateFPS("PushIntoBufferForEncoding, VideoFPS--> ");
    
    /*if(m_bIsCheckCall==true)
    {
        m_nDeviceCheckFrameCounter++;
        if(m_nDeviceCheckFrameCounter>75) return m_nDeviceCheckFrameCounter;
    }*/

    //LOGE("CVideoCallSession::PushIntoBufferForEncoding called");
    
	

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

	bool bIsThreadStartedFlag = false;

#ifdef	OLD_ENCODING_THREAD

	bIsThreadStartedFlag = m_pVideoEncodingThread->IsThreadStarted();

#else

	if (m_nServiceType == SERVICE_TYPE_CALL || m_nServiceType == SERVICE_TYPE_SELF_CALL)
		bIsThreadStartedFlag = m_pVideoEncodingThreadOfCall->IsThreadStarted();
	else if (m_nServiceType == SERVICE_TYPE_LIVE_STREAM || m_nServiceType == SERVICE_TYPE_SELF_STREAM)
		bIsThreadStartedFlag = m_pVideoEncodingThreadOfLive->IsThreadStarted();

#endif
    
	if (bIsThreadStartedFlag == false)
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
    
	
	CLogPrinter_Write(CLogPrinter::INFO, "CVideoCallSession::PushIntoBufferForEncoding");

	long long currentTimeStamp = m_Tools.CurrentTimestamp();

    //CLogPrinter_WriteLog(CLogPrinter::INFO, INSTENT_TEST_LOG, "CVideoCallSession::PushIntoBufferForEncoding 5");
    //Capturing fps calculation
    if(m_llClientFrameFPSTimeStamp==-1) m_llClientFrameFPSTimeStamp = currentTimeStamp;
    m_ClientFrameCounter++;
    if(currentTimeStamp - m_llClientFrameFPSTimeStamp >= 1000)
    {
        {//Block for Lock
            SessionLocker lock(*m_pVideoCallSessionMutex);
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
				SessionLocker lock(*m_pVideoCallSessionMutex);
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

	int nCaptureTimeDiff = (int)(currentTimeStamp - m_llFirstFrameCapturingTimeStamp);
    
    g_TimeTraceFromCaptureToSend[g_CapturingFrameCounter] = m_Tools.CurrentTimestamp();

    if(g_CapturingFrameCounter < 30)
        //printf("Frame %d --> Trying to Set --> %d..... Capture Time = %lld\n", g_CapturingFrameCounter, nCaptureTimeDiff, m_Tools.CurrentTimestamp());
    
    g_CapturingFrameCounter++;
    
	//CLogPrinter_WriteLog(CLogPrinter::INFO, INSTENT_TEST_LOG, "CVideoCallSession::PushIntoBufferForEncoding 504 -- size " + m_Tools.IntegertoStringConvert(in_size) + "  device_orient = "+ m_Tools.IntegertoStringConvert(device_orientation));

	//int nCroppedDataLen = this->m_pColorConverter->CropWithAspectRatio_YUVNV12_YUVNV21_RGB24(in_data, 352, 288, 1920, 1130, m_CroppedFrame, nCroppedHeight, nCroppedWidth, YUVNV12);

	LOGE_MAIN("CVideoCallSession::PushIntoBufferForEncoding  m_nVideoCallHeight = %d m_nVideoCallWidth = %d", m_nVideoCallHeight, m_nVideoCallWidth);
	//int tempH = 1080;
	//int tempW = 1920;

#ifdef __ANDROID__
	int tempHs = m_nVideoCallWidth * 4;
	int tempWs = m_nVideoCallHeight * 4;
#else
    int tempHs = m_nVideoCallHeight * 4;
    int tempWs = m_nVideoCallWidth * 4;
#endif

	unsigned char* buf = new unsigned char[tempHs * tempWs * 3 / 2];
    //long long startTime = m_Tools.CurrentTimestamp();
    m_pColorConverter->DownScaleYUVNV12_YUVNV21_OneFourth(in_data, tempHs, tempWs, buf);
    //m_pColorConverter->DownScaleYUV420_Dynamic_Version2(in_data, tempHs, tempWs, buf, tempHs/4, tempWs/4);
    //printf("DownScaleYUVNV12_YUVNV21 Time = %lld\n", m_Tools.CurrentTimestamp() - startTime);
    
    
    
    //m_pColorConverter->ConvertNV21ToI420(in_data, tempHs, tempWs);
    //long long startTime = m_Tools.CurrentTimestamp();
	//m_pColorConverter->DownScaleYUV420_OneFourth(in_data, tempHs, tempWs, buf);
    //printf("DownScaleYUVNV12_YUVNV21 Time = %lld\n", m_Tools.CurrentTimestamp() - startTime);
    
    tempHs /= 4;
    tempWs /= 4;

    
	//m_pColorConverter->ConvertI420ToNV21(buf, tempHs, tempWs);

    


	//int returnedValue = m_EncodingBuffer->Queue(in_data, in_size, m_nVideoCallHeight, m_nVideoCallWidth, nCaptureTimeDiff, device_orientation);
	int returnedValue = m_EncodingBuffer->Queue(buf, tempHs * tempWs * 3 / 2, tempHs, tempWs, nCaptureTimeDiff, device_orientation);
    
	delete[] buf;

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
    
	unsigned char uchVersion = (unsigned char)GetVersionController()->GetOwnVersion();
    
	CVideoHeader PacketHeader;

	if (nMiniPacketType == BITRATE_CONTROLL_PACKET_TYPE)
	{
		//PacketHeader.setPacketHeader(BITRATE_CONTROLL_PACKET_TYPE, uchVersion, m_miniPacketBandCounter/*SlotID*/, 0, nMiniPacketType, nByteReceivedOrNetworkType/*Byte Received*/, 0, 0, 0, 0, 0);

        PacketHeader.SetPacketHeader(BITRATE_CONTROLL_PACKET_TYPE,			//packetType
                                     uchVersion,									//VersionCode
                                     VIDEO_HEADER_LENGTH,                       //VideoHeaderLength
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
                                     0,                                          //SenderDeviceType
                                     0,                                          //NumberOfInsets
                                     nullptr,                                    //InsetHeights
                                     nullptr,                                     //InsetWidths
                                     
                                     //MoreInfo
                                     0,                                        //Device FPS
                                     0,                                        //Number of Encode Fail Per FPS
                                     0,                                        //Sigma Value
                                     0,                                        //Brightness Value
                                     0                                        //Media Engine Version
                                    );
        
        printf("TheKing--> SlotID = %d, Received Byte = %d\n", m_miniPacketBandCounter, nByteReceivedOrNetworkType);
        PacketHeader.ShowDetails("BtratePacket SendingSide: ");
	}
	else if (nMiniPacketType == NETWORK_INFO_PACKET_TYPE)
	{
		//PacketHeader.setPacketHeader(NETWORK_INFO_PACKET_TYPE, uchVersion, m_miniPacketBandCounter/*SlotID*/, 0, nMiniPacketType, nByteReceivedOrNetworkType/*Network Type*/, 0, 0, 0, 0, 0);

        PacketHeader.SetPacketHeader(NETWORK_INFO_PACKET_TYPE,				//packetType
                                     uchVersion,								//VersionCode
                                     VIDEO_HEADER_LENGTH,                   // Header Length
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
                                     0,                                      //SenderDeviceType
                                     0,                                      //NumberOfInsets
                                     nullptr,                                //InsetHeights
                                     nullptr,                                 //InsetWidths
                                     
                                     //MoreInfo
                                     0,                                        //Sigma Value
                                     0,                                        //Brightness Value
                                     0,                                        //Device FPS
                                     0,                                        //Number of Encode Fail Per FPS
                                     0                                        //Media Engine Version
                                     );
	}

	m_miniPacket[0] = (int)VIDEO_PACKET_MEDIA_TYPE;

	PacketHeader.GetHeaderInByteArray(m_miniPacket + 1);

#ifndef NO_CONNECTIVITY
	m_pCommonElementsBucket->SendFunctionPointer(m_lfriendID, MEDIA_TYPE_VIDEO, m_miniPacket, PacketHeader.GetHeaderLength() + 1, 0, std::vector< std::pair<int, int> >());
#else
	m_pCommonElementsBucket->m_pEventNotifier->fireAudioPacketEvent(200, PacketHeader.GetHeaderLength() + 1, m_miniPacket);
#endif
}

void CVideoCallSession::CreateAndSend_IDR_Frame_Info_Packet(long long llMissedFrameNumber)
{
	if (m_bDynamic_IDR_Sending_Mechanism == false)
		return;

    CVideoHeader PacketHeader;
    unsigned char uchVersion = (unsigned char)GetVersionController()->GetOwnVersion();
    
    PacketHeader.SetPacketHeader(IDR_FRAME_CONTROL_INFO_TYPE,		//packetType
                                 uchVersion,							//VersionCode
                                 VIDEO_HEADER_LENGTH,					//HeaderLength
                                 0,										//FPSByte
                                 llMissedFrameNumber,                   //FrameNumber
                                 0,										//NetworkType
                                 0,										//Device Orientation
                                 0,										//QualityLevel
                                 0,										//NumberofPacket
                                 0,                                     //PacketNumber
                                 0,                                     //TimeStamp
                                 0,										//PacketStartingIndex
                                 0,										//PacketDataLength
								 0,                                      //SenderDeviceType
								 0,                                      //NumberOfInsets
								 nullptr,                                //InsetHeights
								 nullptr,                                 //InsetWidths
                                 
                                 //MoreInfo
                                 0,                                        //Sigma Value
                                 0,                                        //Brightness Value
                                 0,                                        //Device FPS
                                 0,                                        //Number of Encode Fail Per FPS
                                 0                                        //Media Engine Version
                                 );
    
    m_miniPacket[0] = (int)VIDEO_PACKET_MEDIA_TYPE;
    
    PacketHeader.GetHeaderInByteArray(m_miniPacket + 1);
    
#ifndef NO_CONNECTIVITY
    printf("TheKing--> Trying.... CreateAndSend_IDR_Frame_Info_Packet\n");
	m_pCommonElementsBucket->SendFunctionPointer(m_lfriendID, MEDIA_TYPE_VIDEO, m_miniPacket, PacketHeader.GetHeaderLength() + 1, 0, std::vector< std::pair<int, int> >());
#else
    m_pCommonElementsBucket->m_pEventNotifier->fireAudioPacketEvent(200, PacketHeader.GetHeaderLength() + 1, m_miniPacket);
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
        
#ifdef OLD_DECODING_THREAD

        m_pVideoDecodingThread->Reset();
#else
		m_pVideoDecodingThreadOfCall->Reset();
#endif

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

void CVideoCallSession::FirstFrameCapturingTimeStamp()
{
	m_llFirstFrameCapturingTimeStamp = -1;
}

bool CVideoCallSession::GetResolutionNegotiationStatus()
{
    return m_bResolutionNegotiationDone;
}

void CVideoCallSession::StopDeviceAbilityChecking()
{
	long long llReinitializationStartTime = m_Tools.CurrentTimestamp();

//	CLogPrinter_WriteLog(CLogPrinter::INFO, INSTENT_TEST_LOG, "Video call session destructor 1");

#ifdef	OLD_ENCODING_THREAD

	m_pVideoEncodingThread->StopEncodingThread();

#else

	m_pVideoEncodingThreadOfCall->StopEncodingThread();

#endif

//	CLogPrinter_WriteLog(CLogPrinter::INFO, INSTENT_TEST_LOG, "Video call session destructor 2");
	m_pVideoDepacketizationThread->StopDepacketizationThread();
//	CLogPrinter_WriteLog(CLogPrinter::INFO, INSTENT_TEST_LOG, "Video call session destructor 4");

#ifdef OLD_DECODING_THREAD

	m_pVideoDecodingThread->InstructionToStop();

#else

	m_pVideoDecodingThreadOfCall->InstructionToStop();

#endif

//	CLogPrinter_WriteLog(CLogPrinter::INFO, INSTENT_TEST_LOG, "Video call session destructor 5");

#ifdef OLD_RENDERING_THREAD

	m_pVideoRenderingThread->StopRenderingThread();

#else

	m_pRenderingThreadOfCall->StopRenderingThread();

#endif

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
		m_llSlotResetRightRange = HIGH_QUALITY_FPS;
	}
	else if (m_nCurrentVideoCallQualityLevel == SUPPORTED_RESOLUTION_FPS_352_25)
	{
        m_nVideoCallHeight = m_pController->m_Quality[0].iHeight;
        m_nVideoCallWidth = m_pController->m_Quality[0].iWidth;
		m_nCallFPS = HIGH_QUALITY_FPS;
		m_llSlotResetRightRange = HIGH_QUALITY_FPS;
	}
	else if (m_nCurrentVideoCallQualityLevel == SUPPORTED_RESOLUTION_FPS_352_15)
	{
        m_nVideoCallHeight = m_pController->m_Quality[0].iHeight;
        m_nVideoCallWidth = m_pController->m_Quality[0].iWidth;
        
		m_nCallFPS = LOW_QUALITY_FPS;
		m_llSlotResetRightRange = LOW_QUALITY_FPS;
	}
	else if (m_nCurrentVideoCallQualityLevel == RESOLUTION_FPS_SUPPORT_NOT_TESTED)
	{
        m_nVideoCallHeight = m_pController->m_Quality[0].iHeight;
        m_nVideoCallWidth = m_pController->m_Quality[0].iWidth;
		m_nCallFPS = LOW_QUALITY_FPS;
		m_llSlotResetRightRange = LOW_QUALITY_FPS;
	}

	m_BitRateController->SetCallFPS(m_nCallFPS);

#ifdef	OLD_ENCODING_THREAD

	m_pVideoEncodingThread->SetCallFPS(m_nCallFPS);
	m_pVideoEncodingThread->SetFrameNumber(m_nCallFPS);

#else

	m_pVideoEncodingThreadOfCall->SetCallFPS(m_nCallFPS);
    m_pVideoEncodingThreadOfCall->SetFrameNumber(m_nCallFPS);

#endif

#ifdef OLD_DECODING_THREAD

	m_pVideoDecodingThread->SetCallFPS(m_nCallFPS);

#else

	m_pVideoDecodingThreadOfCall->SetCallFPS(m_nCallFPS);

#endif
    
    m_pFPSController->Reset(m_nCallFPS);

	this->m_pColorConverter->SetHeightWidth(m_nVideoCallHeight, m_nVideoCallWidth);

	if (m_nServiceType == SERVICE_TYPE_LIVE_STREAM || m_nServiceType == SERVICE_TYPE_SELF_STREAM || m_nServiceType == SERVICE_TYPE_CHANNEL)
		this->m_pVideoEncoder->SetHeightWidth(m_nVideoCallHeight, m_nVideoCallWidth, m_nCallFPS, IFRAME_INTERVAL, m_bIsCheckCall, m_nServiceType, CAMARA_VIDEO_DATA);
    else
    {
        if(m_bDynamic_IDR_Sending_Mechanism == true)
        {
			this->m_pVideoEncoder->SetHeightWidth(m_nVideoCallHeight, m_nVideoCallWidth, m_nCallFPS, m_nCallFPS * 2, m_bIsCheckCall, m_nServiceType, CAMARA_VIDEO_DATA);
        }
        else
        {
			this->m_pVideoEncoder->SetHeightWidth(m_nVideoCallHeight, m_nVideoCallWidth, m_nCallFPS, m_nCallFPS / 2 + 1, m_bIsCheckCall, m_nServiceType, CAMARA_VIDEO_DATA);
        }
    }

#ifdef	OLD_ENCODING_THREAD

	m_pVideoEncodingThread->SetNotifierFlag(true);

#else

	m_pVideoEncodingThreadOfCall->SetNotifierFlag(true);

#endif

}

BitRateController* CVideoCallSession::GetBitRateController(){
	return m_BitRateController;
}

void CVideoCallSession::SetCallInLiveType(int nCallInLiveType)
{
	m_nCallInLiveType = nCallInLiveType;
}

int CVideoCallSession::SetEncoderHeightWidth(const long long& lFriendID, int height, int width, int nDataType)
{
	if (m_nVideoCallHeight != height || m_nVideoCallWidth != width || m_nOwnDeviceType == DEVICE_TYPE_DESKTOP)
	{
		m_nVideoCallHeight = height;
		m_nVideoCallWidth = width;


		this->m_pColorConverter->SetHeightWidth(height, width);

		if (m_nServiceType == SERVICE_TYPE_LIVE_STREAM || m_nServiceType == SERVICE_TYPE_SELF_STREAM || m_nServiceType == SERVICE_TYPE_CHANNEL)
			this->m_pVideoEncoder->SetHeightWidth(height, width, m_nCallFPS, IFRAME_INTERVAL, m_bIsCheckCall, m_nServiceType, nDataType);
		else
        {
            if(m_bDynamic_IDR_Sending_Mechanism == true)
            {
				this->m_pVideoEncoder->SetHeightWidth(height, width, m_nCallFPS, m_nCallFPS * 2, m_bIsCheckCall, m_nServiceType, nDataType);
            }
            else
            {
				this->m_pVideoEncoder->SetHeightWidth(height, width, m_nCallFPS, m_nCallFPS / 2 + 1, m_bIsCheckCall, m_nServiceType, nDataType);
            }
        }

		return 1;
	}
	else
	{
		return -1;
	}
}

void CVideoCallSession::SetBeautification(bool bIsEnable)
{
    
#ifdef	OLD_ENCODING_THREAD
    
    m_pVideoEncodingThread->SetBeautification(bIsEnable);
    
#else
    
    if (m_nServiceType == SERVICE_TYPE_CALL || m_nServiceType == SERVICE_TYPE_SELF_CALL)
        m_pVideoEncodingThreadOfCall->SetBeautification(bIsEnable);
    else if (m_nServiceType == SERVICE_TYPE_LIVE_STREAM || m_nServiceType == SERVICE_TYPE_SELF_STREAM)
        m_pVideoEncodingThreadOfLive->SetBeautification(bIsEnable);
    
#endif
    
}

void CVideoCallSession::SetVideoQualityForLive(int quality)
{
	if (quality == VIDEO_QUALITY_HIGH && m_bLiveVideoQuality != VIDEO_QUALITY_HIGH)
	{
		m_bFrameReduce = false;

		m_pVideoEncoder->SetBitrate(BITRATE_BEGIN_FOR_STREAM);
		m_pVideoEncoder->SetMaxBitrate((int)(BITRATE_BEGIN_FOR_STREAM/1.25));

		m_bLiveVideoQuality = VIDEO_QUALITY_HIGH;
	}
	else if (quality == VIDEO_QUALITY_MEDIUM && m_bLiveVideoQuality != VIDEO_QUALITY_MEDIUM)
	{
		int nDeviceFPS = m_VideoFpsCalculator->GetDeviceFPS();

		if (nDeviceFPS > 20)
		{
			m_bFrameReduce = true;

			m_nReduceCheckNumber = 3;

			nDeviceFPS = nDeviceFPS * 2;
			nDeviceFPS = nDeviceFPS / 3;

			m_VideoFpsCalculator->SetDeviceFPS(nDeviceFPS);
		}

		m_pVideoEncoder->SetBitrate(BITRATE_FOR_MEDIUM_STREAM);
		m_pVideoEncoder->SetMaxBitrate(BITRATE_FOR_MEDIUM_STREAM);

		m_bLiveVideoQuality = VIDEO_QUALITY_MEDIUM;
	}
	else if (quality == VIDEO_QUALITY_LOW && m_bLiveVideoQuality != VIDEO_QUALITY_LOW)
	{
		int nDeviceFPS = m_VideoFpsCalculator->GetDeviceFPS();

		if (nDeviceFPS > 20)
		{
			m_bFrameReduce = true;

			m_nReduceCheckNumber = 2;

			nDeviceFPS = nDeviceFPS / 2;

			m_VideoFpsCalculator->SetDeviceFPS(nDeviceFPS);
		}
		else if (nDeviceFPS > 15)
		{
			m_bFrameReduce = true;

			m_nReduceCheckNumber = 3;

			nDeviceFPS = nDeviceFPS * 2;
			nDeviceFPS = nDeviceFPS / 3;

			m_VideoFpsCalculator->SetDeviceFPS(nDeviceFPS);
		}

		m_pVideoEncoder->SetBitrate(BITRATE_FOR_LOW_STREAM);
		m_pVideoEncoder->SetMaxBitrate(BITRATE_FOR_LOW_STREAM);

		m_bLiveVideoQuality = VIDEO_QUALITY_LOW;
	}
}
    
int CVideoCallSession::SetVideoEffect(int nEffectStatus)
{
	if (nEffectStatus == 1)
		m_bVideoEffectEnabled = true;
	else if (nEffectStatus == 0)
		m_bVideoEffectEnabled = false;

#ifdef	OLD_ENCODING_THREAD

	m_pVideoEncodingThread->SetVideoEffect(nEffectStatus);

#else

	if (m_nServiceType == SERVICE_TYPE_CALL || m_nServiceType == SERVICE_TYPE_SELF_CALL)
		m_pVideoEncodingThreadOfCall->SetVideoEffect(nEffectStatus);
	else if (m_nServiceType == SERVICE_TYPE_LIVE_STREAM || m_nServiceType == SERVICE_TYPE_SELF_STREAM)
		m_pVideoEncodingThreadOfLive->SetVideoEffect(nEffectStatus);

#endif

	return 1;
}

int CVideoCallSession::TestVideoEffect( int *param, int size)
{
	if (m_pVideoEncodingThreadOfLive != NULL)
	{
		this->m_pVideoEncodingThreadOfLive->TestVideoEffect(param, size);
	}
	else if (m_pVideoEncodingThreadOfCall != NULL)
	{
		this->m_pVideoEncodingThreadOfCall->TestVideoEffect(param, size);
	}
	else if (m_pVideoEncodingThread != NULL)
	{
		this->m_pVideoEncodingThread->TestVideoEffect(param, size);
	}

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

int CVideoCallSession::SetDeviceHeightWidth(const long long& lFriendID, int height, int width)
{
	m_nDeviceHeight = height;
	m_nDeviceWidth = width;

	this->m_pColorConverter->SetDeviceHeightWidth(height, width);

	return 1;
}

void CVideoCallSession::InterruptOccured()
{

#ifdef OLD_SENDING_THREAD

	m_pSendingThread->InterruptOccured();

#else

	m_pSendingThreadOfLive->InterruptOccured();

#endif

}

void CVideoCallSession::InterruptOver()
{

#ifdef OLD_SENDING_THREAD

	m_pSendingThread->InterruptOver();

#else

	m_pSendingThreadOfLive->InterruptOver();

#endif

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
		m_pVideoEncoder->CreateVideoEncoder(iHeight, iWidth, m_nCallFPS, IFRAME_INTERVAL, m_bIsCheckCall, m_nServiceType);
    else
    {
        if(m_bDynamic_IDR_Sending_Mechanism == true)
        {
            m_pVideoEncoder->CreateVideoEncoder(iHeight, iWidth, m_nCallFPS, m_nCallFPS * 2, m_bIsCheckCall, m_nServiceType);
        }
        else
        {
            m_pVideoEncoder->CreateVideoEncoder(iHeight, iWidth, m_nCallFPS, m_nCallFPS / 2 + 1, m_bIsCheckCall, m_nServiceType);
        }
    }



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
    
	m_pVideoDepacketizationThread = new CVideoDepacketizationThread(m_lfriendID, m_pVideoPacketQueue, m_pRetransVideoPacketQueue, m_pMiniPacketQueue, m_BitRateController, m_pIdrFrameIntervalController, m_pEncodedFrameDepacketizer, m_pCommonElementsBucket, &m_miniPacketBandCounter, m_pVersionController, this);
    

    
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

void CVideoCallSession::StartCallInLive(int nCallInLiveType, int nCalleeID)
{
	if (m_iRole != 0)
		return;
	else
	{
		m_nCallInLiveType = nCallInLiveType;

		if (m_nEntityType == ENTITY_TYPE_PUBLISHER)
		{
			SetFirstVideoPacketTime(-1);
			SetShiftedTime(-1);

			m_pColorConverter->ClearSmallScreen();

#ifdef OLD_DECODING_THREAD

			m_pVideoDecodingThread->ResetForViewerCallerCallStartEnd();	
#else
			m_pVideoDecodingThreadOfLive->ResetForViewerCallerCallStartEnd();
#endif

			LOGSS("##SS## m_bAudioOnlyLive %d nCallInLiveType %d", m_bAudioOnlyLive, m_nCallInLiveType);

#ifdef OLD_SENDING_THREAD

			if (m_bAudioOnlyLive == true && nCallInLiveType == CALL_IN_LIVE_TYPE_AUDIO_VIDEO)
				m_pSendingThread->ResetForPublisherCallerCallStartAudioOnly();
#else
			if (m_bAudioOnlyLive == true && nCallInLiveType == CALL_IN_LIVE_TYPE_AUDIO_VIDEO)
				m_pSendingThreadOfLive->ResetForPublisherCallerCallStartAudioOnly();
#endif

			m_nPublisherInsetNumber = 1;

			m_nEntityType = ENTITY_TYPE_PUBLISHER_CALLER;
		}
		else if (m_nEntityType == ENTITY_TYPE_VIEWER)
		{

#ifdef OLD_DECODING_THREAD

			m_pVideoDecodingThread->ResetForViewerCallerCallStartEnd();
#else
			m_pVideoDecodingThreadOfLive->ResetForViewerCallerCallStartEnd();
#endif

#ifdef	OLD_ENCODING_THREAD

			m_pVideoEncodingThread->ResetForViewerCallerCallEnd();
#else
			m_pVideoEncodingThreadOfLive->ResetForViewerCallerCallEnd();
#endif

#ifdef OLD_SENDING_THREAD

			m_pSendingThread->ResetForViewerCallerCallEnd();
#else
			m_pSendingThreadOfLive->ResetForViewerCallerCallEnd();
#endif

			m_pVideoEncoder->SetBitrate(BITRATE_FOR_INSET_STREAM);
			m_pVideoEncoder->SetMaxBitrate(BITRATE_FOR_INSET_STREAM);

			m_llFirstFrameCapturingTimeStamp = -1;

			m_nEntityType = ENTITY_TYPE_VIEWER_CALLEE;
		}
		/*else if (m_nEntityType == ENTITY_TYPE_PUBLISHER_CALLER)
		{
			if (m_nPublisherInsetNumber == 1)
			{
				m_nPublisherInsetNumber = 2;

				m_pVideoDecoderForSecondInset = new CVideoDecoder(m_pCommonElementsBucket);
				m_pVideoDecodingThreadForSecondInset = new CVideoDecodingThreadOfLive(m_pEncodedFrameDepacketizer, nCalleeID, m_pCommonElementsBucket, m_RenderingBuffer, m_pLiveVideoDecodingQueue, m_pVideoDecoderForSecondInset, m_pColorConverter, this, m_bIsCheckCall, m_nCallFPS);
			}
			else if (m_nPublisherInsetNumber == 2)
			{
				m_nPublisherInsetNumber = 3;

				m_pVideoDecoderForThirdInset = new CVideoDecoder(m_pCommonElementsBucket);
				m_pVideoDecodingThreadForSecondInset = new CVideoDecodingThreadOfLive(m_pEncodedFrameDepacketizer, nCalleeID, m_pCommonElementsBucket, m_RenderingBuffer, m_pLiveVideoDecodingQueue, m_pVideoDecoderForThirdInset, m_pColorConverter, this, m_bIsCheckCall, m_nCallFPS);
			}
		}*/

		m_iRole = 1;
	}
}

void CVideoCallSession::EndCallInLive(int nCalleeID)
{
	if (m_iRole != 1)
		return;
	else
	{
		/*if (m_nEntityType == ENTITY_TYPE_PUBLISHER_CALLER && m_nPublisherInsetNumber == 3)
		{
			m_pVideoDecodingThreadForThirdInset->StartDecodingThread();

			if (NULL != m_pVideoDecodingThreadForThirdInset)
			{
				delete m_pVideoDecodingThreadForThirdInset;
				m_pVideoDecodingThreadForThirdInset = NULL;
			}

			if (NULL != m_pVideoDecoderForThirdInset)
			{
				delete m_pVideoDecoderForThirdInset;

				m_pVideoDecoderForThirdInset = NULL;
			}
		}
		else if (m_nEntityType == ENTITY_TYPE_PUBLISHER_CALLER && m_nPublisherInsetNumber == 2)
		{
			m_pVideoDecodingThreadForSecondInset->StartDecodingThread();

			if (NULL != m_pVideoDecodingThreadForSecondInset)
			{
				delete m_pVideoDecodingThreadForSecondInset;
				m_pVideoDecodingThreadForSecondInset = NULL;
			}

			if (NULL != m_pVideoDecoderForSecondInset)
			{
				delete m_pVideoDecoderForSecondInset;

				m_pVideoDecoderForSecondInset = NULL;
			}
		}
		else */if (m_nEntityType == ENTITY_TYPE_PUBLISHER_CALLER)
		{
			//m_pVideoDepacketizationThread->ResetForPublisherCallerCallEnd();

			SetFirstVideoPacketTime(-1);
			SetShiftedTime(-1);

			m_pColorConverter->ClearSmallScreen();

#ifdef OLD_DECODING_THREAD

			m_pVideoDecodingThread->ResetForViewerCallerCallStartEnd();
#else
			m_pVideoDecodingThreadOfLive->ResetForViewerCallerCallStartEnd();
#endif

#ifdef	OLD_ENCODING_THREAD

			if (m_bAudioOnlyLive)
				m_pVideoEncodingThread->ResetForPublisherCallerInAudioOnly();

#else
			if (m_bAudioOnlyLive)
				m_pVideoEncodingThreadOfLive->ResetForPublisherCallerInAudioOnly();
#endif

			//m_pCommonElementsBucket->m_pEventNotifier->fireVideoNotificationEvent(m_lfriendID, m_pCommonElementsBucket->m_pEventNotifier->LIVE_CALL_INSET_OFF);

			m_nEntityType = ENTITY_TYPE_PUBLISHER;
		}
		else if (m_nEntityType == ENTITY_TYPE_VIEWER_CALLEE)
		{

#ifdef OLD_DECODING_THREAD

			m_pVideoDecodingThread->ResetForViewerCallerCallStartEnd();
#else
			m_pVideoDecodingThreadOfLive->ResetForViewerCallerCallStartEnd();
#endif

#ifdef	OLD_ENCODING_THREAD

			m_pVideoEncodingThread->ResetForViewerCallerCallEnd();
#else
			m_pVideoEncodingThreadOfLive->ResetForViewerCallerCallEnd();
#endif

#ifdef OLD_SENDING_THREAD

			m_pSendingThread->ResetForViewerCallerCallEnd();
#else
			m_pSendingThreadOfLive->ResetForViewerCallerCallEnd();
#endif

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

bool CVideoCallSession::GetAudioOnlyLiveStatus()
{
	return m_bAudioOnlyLive;
}

int CVideoCallSession::GetCallInLiveType()
{
	return m_nCallInLiveType;
}

bool CVideoCallSession::isDynamicIDR_Mechanism_Enable()
{
    return m_bDynamic_IDR_Sending_Mechanism;
}
    
CAverageCalculator* CVideoCallSession::getFpsCalculator()
{
    return m_VideoFpsCalculator;
}

} //namespace MediaSDK


