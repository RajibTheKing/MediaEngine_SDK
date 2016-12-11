#include "Controller.h"
#include "LogPrinter.h"
#include "Tools.h"
#include "VideoCallSession.h"
#include "VideoEncoder.h"
#include "VideoDecoder.h"

//extern int g_StopVideoSending;

CController::CController():

m_nDeviceStrongness(STATUS_UNCHECKED),
m_nMemoryEnoughness(STATUS_UNCHECKED),
m_nEDVideoSupportablity(STATUS_UNCHECKED),
m_nHighFPSVideoSupportablity(STATUS_UNCHECKED),
m_nDeviceSupportedCallFPS(LOW_FRAME_RATE),
m_pAudioEncodeDecodeSession(NULL),
m_pVideoMuxingAndEncodeSession(NULL),
m_pDeviceCapabilityCheckBuffer(NULL),
m_pDeviceCapabilityCheckThread(NULL),
m_nSupportedResolutionFPSLevel(RESOLUTION_FPS_SUPPORT_NOT_TESTED),
m_bDeviceCapabilityRunning(false),
m_bLiveCallRunning(false),
m_EventNotifier(this)
{
	CLogPrinter::Start(CLogPrinter::DEBUGS, "");
	CLogPrinter_Write(CLogPrinter::DEBUGS, "CController::CController() AudioVideoEngine Initializing");
	
	m_pCommonElementsBucket = new CCommonElementsBucket();
    
    m_pVideoSendMutex.reset(new CLockHandler);
    m_pVideoReceiveMutex.reset(new CLockHandler);
    m_pAudioSendMutex.reset(new CLockHandler);
    m_pAudioReceiveMutex.reset(new CLockHandler);

	m_pDeviceCapabilityCheckBuffer = new CDeviceCapabilityCheckBuffer();
	m_pDeviceCapabilityCheckThread = new CDeviceCapabilityCheckThread(this, m_pDeviceCapabilityCheckBuffer, m_pCommonElementsBucket);

	CLogPrinter_Write(CLogPrinter::DEBUGS, "CController::CController() AudioVideoEngine Initialization completed");
}

CController::CController(const char* sLoggerPath, int iLoggerPrintLevel) :

logFilePath(sLoggerPath),
iLoggerPrintLevel(iLoggerPrintLevel),
m_nDeviceStrongness(STATUS_UNCHECKED),
m_nMemoryEnoughness(STATUS_UNCHECKED),
m_nEDVideoSupportablity(STATUS_UNCHECKED),
m_nHighFPSVideoSupportablity(STATUS_UNCHECKED),
m_nDeviceSupportedCallFPS(LOW_FRAME_RATE),
m_pAudioEncodeDecodeSession(NULL),
m_pVideoMuxingAndEncodeSession(NULL),
m_pDeviceCapabilityCheckBuffer(NULL),
m_pDeviceCapabilityCheckThread(NULL),
m_nSupportedResolutionFPSLevel(RESOLUTION_FPS_SUPPORT_NOT_TESTED),
m_EventNotifier(this)
{
	CLogPrinter::Start((CLogPrinter::Priority)iLoggerPrintLevel, sLoggerPath);
	CLogPrinter_Write(CLogPrinter::DEBUGS, "CController::CController() AudioVideoEngine Initializing");

	m_pCommonElementsBucket = new CCommonElementsBucket();
    
    m_pVideoSendMutex.reset(new CLockHandler);
    m_pVideoReceiveMutex.reset(new CLockHandler);
    m_pAudioSendMutex.reset(new CLockHandler);
    m_pAudioReceiveMutex.reset(new CLockHandler);

	m_pDeviceCapabilityCheckBuffer = new CDeviceCapabilityCheckBuffer();
	m_pDeviceCapabilityCheckThread = new CDeviceCapabilityCheckThread(this, m_pDeviceCapabilityCheckBuffer, m_pCommonElementsBucket);

	CLogPrinter_Write(CLogPrinter::DEBUGS, "CController::CController() AudioVideoEngine Initialization completed");
}

void CController::SetLoggerPath(std::string sLoggerPath)
{
	CLogPrinter::SetLoggerPath(sLoggerPath);
}

bool CController::SetLoggingState(bool loggingState, int logLevel)
{
    bool bReturnedValue;
    
    bReturnedValue = CLogPrinter::SetLoggingState(loggingState, logLevel);
    
    return bReturnedValue;
}

CController::~CController()
{
	CLogPrinter_Write(CLogPrinter::WARNING, "CController::~CController()");

	if (NULL != m_pDeviceCapabilityCheckThread)
	{

		delete m_pDeviceCapabilityCheckThread;
		m_pDeviceCapabilityCheckThread = NULL;
	}

	if (NULL != m_pDeviceCapabilityCheckBuffer)
	{
		delete m_pDeviceCapabilityCheckBuffer;
		m_pDeviceCapabilityCheckBuffer = NULL;
	}

	if (NULL != m_pCommonElementsBucket)
	{
		delete m_pCommonElementsBucket;
		m_pCommonElementsBucket = NULL;
	}

	if (NULL != m_pAudioEncodeDecodeSession)
	{
		delete m_pAudioEncodeDecodeSession;

		m_pAudioEncodeDecodeSession = NULL;
	}

	if (NULL != m_pVideoMuxingAndEncodeSession)
	{
		delete m_pVideoMuxingAndEncodeSession;

		m_pVideoMuxingAndEncodeSession = NULL;
	}
    
    SHARED_PTR_DELETE(m_pVideoSendMutex);
    SHARED_PTR_DELETE(m_pVideoReceiveMutex);
    SHARED_PTR_DELETE(m_pAudioSendMutex);
    SHARED_PTR_DELETE(m_pAudioReceiveMutex);

	CLogPrinter_Write(CLogPrinter::WARNING, "CController::~CController() removed everything");
}

bool CController::SetUserName(const LongLong& lUserName)
{
	m_pCommonElementsBucket->SetUserName(lUserName);

	CLogPrinter_Write(CLogPrinter::DEBUGS, "CController::SetUserName() user name: " + m_Tools.LongLongtoStringConvert(lUserName));

	return true;
}

bool CController::StartAudioCall(const LongLong& lFriendID, int nServiceType)
{
	CAudioCallSession* pAudioSession;
    
    //Locker lock1(*m_pAudioSendMutex);
    //Locker lock2(*m_pAudioReceiveMutex);

	bool bExist = m_pCommonElementsBucket->m_pAudioCallSessionList->IsAudioSessionExist(lFriendID, pAudioSession);

	CLogPrinter_Write(CLogPrinter::INFO, "CController::StartAudioCall");

	if (!bExist)
	{
		CLogPrinter_Write(CLogPrinter::INFO, "CController::StartAudioCall Session empty");

		pAudioSession = new CAudioCallSession(lFriendID, m_pCommonElementsBucket, nServiceType);

		pAudioSession->InitializeAudioCallSession(lFriendID);

		m_pCommonElementsBucket->m_pAudioCallSessionList->AddToAudioSessionList(lFriendID, pAudioSession);

		CLogPrinter_Write(CLogPrinter::INFO, "CController::StartAudioCall Session started");

		return true;
	}
	else
	{
		return false;
	}
}

bool CController::SetVolume(const LongLong& lFriendID, int iVolume)
{
	CAudioCallSession* pAudioSession;

	bool bExist = m_pCommonElementsBucket->m_pAudioCallSessionList->IsAudioSessionExist(lFriendID, pAudioSession);
	if (bExist)
	{
		pAudioSession->SetVolume(iVolume);
		return true;
	}
	else
	{
		return false;
	}
}

bool CController::SetLoudSpeaker(const LongLong& lFriendID, bool bOn)
{
	CAudioCallSession* pAudioSession;

	bool bExist = m_pCommonElementsBucket->m_pAudioCallSessionList->IsAudioSessionExist(lFriendID, pAudioSession);
	if (bExist)
	{
		pAudioSession->SetLoudSpeaker(bOn);
		return true;
	}
	else
	{
		return false;
	}
}

bool CController::StartTestAudioCall(const LongLong& lFriendID)
{
	CLogPrinter_WriteLog(CLogPrinter::INFO, CHECK_CAPABILITY_LOG, "CController::StartTestAudioCall() called");

	CAudioCallSession* pAudioSession;

	bool bExist = m_pCommonElementsBucket->m_pAudioCallSessionList->IsAudioSessionExist(lFriendID, pAudioSession);

	if (!bExist)
	{
		CLogPrinter_WriteLog(CLogPrinter::INFO, CHECK_CAPABILITY_LOG, "CController::StartTestAudioCall() session creating");

		pAudioSession = new CAudioCallSession(lFriendID, m_pCommonElementsBucket, SERVICE_TYPE_CALL, DEVICE_ABILITY_CHECK_MOOD);

		pAudioSession->InitializeAudioCallSession(lFriendID);

		m_pCommonElementsBucket->m_pAudioCallSessionList->AddToAudioSessionList(lFriendID, pAudioSession);

		CLogPrinter_WriteLog(CLogPrinter::INFO, CHECK_CAPABILITY_LOG, "CController::StartTestAudioCall() session created");

		return true;
	}
	else
	{
		CLogPrinter_WriteLog(CLogPrinter::INFO, CHECK_CAPABILITY_LOG, "CController::StartTestAudioCall() session already exists");

		return false;
	}
}

CVideoCallSession* CController::StartTestVideoCall(const LongLong& lFriendID, int iVideoHeight, int iVideoWidth, int iNetworkType)
{

	CLogPrinter_WriteLog(CLogPrinter::INFO, CHECK_CAPABILITY_LOG, "CController::StartTestVideoCall() called");
    
	CVideoCallSession* pVideoSession;

	bool bExist = m_pCommonElementsBucket->m_pVideoCallSessionList->IsVideoSessionExist(lFriendID, pVideoSession);

	if (!bExist)
	{
		CLogPrinter_WriteLog(CLogPrinter::INFO, CHECK_CAPABILITY_LOG, "CController::StartTestVideoCall() session creating");

		pVideoSession = new CVideoCallSession(this, lFriendID, m_pCommonElementsBucket, HIGH_FRAME_RATE, &m_nDeviceSupportedCallFPS, DEVICE_ABILITY_CHECK_MOOD, m_pDeviceCapabilityCheckBuffer, m_nSupportedResolutionFPSLevel, 11);

		pVideoSession->InitializeVideoSession(lFriendID, iVideoHeight, iVideoWidth, 11, iNetworkType);

		m_pCommonElementsBucket->m_pVideoCallSessionList->AddToVideoSessionList(lFriendID, pVideoSession);

		CLogPrinter_WriteLog(CLogPrinter::INFO, CHECK_CAPABILITY_LOG, "CController::StartTestVideoCall() session created");

		return pVideoSession;
	}
	else
	{
		//pVideoSession->ReInitializeVideoLibrary(iVideoHeight, iVideoWidth);

		CLogPrinter_WriteLog(CLogPrinter::INFO, CHECK_CAPABILITY_LOG, "CController::StartTestVideoCall() session already exists");

		return NULL;
	}
}

bool CController::StartVideoCall(const LongLong& lFriendID, int iVideoHeight, int iVideoWidth, int nServiceType, int iNetworkType)
{
    if(iVideoHeight * iVideoWidth > 352 * 288)
    {
        m_Quality[1].iHeight = iVideoHeight;
        m_Quality[1].iWidth = iVideoWidth;
    }
    else
    {
        m_Quality[0].iHeight = iVideoHeight;
        m_Quality[0].iWidth = iVideoWidth;
    }
    
    if(m_bDeviceCapabilityRunning == true) return false;
    
    
	CVideoCallSession* pVideoSession;

	CLogPrinter_Write(CLogPrinter::INFO, "CController::StartVideoCall called");
    
    //Locker lock1(*m_pVideoSendMutex);
    //Locker lock2(*m_pVideoReceiveMutex);

	bool bExist = m_pCommonElementsBucket->m_pVideoCallSessionList->IsVideoSessionExist(lFriendID, pVideoSession);

	if (!bExist)
	{
        m_bLiveCallRunning = true;
        
		CLogPrinter_Write(CLogPrinter::DEBUGS, "CController::StartVideoCall Video Session starting");

		pVideoSession = new CVideoCallSession(this, lFriendID, m_pCommonElementsBucket, m_nDeviceSupportedCallFPS, &m_nDeviceSupportedCallFPS, LIVE_CALL_MOOD, NULL, m_nSupportedResolutionFPSLevel,nServiceType);

		pVideoSession->InitializeVideoSession(lFriendID, iVideoHeight, iVideoWidth,nServiceType,iNetworkType);

		m_pCommonElementsBucket->m_pVideoCallSessionList->AddToVideoSessionList(lFriendID, pVideoSession);

		CLogPrinter_Write(CLogPrinter::DEBUGS, "CController::StartVideoCall Video Session started");
        
        

		return true;
	}
	else
	{
       // pVideoSession->ReInitializeVideoLibrary(iVideoHeight, iVideoWidth);
		return false;
	}	
}

int CController::EncodeVideoFrame(const LongLong& lFriendID, unsigned char *in_data, unsigned int in_size)
{
	CVideoCallSession* pVideoSession;

	CLogPrinter_Write(CLogPrinter::DEBUGS, "CController::EncodeAndTransfer called");
    
    Locker lock(*m_pVideoSendMutex);

	bool bExist = m_pCommonElementsBucket->m_pVideoCallSessionList->IsVideoSessionExist(lFriendID, pVideoSession);

	if (bExist)
	{
		CLogPrinter_Write(CLogPrinter::INFO, "CController::EncodeAndTransfer getting encoder");

		CVideoEncoder *pVideoEncoder = pVideoSession->GetVideoEncoder();

		CLogPrinter_Write(CLogPrinter::INFO, "CController::EncodeAndTransfer got encoder");

		if (pVideoEncoder)
			return pVideoSession->PushIntoBufferForEncoding(in_data, in_size, 0);
		else 
			return -1;
	}
	else
	{
		return -1;
	}
}

int CController::PushPacketForDecodingVector(const LongLong& lFriendID, int offset, unsigned char *in_data, unsigned int in_size, int numberOfFrames, int *frameSizes, std::vector< std::pair<int, int> > vMissingFrames)
{
	CVideoCallSession* pVideoSession = NULL;

	CLogPrinter_Write(CLogPrinter::DEBUGS, "CController::PushPacketForDecoding called");

	//	LOGE("CController::PushPacketForDecoding");

	Locker lock(*m_pVideoReceiveMutex);

	bool bExist = m_pCommonElementsBucket->m_pVideoCallSessionList->IsVideoSessionExist(lFriendID, pVideoSession);

	//	LOGE("CController::PushPacketForDecoding video session exists");

	if (bExist)
	{
		//		LOGE("CController::ParseFrameIntoPackets getting PushPacketForDecoding");
		//		CVideoDecoder *pCVideoDecoder = pVideoSession->GetVideoDecoder();
		//		LOGE("CController::ParseFrameIntoPackets got PushPacketForDecoding1");
		//		CEncodedFrameDepacketizer *p_CEncodedFrameDepacketizer = pCVideoDecoder->GetEncodedFrameDepacketizer();
		//		CEncodedFrameDepacketizer *p_CEncodedFrameDepacketizer = pVideoSession->GetEncodedFrameDepacketizer();
		//		LOGE("CController::ParseFrameIntoPackets got PushPacketForDecoding2");
		//		CLogPrinter_WriteSpecific(CLogPrinter::DEBUGS, " CNTRL SIGBYTE: "+ m_Tools.IntegertoStringConvert((int)in_data[1+SIGNAL_BYTE_INDEX]));
		if (pVideoSession)
			return pVideoSession->PushPacketForMergingVector(offset, in_data, in_size, false, numberOfFrames, frameSizes, vMissingFrames);
		else
			return -1;
	}
	else
	{
		return -1;
	}
}

int CController::PushPacketForDecoding(const LongLong& lFriendID,unsigned char *in_data, unsigned int in_size, int numberOfFrames, int *frameSizes, int numberOfMissingFrames, int *missingFrames)
{
	CVideoCallSession* pVideoSession = NULL;

	CLogPrinter_Write(CLogPrinter::DEBUGS, "CController::PushPacketForDecoding called");

//	LOGE("CController::PushPacketForDecoding");
    
    Locker lock(*m_pVideoReceiveMutex);

	bool bExist = m_pCommonElementsBucket->m_pVideoCallSessionList->IsVideoSessionExist(lFriendID, pVideoSession);

//	LOGE("CController::PushPacketForDecoding video session exists");

	if (bExist)
	{
//		LOGE("CController::ParseFrameIntoPackets getting PushPacketForDecoding");
//		CVideoDecoder *pCVideoDecoder = pVideoSession->GetVideoDecoder();
//		LOGE("CController::ParseFrameIntoPackets got PushPacketForDecoding1");
//		CEncodedFrameDepacketizer *p_CEncodedFrameDepacketizer = pCVideoDecoder->GetEncodedFrameDepacketizer();
//		CEncodedFrameDepacketizer *p_CEncodedFrameDepacketizer = pVideoSession->GetEncodedFrameDepacketizer();
//		LOGE("CController::ParseFrameIntoPackets got PushPacketForDecoding2");
//		CLogPrinter_WriteSpecific(CLogPrinter::DEBUGS, " CNTRL SIGBYTE: "+ m_Tools.IntegertoStringConvert((int)in_data[1+SIGNAL_BYTE_INDEX]));
		if (pVideoSession)
			return pVideoSession->PushPacketForMerging(in_data, in_size, false, numberOfFrames, frameSizes, numberOfMissingFrames, missingFrames);
		else
			return -1;
	}
	else
	{
		return -1;
	}
}

int CController::PushAudioForDecodingVector(const LongLong& lFriendID, int nOffset, unsigned char *in_data, unsigned int in_size, int numberOfFrames, int *frameSizes, std::vector< std::pair<int, int> > vMissingFrames)
{
	CAudioCallSession* pAudioSession;

	CLogPrinter_Write(CLogPrinter::DEBUGS, "CController::PushAudioForDecoding called");

	//	LOGE("CController::PushPacketForDecoding");

	Locker lock(*m_pAudioReceiveMutex);

	bool bExist = m_pCommonElementsBucket->m_pAudioCallSessionList->IsAudioSessionExist(lFriendID, pAudioSession);

	//	LOGE("CController::PushPacketForDecoding Audio session exists");

	if (bExist)
	{
		//LOGE("CController::ParseFrameIntoPackets getting PushPacketForDecoding");

		CLogPrinter_Write(CLogPrinter::DEBUGS, "CController::PushAudioForDecoding called 2");

		//if (pCAudioDecoder)

		{
			CLogPrinter_Write(CLogPrinter::DEBUGS, "pCAudioDecoder exists");
			return pAudioSession->DecodeAudioDataVector(nOffset, in_data, in_size, numberOfFrames, frameSizes, vMissingFrames);
		}

		/*else
		{
		CLogPrinter_Write(CLogPrinter::DEBUGS, "pCAudioDecoder doesnt exist");
		return -1;
		}*/
	}
	else
	{
		return -1;
	}
}

int CController::PushAudioForDecoding(const LongLong& lFriendID, int nOffset, unsigned char *in_data, unsigned int in_size, int numberOfFrames, int *frameSizes, int numberOfMissingFrames, int *missingFrames)
{
	CAudioCallSession* pAudioSession;

	CLogPrinter_Write(CLogPrinter::DEBUGS, "CController::PushAudioForDecoding called");

	//	LOGE("CController::PushPacketForDecoding");
    
    Locker lock(*m_pAudioReceiveMutex);

	bool bExist = m_pCommonElementsBucket->m_pAudioCallSessionList->IsAudioSessionExist(lFriendID, pAudioSession);

	//	LOGE("CController::PushPacketForDecoding Audio session exists");

	if (bExist)
	{
				//LOGE("CController::ParseFrameIntoPackets getting PushPacketForDecoding");
        
        CLogPrinter_Write(CLogPrinter::DEBUGS, "CController::PushAudioForDecoding called 2");

		//if (pCAudioDecoder)
        
        {
            CLogPrinter_Write(CLogPrinter::DEBUGS, "pCAudioDecoder exists");
            return pAudioSession->DecodeAudioData(nOffset,in_data, in_size, numberOfFrames, frameSizes, numberOfMissingFrames, missingFrames);
        }
			
		/*else
        {
            CLogPrinter_Write(CLogPrinter::DEBUGS, "pCAudioDecoder doesnt exist");
			return -1;
        }*/
	}
	else
	{
		return -1;
	}
}

long long g_lPrevAudioFrame = 0;

int iDataSentInCurrentSec = 0;
long long llTimeStamp = 0;
int CController::SendAudioData(const LongLong& lFriendID, short *in_data, unsigned int in_size)
{
	long long llNow = m_Tools.CurrentTimestamp();
	if(llNow - llTimeStamp >= 1000)
	{
//		CLogPrinter_WriteLog(CLogPrinter::INFO, INSTENT_TEST_LOG, "Num AudioDataSent = " + m_Tools.IntegertoStringConvert(iDataSentInCurrentSec));
		iDataSentInCurrentSec = 0;
		llTimeStamp = llNow;
	}
	iDataSentInCurrentSec ++;
	CAudioCallSession* pAudioSession;

	CLogPrinter_Write(CLogPrinter::INFO, "CController::SendAudioData");
    
    Locker lock(*m_pAudioSendMutex);

	bool bExist = m_pCommonElementsBucket->m_pAudioCallSessionList->IsAudioSessionExist(lFriendID, pAudioSession);

	CLogPrinter_Write(CLogPrinter::INFO, "CController::SendAudioData audio session exists");

	if (bExist)
	{
		CLogPrinter_Write(CLogPrinter::INFO, "CController::SendAudioData getting encoder");
		//CAudioCodec *pAudioEncoder = pAudioSession->GetAudioCodec();
		CLogPrinter_Write(CLogPrinter::INFO, "CController::SendAudioData got encoder");

		//if (pAudioEncoder)
		{
            /*for(int i=0;i<in_size;i++)
            {
                in_data[i] = rand()%255;
            }*/
            long long lEncodeStartTime = m_Tools.CurrentTimestamp();
            
            int ret = pAudioSession->EncodeAudioData(in_data,in_size);
            //CLogPrinter_WriteLog(CLogPrinter::INFO, INSTENT_TEST_LOG, "TheKing--> AudioEncodingTime = " + m_Tools.LongLongtoStringConvert((m_Tools.CurrentTimestamp() - lEncodeStartTime)) + "asdfasdf");
            //printf("TheKing--> zAudioEncodingTime = %lld, zAudioEncodingTimeDiff = %lld, in_size = %d\n", m_Tools.CurrentTimestamp() - lEncodeStartTime, m_Tools.CurrentTimestamp() - g_lPrevAudioFrame, in_size);
            g_lPrevAudioFrame = m_Tools.CurrentTimestamp();
            return ret;
		}
		
	}
	else
	{
		return -1;
	}
}

int CController::SendVideoData(const LongLong& lFriendID, unsigned char *in_data, unsigned int in_size, unsigned int orientation_type, int device_orientation)
{
	/*if (g_StopVideoSending)
	{
		CLogPrinter_WriteSpecific6(CLogPrinter::DEBUGS, "SendVideoData stopped");
		return -1;

	}*/
	CVideoCallSession* pVideoSession;

	CLogPrinter_Write(CLogPrinter::DEBUGS, "CController::EncodeAndTransfer called");
    
    
    Locker lock(*m_pVideoSendMutex);

	bool bExist = m_pCommonElementsBucket->m_pVideoCallSessionList->IsVideoSessionExist(lFriendID, pVideoSession);

	if (bExist)
	{
		CLogPrinter_Write(CLogPrinter::INFO, "CController::EncodeAndTransfer getting encoder");

//		CVideoEncoder *pVideoEncoder = pVideoSession->GetVideoEncoder();

		CLogPrinter_Write(CLogPrinter::INFO, "CController::EncodeAndTransfer got encoder");

		if (pVideoSession)
		{
			if (in_size > MAX_VIDEO_ENCODER_FRAME_SIZE)
				return -1;

			pVideoSession->m_pVideoEncodingThread->SetOrientationType(orientation_type);
			return pVideoSession->PushIntoBufferForEncoding(in_data, in_size, device_orientation);
		}
		else
			return -1;
	}
	else
	{
		return -1;
	}
}

int CController::SetEncoderHeightWidth(const LongLong& lFriendID, int height, int width)
{
	CVideoCallSession* pVideoSession;

	if(height * width > 352 * 288)
	{
		m_Quality[1].iHeight = height;
		m_Quality[1].iWidth = width;
	}
	else
	{
		m_Quality[0].iHeight = height;
		m_Quality[0].iWidth = width;
	}

	bool bExist = m_pCommonElementsBucket->m_pVideoCallSessionList->IsVideoSessionExist(lFriendID, pVideoSession);

	if (bExist)
	{
		return pVideoSession->SetEncoderHeightWidth(lFriendID, height, width);
	}
	else
	{
		return -1;
	}
}

int CController::SetBitRate(const LongLong& lFriendID, int bitRate)
{
	return -1;
}

int CController::CheckDeviceCapability(const LongLong& lFriendID, int iHeightHigh, int iWidthHigh, int iHeightLow, int iWidthLow)
{
	CLogPrinter_WriteLog(CLogPrinter::INFO, CHECK_CAPABILITY_LOG, "CController::CheckDeviceCapability CheckDeviceCapability started");

        m_Quality[0].iHeight = iHeightLow;
        m_Quality[0].iWidth = iWidthLow;
        m_Quality[1].iHeight = iHeightHigh;
        m_Quality[1].iWidth = iWidthHigh;

    if(m_bLiveCallRunning == true) return -1;

	if (m_bDeviceCapabilityRunning == true) return -1;
    
    m_bDeviceCapabilityRunning = true;
    
	if (m_pDeviceCapabilityCheckBuffer->GetQueueSize() == 0)
		m_pDeviceCapabilityCheckThread->StartDeviceCapabilityCheckThread(iHeightHigh, iWidthHigh);

	m_pDeviceCapabilityCheckBuffer->Queue(lFriendID, START_DEVICE_CHECK, DEVICE_CHECK_STARTING, iHeightHigh, iWidthHigh);

	CLogPrinter_WriteLog(CLogPrinter::INFO, CHECK_CAPABILITY_LOG, "CController::CheckDeviceCapability CheckDeviceCapability start instruction queued");
    
    return 1;
}

int CController::SetDeviceCapabilityResults(int iNotification, int iHeightHigh, int iWidthHigh, int iHeightLow, int iWidthLow)
{
    m_Quality[0].iHeight = iHeightLow;
    m_Quality[0].iWidth = iWidthLow;
    m_Quality[1].iHeight = iHeightHigh;
    m_Quality[1].iWidth = iWidthHigh;
    
    if(iNotification == CEventNotifier::SET_CAMERA_RESOLUTION_640x480_25FPS)
    {
        m_nSupportedResolutionFPSLevel = SUPPORTED_RESOLUTION_FPS_640_25;
    }
    else if(iNotification == CEventNotifier::SET_CAMERA_RESOLUTION_352x288_25FPS)
    {
        m_nSupportedResolutionFPSLevel = SUPPORTED_RESOLUTION_FPS_352_25;
    }
    else if(iNotification == CEventNotifier::SET_CAMERA_RESOLUTION_352x288_25FPS_NOT_SUPPORTED)
    {
        m_nSupportedResolutionFPSLevel = SUPPORTED_RESOLUTION_FPS_352_15;
        
    }
    else
    {
        m_nSupportedResolutionFPSLevel = RESOLUTION_FPS_SUPPORT_NOT_TESTED;
    }
    
    return 1;
}

void CController::initializeEventHandler()
{
	m_pCommonElementsBucket->m_pEventNotifier = &m_EventNotifier;

	CLogPrinter_Write(CLogPrinter::DEBUGS, "CController::initializeEventHandler() EventHandler Initialized");
}

bool CController::StopAudioCall(const LongLong& lFriendID)
{
    CLogPrinter_Write(CLogPrinter::ERRORS, "CController::StopAudioCall() called.");
    
    Locker lock1(*m_pAudioSendMutex);
    Locker lock2(*m_pAudioReceiveMutex);
    
    CAudioCallSession *m_pSession;
    
    m_pSession = m_pCommonElementsBucket->m_pAudioCallSessionList->GetFromAudioSessionList(lFriendID);
    
    if (NULL == m_pSession)
    {
        CLogPrinter_Write(CLogPrinter::ERRORS, "CController::StopAudioCall() Session Does not Exist.");
        return false;
    }

    bool bReturnedValue = m_pCommonElementsBucket->m_pAudioCallSessionList->RemoveFromAudioSessionList(lFriendID);

	CLogPrinter_Write(CLogPrinter::ERRORS, "CController::StopAudioCall() ended " + m_Tools.IntegertoStringConvert(bReturnedValue));
    
    return bReturnedValue;
}

bool CController::StopTestAudioCall(const LongLong& lFriendID)
{
	CLogPrinter_WriteLog(CLogPrinter::INFO, CHECK_CAPABILITY_LOG, "CController::StopTestAudioCall() called");

	Locker lock1(*m_pAudioSendMutex);
	Locker lock2(*m_pAudioReceiveMutex);

	CLogPrinter_WriteLog(CLogPrinter::INFO, CHECK_CAPABILITY_LOG, "CController::StopTestAudioCall() checking session");

	CAudioCallSession *m_pSession;

	m_pSession = m_pCommonElementsBucket->m_pAudioCallSessionList->GetFromAudioSessionList(lFriendID);

	if (NULL == m_pSession)
	{
		CLogPrinter_WriteLog(CLogPrinter::INFO, CHECK_CAPABILITY_LOG, "CController::StopTestAudioCall() session doesn't exists");

		return false;
	}

	bool bReturnedValue = m_pCommonElementsBucket->m_pAudioCallSessionList->RemoveFromAudioSessionList(lFriendID);

	CLogPrinter_WriteLog(CLogPrinter::INFO, CHECK_CAPABILITY_LOG, "CController::StopTestAudioCall() ended with bReturnedValue " + m_Tools.IntegertoStringConvert(bReturnedValue));

	return bReturnedValue;
}

bool CController::StopTestVideoCall(const LongLong& lFriendID)
{
	CLogPrinter_WriteLog(CLogPrinter::INFO, CHECK_CAPABILITY_LOG, "CController::StopVideoCall() called");

	Locker lock1(*m_pVideoSendMutex);
	Locker lock2(*m_pVideoReceiveMutex);

	CLogPrinter_WriteLog(CLogPrinter::INFO, CHECK_CAPABILITY_LOG, "CController::StopVideoCall() checking session");

	CVideoCallSession *m_pSession;

	m_pSession = m_pCommonElementsBucket->m_pVideoCallSessionList->GetFromVideoSessionList(lFriendID);

	if (NULL == m_pSession)
	{
		CLogPrinter_WriteLog(CLogPrinter::INFO, CHECK_CAPABILITY_LOG, "CController::StopVideoCall() session doesn't exists");

		return false;
	}

	bool bReturnedValue = m_pCommonElementsBucket->m_pVideoCallSessionList->RemoveFromVideoSessionList(lFriendID);

	CLogPrinter_WriteLog(CLogPrinter::INFO, CHECK_CAPABILITY_LOG, "CController::StopVideoCall() ended with bReturnedValue " + m_Tools.IntegertoStringConvert(bReturnedValue));

    m_bDeviceCapabilityRunning = false;
    
	return bReturnedValue;
}

bool CController::StopVideoCall(const LongLong& lFriendID)
{
    CLogPrinter_Write(CLogPrinter::ERRORS, "CController::StopVideoCall() called.");
//    CLogPrinter_WriteLog(CLogPrinter::INFO, INSTENT_TEST_LOG, "StopVideo call operation started");
    
    Locker lock1(*m_pVideoSendMutex);
//    CLogPrinter_WriteLog(CLogPrinter::INFO, INSTENT_TEST_LOG, "StopVideo call After first lock");
    Locker lock2(*m_pVideoReceiveMutex);
//    CLogPrinter_WriteLog(CLogPrinter::INFO, INSTENT_TEST_LOG, "StopVideo call After Second lock");
    
    CVideoCallSession *m_pSession;
    
    m_pSession = m_pCommonElementsBucket->m_pVideoCallSessionList->GetFromVideoSessionList(lFriendID);

    if (NULL == m_pSession)
    {
        CLogPrinter_Write(CLogPrinter::ERRORS, "CController::StopVideoCall() Session Does not Exist.");
        return false;
    }

    bool bReturnedValue = m_pCommonElementsBucket->m_pVideoCallSessionList->RemoveFromVideoSessionList(lFriendID);

    CLogPrinter_Write(CLogPrinter::ERRORS, "CController::StopVideoall() ended " + m_Tools.IntegertoStringConvert(bReturnedValue));
    
    
    m_bLiveCallRunning = false;
    
    return bReturnedValue;
}

int CController::StartAudioEncodeDecodeSession()
{
	if (NULL == m_pAudioEncodeDecodeSession)
		m_pAudioEncodeDecodeSession = new CAudioFileEncodeDecodeSession();

	return m_pAudioEncodeDecodeSession->StartAudioEncodeDecodeSession();
}

int CController::EncodeAudioFrame(short *psaEncodingDataBuffer, int nAudioFrameSize, unsigned char *ucaEncodedDataBuffer)
{
	if (NULL == m_pAudioEncodeDecodeSession)
	{
		return 0;
	}
	else
		return m_pAudioEncodeDecodeSession->EncodeAudioFile(psaEncodingDataBuffer, nAudioFrameSize, ucaEncodedDataBuffer);
}

int CController::DecodeAudioFrame(unsigned char *ucaDecodedDataBuffer, int nAudioFrameSize, short *psaDecodingDataBuffer)
{
	if (NULL == m_pAudioEncodeDecodeSession)
	{
		return 0;
	}
	else
		return m_pAudioEncodeDecodeSession->DecodeAudioFile(ucaDecodedDataBuffer, nAudioFrameSize, psaDecodingDataBuffer);
}

int CController::StopAudioEncodeDecodeSession()
{
	if (NULL != m_pAudioEncodeDecodeSession)
	{
		m_pAudioEncodeDecodeSession->StopAudioEncodeDecodeSession();

		delete m_pAudioEncodeDecodeSession;

		m_pAudioEncodeDecodeSession = NULL;

		return 1;
	}
	else
		return 0;
}

int CController::StartVideoMuxingAndEncodeSession(unsigned char *pBMP32Data,int iLen, int nVideoHeight, int nVideoWidth)
{
	LOGE("fahad -->> CController::StartVideoMuxingAndEncodeSession 0");
	if (NULL == m_pVideoMuxingAndEncodeSession)
	{
		LOGE("fahad -->> CController::StartVideoMuxingAndEncodeSession null");
		m_pVideoMuxingAndEncodeSession = new CVideoMuxingAndEncodeSession(m_pCommonElementsBucket);
	}

	return m_pVideoMuxingAndEncodeSession->StartVideoMuxingAndEncodeSession(pBMP32Data, iLen, nVideoHeight, nVideoWidth);
}

int CController::FrameMuxAndEncode( unsigned char *pVideoYuv, int iHeight, int iWidth, unsigned char *pMergedData)
{
	if (NULL == m_pVideoMuxingAndEncodeSession)
	{
		return 0;
	}
	else
		return m_pVideoMuxingAndEncodeSession->FrameMuxAndEncode(pVideoYuv, iHeight, iWidth, pMergedData);
}

int CController::StopVideoMuxingAndEncodeSession()
{
	if (NULL != m_pVideoMuxingAndEncodeSession)
	{
		m_pVideoMuxingAndEncodeSession->StopVideoMuxingAndEncodeSession();

		delete m_pVideoMuxingAndEncodeSession;

		m_pVideoMuxingAndEncodeSession = NULL;

		return 1;
	}
	else
		return 0;
}


void CController::UninitializeLibrary()
{
	CLogPrinter_Write(CLogPrinter::INFO, "CController::UninitializeLibrary() for all friend and all media");

	if (NULL != m_pDeviceCapabilityCheckThread)
	{
		m_pDeviceCapabilityCheckThread->StopDeviceCapabilityCheckThread();
	}

	m_pCommonElementsBucket->m_pVideoCallSessionList->ClearAllFromVideoSessionList();
	m_pCommonElementsBucket->m_pVideoEncoderList->ClearAllFromVideoEncoderList();
}

void CController::SetNotifyClientWithPacketCallback(void(*callBackFunctionPointer)(LongLong, unsigned char*, int))
{
    m_EventNotifier.SetNotifyClientWithPacketCallback(callBackFunctionPointer);
}

void CController::SetNotifyClientWithVideoDataCallback(void(*callBackFunctionPointer)(LongLong, int, unsigned char*, int, int, int, int))
{
	m_EventNotifier.SetNotifyClientWithVideoDataCallback(callBackFunctionPointer);
}

void CController::SetNotifyClientWithVideoNotificationCallback(void(*callBackFunctionPointer)(LongLong, int))
{
	m_EventNotifier.SetNotifyClientWithVideoNotificationCallback(callBackFunctionPointer);
}

void CController::SetNotifyClientWithNetworkStrengthNotificationCallback(void(*callBackFunctionPointer)(IPVLongType, int))
{
	m_EventNotifier.SetNotifyClientWithNetworkStrengthNotificationCallback(callBackFunctionPointer);
}

void CController::SetNotifyClientWithAudioDataCallback(void(*callBackFunctionPointer)(LongLong, int, short*, int))
{
    m_EventNotifier.SetNotifyClientWithAudioDataCallback(callBackFunctionPointer);
}

void CController::SetNotifyClientWithAudioPacketDataCallback(void(*callBackFunctionPointer)(IPVLongType, unsigned char*, int))
{
	m_EventNotifier.SetNotifyClientWithAudioPacketDataCallback(callBackFunctionPointer);
}

void CController::SetNotifyClientWithAudioAlarmCallback(void(*callBackFunctionPointer)(LongLong, short*, int))
{
	m_EventNotifier.SetNotifyClientWithAudioAlarmCallback(callBackFunctionPointer);
}

void CController::SetSendFunctionPointer(void(*callBackFunctionPointer)(LongLong, int, unsigned char*, int, int))
{
    m_pCommonElementsBucket->SetSendFunctionPointer(callBackFunctionPointer);
}

