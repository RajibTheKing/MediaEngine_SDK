#include "Controller.h"
#include "LogPrinter.h"
#include "Tools.h"
#include "VideoCallSession.h"
#include "VideoEncoder.h"
#include "VideoDecoder.h"
#include "AudioSessionOptions.h"
#include "AudioResources.h"
#include "AudioCallSession.h"
#include "InterfaceOfAudioVideoEngine.h"



namespace MediaSDK
{

class AudioSessionOptions;

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
m_bCallInLiveEnabled(false),
m_EventNotifier(this),
m_llLastTimeStamp(-1)
{
	CLogPrinter::Start(CLogPrinter::DEBUGS, "");
	CLogPrinter_Write(CLogPrinter::DEBUGS, "CController::CController() AudioVideoEngine Initializing");

	m_nDeviceDisplayHeight = -1;
	m_nDeviceDisplayWidth = -1;
	
	m_pCommonElementsBucket = new CCommonElementsBucket();
    
    m_pVideoSendMutex.reset(new CLockHandler);
	m_pVideoStartMutex.reset(new CLockHandler);
    m_pVideoReceiveMutex.reset(new CLockHandler);
    m_pAudioSendMutex.reset(new CLockHandler);
    m_pAudioReceiveMutex.reset(new CLockHandler);
	m_pAudioLockMutex.reset(new CLockHandler);

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
m_bDeviceCapabilityRunning(false),
m_bLiveCallRunning(false),
m_bCallInLiveEnabled(false),
m_EventNotifier(this),
m_llLastTimeStamp(-1)
{
	CLogPrinter::Start((CLogPrinter::Priority)iLoggerPrintLevel, sLoggerPath);
	CLogPrinter_Write(CLogPrinter::DEBUGS, "CController::CController() AudioVideoEngine Initializing");

	m_nDeviceDisplayHeight = -1;
	m_nDeviceDisplayWidth = -1;

	m_pCommonElementsBucket = new CCommonElementsBucket();
    
    m_pVideoSendMutex.reset(new CLockHandler);
	m_pVideoStartMutex.reset(new CLockHandler);
    m_pVideoReceiveMutex.reset(new CLockHandler);
    m_pAudioSendMutex.reset(new CLockHandler);
    m_pAudioReceiveMutex.reset(new CLockHandler);
	m_pAudioLockMutex.reset(new CLockHandler);

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

	CLogPrinter::Stop();
    
    SHARED_PTR_DELETE(m_pVideoSendMutex);
	SHARED_PTR_DELETE(m_pVideoStartMutex);
    SHARED_PTR_DELETE(m_pVideoReceiveMutex);
    SHARED_PTR_DELETE(m_pAudioSendMutex);
    SHARED_PTR_DELETE(m_pAudioReceiveMutex);
	
	SHARED_PTR_DELETE(m_pAudioLockMutex);

	CLogPrinter_Write(CLogPrinter::WARNING, "CController::~CController() removed everything");
}

bool CController::SetUserName(const long long& lUserName)
{
	m_pCommonElementsBucket->SetUserName(lUserName);

	CLogPrinter_Write(CLogPrinter::DEBUGS, "CController::SetUserName() user name: " + m_Tools.LongLongtoStringConvert(lUserName));

	return true;
}

bool CController::StartAudioCall(const long long& lFriendID, int nServiceType, int nEntityType, int nAudioSpeakerType, bool bOpusCodec)
{
	StartAudioCallLocker lock3(*m_pAudioLockMutex);
	CAudioCallSession* pAudioSession;

	bool bExist = m_pCommonElementsBucket->m_pAudioCallSessionList->IsAudioSessionExist(lFriendID, pAudioSession);

	CLogPrinter_Write(CLogPrinter::INFO, "CController::StartAudioCall");

	if (!bExist)
	{
		CLogPrinter_Write(CLogPrinter::INFO, "CController::StartAudioCall Session empty");

		AudioSessionOptions audioSessionOptions;
		audioSessionOptions.SetOptions(nServiceType, nEntityType);
		AudioResources audioResources(audioSessionOptions);
		
		pAudioSession = new CAudioCallSession(m_bLiveCallRunning, lFriendID, m_pCommonElementsBucket, nServiceType, nEntityType, audioResources, nAudioSpeakerType, bOpusCodec);

		m_pCommonElementsBucket->m_pAudioCallSessionList->AddToAudioSessionList(lFriendID, pAudioSession);

		CLogPrinter_Write(CLogPrinter::INFO, "CController::StartAudioCall Session started");

		return true;
	}
	else
	{
		return false;
	}
}

bool CController::SetVolume(const long long& lFriendID, int iVolume, bool bRecorder)
{
	CAudioCallSession* pAudioSession;
	
	SetVolumeLocker lock1(*m_pAudioLockMutex);

	bool bExist = m_pCommonElementsBucket->m_pAudioCallSessionList->IsAudioSessionExist(lFriendID, pAudioSession);
	if (bExist)
	{
		pAudioSession->SetVolume(iVolume, bRecorder);
		return true;
	}
	else
	{
		return false;
	}
}

bool CController::SetSpeakerType(const long long& lFriendID, int iSpeakerType)
{
	SetLoudSpeakerLocker lock1(*m_pAudioLockMutex);

	CAudioCallSession* pAudioSession;

	bool bExist = m_pCommonElementsBucket->m_pAudioCallSessionList->IsAudioSessionExist(lFriendID, pAudioSession);
	if (bExist)
	{
		pAudioSession->SetSpeakerType(iSpeakerType);
		return true;
	}
	else
	{
		return false;
	}
}

bool CController::SetEchoCanceller(const long long& lFriendID, bool bOn)
{
	SetEchoCancellerLocker lock1(*m_pAudioLockMutex);
	CAudioCallSession* pAudioSession;
	//return false;

	bool bExist = m_pCommonElementsBucket->m_pAudioCallSessionList->IsAudioSessionExist(lFriendID, pAudioSession);
	if (bExist)
	{
		pAudioSession->SetEchoCanceller(bOn);
		return true;
	}
	else
	{
		return false;
	}
}

bool CController::StartTestAudioCall(const long long& lFriendID)
{
	CLogPrinter_WriteLog(CLogPrinter::INFO, CHECK_CAPABILITY_LOG, "CController::StartTestAudioCall() called");

	CAudioCallSession* pAudioSession;

	StartTestAudioCallLocker lock(*m_pAudioLockMutex);

	bool bExist = m_pCommonElementsBucket->m_pAudioCallSessionList->IsAudioSessionExist(lFriendID, pAudioSession);

	if (!bExist)
	{
		CLogPrinter_WriteLog(CLogPrinter::INFO, CHECK_CAPABILITY_LOG, "CController::StartTestAudioCall() session creating");
		AudioSessionOptions audioSessionOptions;
		audioSessionOptions.SetOptions(SERVICE_TYPE_CALL, DEVICE_ABILITY_CHECK_MOOD);
		AudioResources audioResources(audioSessionOptions);

		pAudioSession = new CAudioCallSession(m_bLiveCallRunning, lFriendID, m_pCommonElementsBucket, SERVICE_TYPE_CALL, DEVICE_ABILITY_CHECK_MOOD, audioResources, AUDIO_PLAYER_DEFAULT, true);

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

CVideoCallSession* CController::StartTestVideoCall(const long long& lFriendID, int iVideoHeight, int iVideoWidth, int iNetworkType)
{

	CLogPrinter_WriteLog(CLogPrinter::INFO, CHECK_CAPABILITY_LOG, "CController::StartTestVideoCall() called");
    
	CVideoCallSession* pVideoSession;

	bool bExist = m_pCommonElementsBucket->m_pVideoCallSessionList->IsVideoSessionExist(lFriendID, pVideoSession);

	if (!bExist)
	{
		CLogPrinter_WriteLog(CLogPrinter::INFO, CHECK_CAPABILITY_LOG, "CController::StartTestVideoCall() session creating");

		pVideoSession = new CVideoCallSession(this, lFriendID, m_pCommonElementsBucket, HIGH_FRAME_RATE, &m_nDeviceSupportedCallFPS, DEVICE_ABILITY_CHECK_MOOD, m_pDeviceCapabilityCheckBuffer, m_nSupportedResolutionFPSLevel, SERVICE_TYPE_CALL, ENTITY_TYPE_CALLER, false, false);

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

bool CController::StartVideoCall(const long long& lFriendID, int iVideoHeight, int iVideoWidth, int nServiceType, int nEntityType, int iNetworkType, bool bAudioOnlyLive, bool bSelfViewOnly)
{
	CLogPrinter_LOG(API_FLOW_CHECK_LOG, "CController::StartVideoCall called 1 ID %lld", lFriendID);

	StartCallLocker lock1(*m_pVideoStartMutex);

	CLogPrinter_LOG(API_FLOW_CHECK_LOG, "CController::StartVideoCall called 2 ID %lld", lFriendID);

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

	long long llCheckDeviceCapabilityStartTime = m_Tools.CurrentTimestamp();

	while (m_bDeviceCapabilityRunning == true)
	{
		m_Tools.SOSleep(10);

		if (m_Tools.CurrentTimestamp() - llCheckDeviceCapabilityStartTime > LIVE_START_HEIGHEST_WAITTIME)
		{
			int notification = m_EventNotifier.VIDEO_SESSION_START_FAILED;
			//m_EventNotifier.fireVideoNotificationEvent(lFriendID, notification);
			break;
		}
	}
    
    if(m_bDeviceCapabilityRunning == true) return false;
    
    
	CVideoCallSession* pVideoSession;

	CLogPrinter_Write(CLogPrinter::INFO, "CController::StartVideoCall called");
    
    //Locker lock1(*m_pVideoSendMutex);
    //Locker lock2(*m_pVideoReceiveMutex);

	CLogPrinter_LOG(API_FLOW_CHECK_LOG, "CController::StartVideoCall called checking session ID %lld", lFriendID);

	bool bExist = m_pCommonElementsBucket->m_pVideoCallSessionList->IsVideoSessionExist(lFriendID, pVideoSession);

	CLogPrinter_LOG(API_FLOW_CHECK_LOG, "CController::StartVideoCall session size %d bExist %d ID %lld", m_pCommonElementsBucket->m_pVideoCallSessionList->GetSessionListSize(), bExist, lFriendID);

	if (!bExist)
	{
        m_bLiveCallRunning = true;
        
		CLogPrinter_Write(CLogPrinter::DEBUGS, "CController::StartVideoCall Video Session starting");

		pVideoSession = new CVideoCallSession(this, lFriendID, m_pCommonElementsBucket, m_nDeviceSupportedCallFPS, &m_nDeviceSupportedCallFPS, LIVE_CALL_MOOD, NULL, m_nSupportedResolutionFPSLevel, nServiceType, nEntityType, bAudioOnlyLive, bSelfViewOnly);

		pVideoSession->InitializeVideoSession(lFriendID, iVideoHeight, iVideoWidth,nServiceType,iNetworkType);

		m_pCommonElementsBucket->m_pVideoCallSessionList->AddToVideoSessionList(lFriendID, pVideoSession);

		CLogPrinter_Write(CLogPrinter::DEBUGS, "CController::StartVideoCall Video Session started");

		CLogPrinter_LOG(API_FLOW_CHECK_LOG, "CController::StartVideoCall called session created ID %lld", lFriendID);

		return true;
	}
	else
	{
		CLogPrinter_LOG(API_FLOW_CHECK_LOG, "CController::StartVideoCall called create failed ID %lld", lFriendID);

       // pVideoSession->ReInitializeVideoLibrary(iVideoHeight, iVideoWidth);
		return false;
	}	

	CLogPrinter_LOG(API_FLOW_CHECK_LOG, "CController::StartVideoCall done ID %lld", lFriendID);
}

int CController::EncodeVideoFrame(const long long& lFriendID, unsigned char *in_data, unsigned int in_size)
{
	CVideoCallSession* pVideoSession;

	CLogPrinter_Write(CLogPrinter::DEBUGS, "CController::EncodeAndTransfer called");
    
	ControllerEncodeVideoFrameLocker lock(*m_pVideoSendMutex);

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

int CController::PushPacketForDecodingVector(const long long& lFriendID, int offset, unsigned char *in_data, unsigned int in_size, int numberOfFrames, int *frameSizes, std::vector< std::pair<int, int> > vMissingFrames)
{
	CVideoCallSession* pVideoSession = NULL;

	CLogPrinter_Write(CLogPrinter::DEBUGS, "CController::PushPacketForDecoding called");

	//	LOGE("CController::PushPacketForDecoding");

	ReceiveLockerLive lock(*m_pVideoReceiveMutex);

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

int CController::PushPacketForDecoding(const long long& lFriendID,unsigned char *in_data, unsigned int in_size, int numberOfFrames, int *frameSizes, int numberOfMissingFrames, int *missingFrames)
{
	CVideoCallSession* pVideoSession = NULL;

	CLogPrinter_Write(CLogPrinter::DEBUGS, "CController::PushPacketForDecoding called");

//	LOGE("CController::PushPacketForDecoding");
    
	ReceiveLockerCall lock(*m_pVideoReceiveMutex);

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

int CController::PushAudioForDecoding(const long long& lFriendID, int nOffset, unsigned char *in_data, unsigned int in_size, int numberOfFrames, int *frameSizes, std::vector< std::pair<int, int> > vMissingFrames)
{
	CAudioCallSession* pAudioSession;

	CLogPrinter_Write(CLogPrinter::DEBUGS, "CController::PushAudioForDecoding called");

	//	LOGE("CController::PushPacketForDecoding");
	
	PushAudioForDecodingLocker lock2(*m_pAudioLockMutex);


	bool bExist = m_pCommonElementsBucket->m_pAudioCallSessionList->IsAudioSessionExist(lFriendID, pAudioSession);

	//	LOGE("CController::PushPacketForDecoding Audio session exists");

	if (bExist)
	{
		//LOGE("CController::ParseFrameIntoPackets getting PushPacketForDecoding");

		CLogPrinter_Write(CLogPrinter::DEBUGS, "CController::PushAudioForDecoding called 2");

		//if (pCAudioDecoder)

		{
			CLogPrinter_Write(CLogPrinter::DEBUGS, "pCAudioDecoder exists");
			return pAudioSession->DecodeAudioData(nOffset, in_data, in_size, numberOfFrames, frameSizes, vMissingFrames);
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
int CController::SendAudioData(const long long& lFriendID, short *in_data, unsigned int in_size)
{
	//if ((m_nServiceType == SERVICE_TYPE_LIVE_STREAM || m_nServiceType == SERVICE_TYPE_SELF_STREAM || m_nServiceType == SERVICE_TYPE_CHANNEL) && m_nCallInLiveType == CALL_IN_LIVE_TYPE_AUDIO_ONLY)
	//	return -5;

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
    
	SendAudioDataLocker lock2(*m_pAudioLockMutex);

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
            
            int ret = pAudioSession->PushAudioData(in_data,in_size);
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

int CController::CancelAudioData(const long long& lFriendID, short *in_data, unsigned int in_size)
{
	CancelAudioDataLocker lock2(*m_pAudioLockMutex);
	CAudioCallSession* pAudioSession;

	CLogPrinter_Write(CLogPrinter::INFO, "CController::CancelAudioData");

	bool bExist = m_pCommonElementsBucket->m_pAudioCallSessionList->IsAudioSessionExist(lFriendID, pAudioSession);

	CLogPrinter_Write(CLogPrinter::INFO, "CController::SendAudioData audio session exists");

	if (bExist)
	{
		int ret = pAudioSession->CancelAudioData(in_data, in_size);
		return ret;		
	}
	else
	{
		return -1;
	}
}


int CController::SendVideoData(const long long& lFriendID, unsigned char *in_data, unsigned int in_size, unsigned int orientation_type, int device_orientation)
{
	/*if (g_StopVideoSending)
	{
		CLogPrinter_WriteSpecific6(CLogPrinter::DEBUGS, "SendVideoData stopped");
		return -1;

	}*/
	CVideoCallSession* pVideoSession;

	CLogPrinter_Write(CLogPrinter::DEBUGS, "CController::EncodeAndTransfer called");
    
    
	SendLocker lock(*m_pVideoSendMutex);

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

#ifdef OLD_ENCODING_THREAD

				pVideoSession->m_pVideoEncodingThread->SetOrientationType(orientation_type);
#else
			if (pVideoSession->GetServiceType() == SERVICE_TYPE_CALL || pVideoSession->GetServiceType() == SERVICE_TYPE_SELF_CALL)
				pVideoSession->m_pVideoEncodingThreadOfCall->SetOrientationType(orientation_type);
			else if (pVideoSession->GetServiceType() == SERVICE_TYPE_LIVE_STREAM || pVideoSession->GetServiceType() == SERVICE_TYPE_SELF_STREAM)
				pVideoSession->m_pVideoEncodingThreadOfLive->SetOrientationType(orientation_type);
#endif

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

int CController::SetEncoderHeightWidth(const long long& lFriendID, int height, int width, int nDataType)
{
	CVideoCallSession* pVideoSession;

	SetEncoderLocker lock(*m_pVideoSendMutex);
    
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
		return pVideoSession->SetEncoderHeightWidth(lFriendID, height, width, nDataType);
	}
	else
	{
		return -1;
	}
}

int CController::SetBeautification(const IPVLongType llFriendID, bool bIsEnable)
{
    CVideoCallSession* pVideoSession;

	SetBeautifyLocker lock(*m_pVideoSendMutex);
    
    bool bExist = m_pCommonElementsBucket->m_pVideoCallSessionList->IsVideoSessionExist(llFriendID, pVideoSession);
    
    if (bExist)
    {
        pVideoSession->SetBeautification(bIsEnable);
        return 1;
    }
    else
    {
        return -1;
    }
}

int CController::SetVideoQualityForLive(const IPVLongType llFriendID, int quality)
{
	CVideoCallSession* pVideoSession;

	SetBeautifyLocker lock(*m_pVideoSendMutex);

	bool bExist = m_pCommonElementsBucket->m_pVideoCallSessionList->IsVideoSessionExist(llFriendID, pVideoSession);

	if (bExist)
	{
		pVideoSession->SetVideoQualityForLive(quality);
		return 1;
	}
	else
	{
		return -1;
	}
}

int CController::SetVideoEffect(const long long llFriendID, int nEffectStatus)
{
	CVideoCallSession* pVideoSession;

	bool bExist = m_pCommonElementsBucket->m_pVideoCallSessionList->IsVideoSessionExist(llFriendID, pVideoSession);

	if (bExist)
	{
		return pVideoSession->SetVideoEffect(nEffectStatus);
	}
	else
	{
		return -1;
	}
}

void CController::SetCallInLiveType(const long long llFriendID, int nCallInLiveType)
{
	CVideoCallSession* pVideoSession;

	CLogPrinter_Write(CLogPrinter::DEBUGS, "CController::SetCallInLiveType called");

	SetCallInLiveTypeLocker lock(*m_pVideoSendMutex);
	SetCallInLiveTypeLocker lock2(*m_pAudioLockMutex);

	bool bExist = m_pCommonElementsBucket->m_pVideoCallSessionList->IsVideoSessionExist(llFriendID, pVideoSession);

	if (bExist)
	{
		CLogPrinter_Write(CLogPrinter::INFO, "CController::SetCallInLiveType got session");

		if (pVideoSession)
		{
			pVideoSession->SetCallInLiveType(nCallInLiveType);
		}
	}

	CAudioCallSession* pAudioSession;

	bExist = m_pCommonElementsBucket->m_pAudioCallSessionList->IsAudioSessionExist(llFriendID, pAudioSession);

	if (bExist)
	{
		CLogPrinter_Write(CLogPrinter::INFO, "CController::SetCallInLiveType got session");

		if (pAudioSession)
		{
			pAudioSession->SetCallInLiveType(nCallInLiveType);
		}
	}
}


int CController::TestVideoEffect(const long long llFriendID, int *param, int size)
{
	CVideoCallSession* pVideoSession;

	bool bExist = m_pCommonElementsBucket->m_pVideoCallSessionList->IsVideoSessionExist(llFriendID, pVideoSession);

	if (bExist)
	{
		return pVideoSession->TestVideoEffect(param, size);
	}
	else
	{
		return -1;
	}
}


int CController::SetDeviceDisplayHeightWidth(int height, int width)
{
	//Locker lock(*m_pVideoSendMutex);
	//Locker lock2(*m_pVideoReceiveMutex);

	m_nDeviceDisplayHeight = height;
	m_nDeviceDisplayWidth = width;
	return 1;
}

int CController::SetBitRate(const long long& lFriendID, int bitRate)
{
	return -1;
}

int CController::CheckDeviceCapability(const long long& lFriendID, int iHeightHigh, int iWidthHigh, int iHeightLow, int iWidthLow)
{
	CLogPrinter_LOG(API_FLOW_CHECK_LOG, "CController::CheckDeviceCapability called 1 ID %lld", lFriendID);

	StartCheckCapabilityLocker lock1(*m_pVideoStartMutex);

	CLogPrinter_LOG(API_FLOW_CHECK_LOG, "CController::CheckDeviceCapability called 2 ID %lld", lFriendID);

	if (m_bDeviceCapabilityRunning == true) return -1;
	m_bDeviceCapabilityRunning = true;

	CLogPrinter_WriteLog(CLogPrinter::INFO, CHECK_CAPABILITY_LOG, "CController::CheckDeviceCapability CheckDeviceCapability started --> ffiendID = " + m_Tools.getText(lFriendID));

    m_Quality[0].iHeight = iHeightLow;
    m_Quality[0].iWidth = iWidthLow;
    m_Quality[1].iHeight = iHeightHigh;
    m_Quality[1].iWidth = iWidthHigh;

    long long llCheckDeviceCapabilityStartTime = m_Tools.CurrentTimestamp();
    
    while(m_bLiveCallRunning==true)
    {
        m_Tools.SOSleep(10);

		if (m_Tools.CurrentTimestamp() - llCheckDeviceCapabilityStartTime > CALL_START_HIGHEST_WAITTIME)
        {
            int notification = m_EventNotifier.SET_CAMERA_RESOLUTION_FAILED;
            //m_EventNotifier.fireVideoNotificationEvent(lFriendID, notification);
            break;
        }
    }

    if(m_bLiveCallRunning == true)
    {
        m_bDeviceCapabilityRunning = false;
        return -1;
    }


    

    
	if (m_pDeviceCapabilityCheckBuffer->GetQueueSize() == 0)
	{

		int iRet = m_pDeviceCapabilityCheckThread->StartDeviceCapabilityCheckThread(iHeightHigh, iWidthHigh);

		if(iRet == -1)
		{
			m_bDeviceCapabilityRunning = false;
			return -1;
		}
	}

	CLogPrinter_LOG(API_FLOW_CHECK_LOG, "CController::CheckDeviceCapability queueing ID %lld", lFriendID);

	m_pDeviceCapabilityCheckBuffer->Queue(lFriendID, START_DEVICE_CHECK, DEVICE_CHECK_STARTING, iHeightHigh, iWidthHigh);

	CLogPrinter_LOG(API_FLOW_CHECK_LOG, "CController::CheckDeviceCapability done ID %lld", lFriendID);
	
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

void CController::InterruptOccured(const long long lFriendID)
{
	CVideoCallSession* pVideoSession;

	CLogPrinter_Write(CLogPrinter::DEBUGS, "CController::InterruptOccured called");

	CLogPrinter_LOG(API_FLOW_CHECK_LOG, "CController::InterruptOccured called 1 ID %lld", lFriendID);

	InterruptOccuredLocker lock(*m_pVideoSendMutex);

	CLogPrinter_LOG(API_FLOW_CHECK_LOG, "CController::InterruptOccured called 2 ID %lld", lFriendID);

	bool bExist = m_pCommonElementsBucket->m_pVideoCallSessionList->IsVideoSessionExist(lFriendID, pVideoSession);

	CLogPrinter_LOG(API_FLOW_CHECK_LOG, "CController::InterruptOccured size of ID %lld list size %d bExist %d", lFriendID, m_pCommonElementsBucket->m_pVideoCallSessionList->GetSessionListSize(), bExist);

	if (bExist)
	{
		CLogPrinter_LOG(API_FLOW_CHECK_LOG, "CController::InterruptOccured called 3 ID %lld", lFriendID);

		if (pVideoSession)
		{
			CLogPrinter_LOG(API_FLOW_CHECK_LOG, "CController::InterruptOccured called 4 ID %lld", lFriendID);

			pVideoSession->InterruptOccured();

			CLogPrinter_LOG(API_FLOW_CHECK_LOG, "CController::InterruptOccured called 5 ID %lld", lFriendID);
		}
	}

	CLogPrinter_LOG(API_FLOW_CHECK_LOG, "CController::InterruptOccured done ID %lld", lFriendID);
}

void CController::InterruptOver(const long long lFriendID)
{
	CVideoCallSession* pVideoSession;

	CLogPrinter_Write(CLogPrinter::DEBUGS, "CController::InterruptOver called");

	CLogPrinter_LOG(API_FLOW_CHECK_LOG, "CController::InterruptOver called 1 ID %lld", lFriendID);

	InterruptOverLocker lock(*m_pVideoSendMutex);

	CLogPrinter_LOG(API_FLOW_CHECK_LOG, "CController::InterruptOver called 2 ID %lld", lFriendID);

	bool bExist = m_pCommonElementsBucket->m_pVideoCallSessionList->IsVideoSessionExist(lFriendID, pVideoSession);

	CLogPrinter_LOG(API_FLOW_CHECK_LOG, "CController::InterruptOver size of ID %lld list size %d bExist %d", lFriendID, m_pCommonElementsBucket->m_pVideoCallSessionList->GetSessionListSize(), bExist);

	if (bExist)
	{
		CLogPrinter_Write(CLogPrinter::INFO, "CController::InterruptOver got session");

		CLogPrinter_LOG(API_FLOW_CHECK_LOG, "CController::InterruptOver called 3 ID %lld", lFriendID);

		if (pVideoSession)
		{
			CLogPrinter_LOG(API_FLOW_CHECK_LOG, "CController::InterruptOver called 4 ID %lld", lFriendID);

			pVideoSession->InterruptOver();

			CLogPrinter_LOG(API_FLOW_CHECK_LOG, "CController::InterruptOver called 5ID %lld", lFriendID);
		}
	}

	CLogPrinter_LOG(API_FLOW_CHECK_LOG, "CController::InterruptOver done ID %lld", lFriendID);
}

void CController::initializeEventHandler()
{
	m_pCommonElementsBucket->m_pEventNotifier = &m_EventNotifier;

	CLogPrinter_Write(CLogPrinter::DEBUGS, "CController::initializeEventHandler() EventHandler Initialized");
}

bool CController::StopAudioCall(const long long& lFriendID)
{
	COW("###^^^### STOP AUDIO CALL CALLING...........");
    CLogPrinter_Write(CLogPrinter::ERRORS, "CController::StopAudioCall() called.");
    
	StopAudioCallLocker lock3(*m_pAudioLockMutex);

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

bool CController::StopTestAudioCall(const long long& lFriendID)
{
	CLogPrinter_WriteLog(CLogPrinter::INFO, CHECK_CAPABILITY_LOG, "CController::StopTestAudioCall() called");

	StopTestAudioCallLocker lock3(*m_pAudioLockMutex);

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

bool CController::StopTestVideoCall(const long long& lFriendID)
{
	CLogPrinter_WriteLog(CLogPrinter::INFO, CHECK_CAPABILITY_LOG, "CController::StopTestVideoCall() called -> friendID = " + m_Tools.getText(lFriendID));

	StopTestCallLocker lock1(*m_pVideoSendMutex);
	StopTestCallLocker lock2(*m_pVideoReceiveMutex);

	CLogPrinter_WriteLog(CLogPrinter::INFO, CHECK_CAPABILITY_LOG, "CController::StopTestVideoCall() checking session");

	CVideoCallSession *m_pSession;

	m_pSession = m_pCommonElementsBucket->m_pVideoCallSessionList->GetFromVideoSessionList(lFriendID);

	if (NULL == m_pSession)
	{
		CLogPrinter_WriteLog(CLogPrinter::INFO, CHECK_CAPABILITY_LOG, "CController::StopTestVideoCall() session doesn't exists");

		return false;
	}

	bool bReturnedValue = m_pCommonElementsBucket->m_pVideoCallSessionList->RemoveFromVideoSessionList(lFriendID);

	CLogPrinter_WriteLog(CLogPrinter::INFO, CHECK_CAPABILITY_LOG, "CController::StopTestVideoCall() ended with bReturnedValue " + m_Tools.IntegertoStringConvert(bReturnedValue));

    m_bDeviceCapabilityRunning = false;
    
	return bReturnedValue;
}

bool CController::StopVideoCall(const long long& lFriendID)
{
    CLogPrinter_Write(CLogPrinter::ERRORS, "CController::StopVideoCall() called. --> friendID = " + m_Tools.getText(lFriendID));
//    CLogPrinter_WriteLog(CLogPrinter::INFO, INSTENT_TEST_LOG, "StopVideo call operation started");
    
	CLogPrinter_LOG(API_FLOW_CHECK_LOG, "CController::StopVideoCall called 1 ID %lld", lFriendID);
    
	StopCallLocker lock1(*m_pVideoSendMutex);
//    CLogPrinter_WriteLog(CLogPrinter::INFO, INSTENT_TEST_LOG, "StopVideo call After first lock");
	StopCallLocker lock2(*m_pVideoReceiveMutex);
//    CLogPrinter_WriteLog(CLogPrinter::INFO, INSTENT_TEST_LOG, "StopVideo call After Second lock");
    
    CVideoCallSession *m_pSession;
    
	CLogPrinter_LOG(API_FLOW_CHECK_LOG, "CController::StopVideoCall called 2 ID %lld", lFriendID);
    
    m_pSession = m_pCommonElementsBucket->m_pVideoCallSessionList->GetFromVideoSessionList(lFriendID);

	CLogPrinter_LOG(API_FLOW_CHECK_LOG, "CController::StopVideoCall session got %d, size of list %d", m_pSession, m_pCommonElementsBucket->m_pVideoCallSessionList->GetSessionListSize());

    if (NULL == m_pSession)
    {
        CLogPrinter_Write(CLogPrinter::ERRORS, "CController::StopVideoCall() Session Does not Exist.");
        return false;
    }

	CLogPrinter_LOG(API_FLOW_CHECK_LOG, "CController::StopVideoCall removing session ID %lld", lFriendID);

    bool bReturnedValue = m_pCommonElementsBucket->m_pVideoCallSessionList->RemoveFromVideoSessionList(lFriendID);

    CLogPrinter_Write(CLogPrinter::ERRORS, "CController::StopVideoall() ended " + m_Tools.IntegertoStringConvert(bReturnedValue));

    
    m_bLiveCallRunning = false;

	CLogPrinter_LOG(API_FLOW_CHECK_LOG, "CController::StopVideoCall session removed ID %lld", lFriendID);
	CLogPrinter_LOG(API_FLOW_CHECK_LOG, "CController::StopVideoCall done ID %lld", lFriendID);
    
    return bReturnedValue;
}

int CController::StartAudioEncodeDecodeSession()
{
	ControllerLockerStart lock(*m_pAudioLockMutex);

	if (NULL == m_pAudioEncodeDecodeSession)
		m_pAudioEncodeDecodeSession = new CAudioFileEncodeDecodeSession();

	return m_pAudioEncodeDecodeSession->StartAudioEncodeDecodeSession();
}

int CController::EncodeAudioFrame(short *psaEncodingDataBuffer, int nAudioFrameSize, unsigned char *ucaEncodedDataBuffer)
{
	ControllerLockerStart lock(*m_pAudioLockMutex);

	if (NULL == m_pAudioEncodeDecodeSession)
	{
		return 0;
	}
	else
		return m_pAudioEncodeDecodeSession->EncodeAudioFile(psaEncodingDataBuffer, nAudioFrameSize, ucaEncodedDataBuffer);
}

int CController::DecodeAudioFrame(unsigned char *ucaDecodedDataBuffer, int nAudioFrameSize, short *psaDecodingDataBuffer)
{
	EncodeAudioFrameLocker lock(*m_pAudioLockMutex);

	if (NULL == m_pAudioEncodeDecodeSession)
	{
		return 0;
	}
	else
		return m_pAudioEncodeDecodeSession->DecodeAudioFile(ucaDecodedDataBuffer, nAudioFrameSize, psaDecodingDataBuffer);
}

int CController::StopAudioEncodeDecodeSession()
{
	DecodeAudioFrameLocker lock(*m_pAudioLockMutex);
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
	LOGENEW("fahad -->> CController::StartVideoMuxingAndEncodeSession 0");
	if (NULL == m_pVideoMuxingAndEncodeSession)
	{
		LOGENEW("fahad -->> CController::StartVideoMuxingAndEncodeSession null");
		m_pVideoMuxingAndEncodeSession = new CVideoMuxingAndEncodeSession(m_pCommonElementsBucket);
	}

	return m_pVideoMuxingAndEncodeSession->StartVideoMuxingAndEncodeSession(pBMP32Data, iLen, nVideoHeight, nVideoWidth);
}

int CController::FrameMuxAndEncode( unsigned char *pVideoYuv, int iHeight, int iWidth)
{
	if (NULL == m_pVideoMuxingAndEncodeSession)
	{
		return 0;
	}
	else
		return m_pVideoMuxingAndEncodeSession->FrameMuxAndEncode(pVideoYuv, iHeight, iWidth);
}

int CController::StopVideoMuxingAndEncodeSession(unsigned char *finalData)
{
	if (NULL != m_pVideoMuxingAndEncodeSession)
	{
		int ret = m_pVideoMuxingAndEncodeSession->StopVideoMuxingAndEncodeSession(finalData);

		delete m_pVideoMuxingAndEncodeSession;

		m_pVideoMuxingAndEncodeSession = NULL;

		return ret;
	}
	else
		return 0;
}


void CController::UninitializeLibrary()
{
	CLogPrinter_Write(CLogPrinter::INFO, "CController::UninitializeLibrary() for all friend and all media");
	CLogPrinter_LOG(API_FLOW_CHECK_LOG, "CController::UninitializeLibrary called 1");
	COW("###^^^### UNINT LIBRARY ... ");
	if (NULL != m_pDeviceCapabilityCheckThread)
	{
		CLogPrinter_LOG(API_FLOW_CHECK_LOG, "CController::UninitializeLibrary called 2");

		m_pDeviceCapabilityCheckThread->StopDeviceCapabilityCheckThread();
	}

	UninitLibLocker lock1(*m_pVideoSendMutex);
	UninitLibLocker lock2(*m_pVideoReceiveMutex);
	UninitLibLocker lock3(*m_pAudioLockMutex);
	UninitLibLocker lock4(*m_pAudioSendMutex);
	UninitLibLocker lock5(*m_pAudioReceiveMutex);

	CLogPrinter_LOG(API_FLOW_CHECK_LOG, "CController::UninitializeLibrary remoging sessions");

	CLogPrinter_LOG(API_FLOW_CHECK_LOG, "CController::UninitializeLibrary number of sessions %d", m_pCommonElementsBucket->m_pVideoCallSessionList->GetSessionListSize());

	m_pCommonElementsBucket->m_pVideoCallSessionList->ClearAllFromVideoSessionList();
	m_pCommonElementsBucket->m_pVideoEncoderList->ClearAllFromVideoEncoderList();

	m_pCommonElementsBucket->m_pAudioCallSessionList->ClearAllFromAudioSessionList();

	CLogPrinter_LOG(API_FLOW_CHECK_LOG, "CController::UninitializeLibrary sessions removed");
	CLogPrinter_LOG(API_FLOW_CHECK_LOG, "CController::UninitializeLibrary done");
}

void CController::SetNotifyClientWithPacketCallback(void(*callBackFunctionPointer)(long long, unsigned char*, int))
{
    m_EventNotifier.SetNotifyClientWithPacketCallback(callBackFunctionPointer);
}


void CController::SetNotifyClientWithVideoDataCallback(void(*callBackFunctionPointer)(long long, int, unsigned char*, int, int, int, int, int, int))
{
	m_EventNotifier.SetNotifyClientWithVideoDataCallback(callBackFunctionPointer);
}


void CController::SetNotifyClientWithVideoNotificationCallback(void(*callBackFunctionPointer)(long long, int))
{
	m_EventNotifier.SetNotifyClientWithVideoNotificationCallback(callBackFunctionPointer);
}

void CController::SetNotifyClientWithNetworkStrengthNotificationCallback(void(*callBackFunctionPointer)(long long, int))
{
	m_EventNotifier.SetNotifyClientWithNetworkStrengthNotificationCallback(callBackFunctionPointer);
}

void CController::SetNotifyClientWithAudioDataCallback(void(*callBackFunctionPointer)(long long, int, short*, int))
{
    m_EventNotifier.SetNotifyClientWithAudioDataCallback(callBackFunctionPointer);
}

void CController::SetNotifyClientWithAudioPacketDataCallback(void(*callBackFunctionPointer)(long long, unsigned char*, int))
{
	m_EventNotifier.SetNotifyClientWithAudioPacketDataCallback(callBackFunctionPointer);
}

void CController::SetNotifyClientWithAudioAlarmCallback(void(*callBackFunctionPointer)(long long, short*, int))
{
	m_EventNotifier.SetNotifyClientWithAudioAlarmCallback(callBackFunctionPointer);
}

void CController::SetSendFunctionPointer(SendFunctionPointerType callBackFunctionPointer)
{
    m_pCommonElementsBucket->SetSendFunctionPointer(callBackFunctionPointer);
}

bool CController::StartAudioCallInLive(const long long& lFriendID, int iRole, int nCallInLiveType)
{
	StartAudioCallInLiveLocker lock(*m_pAudioLockMutex);

	CAudioCallSession* pAudioSession;

	bool bExist = m_pCommonElementsBucket->m_pAudioCallSessionList->IsAudioSessionExist(lFriendID, pAudioSession);
	if (bExist)
	{
		pAudioSession->StartCallInLive(iRole, nCallInLiveType);
		return true;
	}
	else
	{
		return false;
	}
}

bool CController::EndAudioCallInLive(const long long& lFriendID)
{
	EndAudioCallInLiveLocker lock(*m_pAudioLockMutex);
	CAudioCallSession* pAudioSession;

	bool bExist = m_pCommonElementsBucket->m_pAudioCallSessionList->IsAudioSessionExist(lFriendID, pAudioSession);
	if (bExist)
	{
		pAudioSession->EndCallInLive();
		return true;
	}
	else
	{
		return false;
	}
}

bool CController::StartVideoCallInLive(const long long& lFriendID, int nCallInLiveType, int nCalleeID)
{
	CVideoCallSession* pVideoSession;

	StartCallInLiveLocker lock(*m_pVideoSendMutex);
	StartCallInLiveLocker lock2(*m_pVideoReceiveMutex);

	bool bExist = m_pCommonElementsBucket->m_pVideoCallSessionList->IsVideoSessionExist(lFriendID, pVideoSession);
	
	if (bExist)
	{
		pVideoSession->StartCallInLive(nCallInLiveType, nCalleeID);

		return true;
	}
	else
	{
		return false;
	}
}

bool CController::EndVideoCallInLive(const long long& lFriendID, int nCalleeID)
{
	CVideoCallSession* pVideoSession;

	EndCallInLiveLocker lock(*m_pVideoSendMutex);
	EndCallInLiveLocker lock2(*m_pVideoReceiveMutex);

	bool bExist = m_pCommonElementsBucket->m_pVideoCallSessionList->IsVideoSessionExist(lFriendID, pVideoSession);

	if (bExist)
	{
		pVideoSession->EndCallInLive(nCalleeID);

		return true;
	}
	else
	{
		return false;
	}
}


bool CController::IsCallInLiveEnabled()
{
	return this->m_bCallInLiveEnabled;
}

void CController::SetCallInLiveEnabled(bool value)
{
	this->m_bCallInLiveEnabled = value;
}

int CController::GetDeviceDisplayHeight()
{
	return m_nDeviceDisplayHeight;
}

int CController::GetDeviceDisplayWidth()
{
	return m_nDeviceDisplayWidth;
}
    
void CController::TraverseReceivedVideoData(int offset, unsigned char *in_data, unsigned int in_size, int numberOfFrames, int *frameSizes, std::vector< std::pair<int, int> > vMissingFrames)
{
    
    unsigned char ucTmp;
    int iTmp;
    int commonLeft;
    int commonRight;
    
    for(int i=0;i<in_size;i++)
    {
        ucTmp = in_data[i];
    }
    
    for(int i=0;i<numberOfFrames;i++)
    {
        iTmp = frameSizes[i];
    }
    
    int numberOfMissingFrame = (int)vMissingFrames.size();
    
    for(int i=0;i<numberOfMissingFrame;i++)
    {
        commonLeft = vMissingFrames[i].first;
        commonRight = vMissingFrames[i].second;
    }
}

} //namespace MediaSDK
