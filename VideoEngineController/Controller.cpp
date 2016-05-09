#include "Controller.h"
#include "LogPrinter.h"
#include "Tools.h"
#include "VideoCallSession.h"
#include "VideoEncoder.h"
#include "VideoDecoder.h"

CController::CController():

m_nDeviceStrongness(STATUS_UNCHECKED),
m_nMemoryEnoughness(STATUS_UNCHECKED),
m_nEDVideoSupportablity(STATUS_UNCHECKED),
m_nHighFPSVideoSupportablity(STATUS_UNCHECKED),
m_nDeviceSupportedCallFPS(LOW_FRAME_RATE),
m_pAudioEncodeDecodeSession(NULL),
m_nSupportedResolutionFPSLevel(RESOLUTION_FPS_SUPPORT_NOT_TESTED)

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
m_nSupportedResolutionFPSLevel(RESOLUTION_FPS_SUPPORT_NOT_TESTED)

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

bool CController::StartAudioCall(const LongLong& lFriendID)
{
	CAudioCallSession* pAudioSession;
    
    //Locker lock1(*m_pAudioSendMutex);
    //Locker lock2(*m_pAudioReceiveMutex);

	bool bExist = m_pCommonElementsBucket->m_pAudioCallSessionList->IsAudioSessionExist(lFriendID, pAudioSession);

	CLogPrinter_Write(CLogPrinter::INFO, "CController::StartAudioCall");

	if (!bExist)
	{
		CLogPrinter_Write(CLogPrinter::INFO, "CController::StartAudioCall Session empty");

		pAudioSession = new CAudioCallSession(lFriendID, m_pCommonElementsBucket);

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

bool CController::StartTestAudioCall(const LongLong& lFriendID)
{
	CAudioCallSession* pAudioSession;

	bool bExist = m_pCommonElementsBucket->m_pAudioCallSessionList->IsAudioSessionExist(lFriendID, pAudioSession);

	CLogPrinter_Write(CLogPrinter::INFO, "CController::StartAudioCall");

	if (!bExist)
	{
		CLogPrinter_Write(CLogPrinter::INFO, "CController::StartAudioCall Session empty");

		pAudioSession = new CAudioCallSession(lFriendID, m_pCommonElementsBucket, DEVICE_ABILITY_CHECK_MOOD);

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

bool CController::StartTestVideoCall(const LongLong& lFriendID, int iVideoHeight, int iVideoWidth, int iNetworkType)
{
	CVideoCallSession* pVideoSession;

	CLogPrinter_Write(CLogPrinter::INFO, "CController::StartVideoCall called");

	bool bExist = m_pCommonElementsBucket->m_pVideoCallSessionList->IsVideoSessionExist(lFriendID, pVideoSession);

	if (!bExist)
	{
		CLogPrinter_Write(CLogPrinter::DEBUGS, "CController::StartVideoCall Video Session starting");

		pVideoSession = new CVideoCallSession(lFriendID, m_pCommonElementsBucket, HIGH_FRAME_RATE, &m_nDeviceSupportedCallFPS, DEVICE_ABILITY_CHECK_MOOD, m_pDeviceCapabilityCheckBuffer, m_nSupportedResolutionFPSLevel);

		pVideoSession->InitializeVideoSession(lFriendID, iVideoHeight, iVideoWidth, iNetworkType);

		m_pCommonElementsBucket->m_pVideoCallSessionList->AddToVideoSessionList(lFriendID, pVideoSession);

		CLogPrinter_Write(CLogPrinter::DEBUGS, "CController::StartVideoCall Video Session started");

		return true;
	}
	else
	{
		//pVideoSession->ReInitializeVideoLibrary(iVideoHeight, iVideoWidth);
		return false;
	}
}

bool CController::StartVideoCall(const LongLong& lFriendID, int iVideoHeight, int iVideoWidth, int iNetworkType)
{
	CVideoCallSession* pVideoSession;

	CLogPrinter_Write(CLogPrinter::INFO, "CController::StartVideoCall called");
    
    //Locker lock1(*m_pVideoSendMutex);
    //Locker lock2(*m_pVideoReceiveMutex);

	bool bExist = m_pCommonElementsBucket->m_pVideoCallSessionList->IsVideoSessionExist(lFriendID, pVideoSession);

	if (!bExist)
	{
		CLogPrinter_Write(CLogPrinter::DEBUGS, "CController::StartVideoCall Video Session starting");

		pVideoSession = new CVideoCallSession(lFriendID, m_pCommonElementsBucket, m_nDeviceSupportedCallFPS, &m_nDeviceSupportedCallFPS, LIVE_CALL_MOOD, NULL, m_nSupportedResolutionFPSLevel);

		pVideoSession->InitializeVideoSession(lFriendID, iVideoHeight, iVideoWidth,iNetworkType);

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

int CController::PushPacketForDecoding(const LongLong& lFriendID, unsigned char *in_data, unsigned int in_size)
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
			return pVideoSession->PushPacketForMerging(++in_data, in_size-1,false);
		else
			return -1;
	}
	else
	{
		return -1;
	}
}

int CController::PushAudioForDecoding(const LongLong& lFriendID, unsigned char *in_data, unsigned int in_size)
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
        
		CAudioDecoder *pCAudioDecoder = pAudioSession->GetAudioDecoder();

		//if (pCAudioDecoder)
        {
            CLogPrinter_Write(CLogPrinter::DEBUGS, "pCAudioDecoder exists");
            return pAudioSession->DecodeAudioData(in_data, in_size);
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

int CController::SendAudioData(const LongLong& lFriendID, short *in_data, unsigned int in_size)
{
	CAudioCallSession* pAudioSession;

	CLogPrinter_Write(CLogPrinter::INFO, "CController::SendAudioData");
    
    Locker lock(*m_pAudioSendMutex);

	bool bExist = m_pCommonElementsBucket->m_pAudioCallSessionList->IsAudioSessionExist(lFriendID, pAudioSession);

	CLogPrinter_Write(CLogPrinter::INFO, "CController::SendAudioData audio session exists");

	if (bExist)
	{
		CLogPrinter_Write(CLogPrinter::INFO, "CController::SendAudioData getting encoder");
		CAudioEncoder *pAudioEncoder = pAudioSession->GetAudioEncoder();
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

int CController::SetHeightWidth(const LongLong& lFriendID, int width, int height)
{
/*	CVideoCallSession* pVideoSession;

	bool bExist = m_pCommonElementsBucket->m_pVideoCallSessionList->IsVideoSessionExist(lFriendID, pVideoSession);

	if (bExist)
	{
		CVideoDecoder *pVideoDecoder = pVideoSession->GetVideoDecoder();

		if (pVideoDecoder)
			return pVideoDecoder->Decode(in_data, in_size, out_buffer, width, height);
		else return -1;
	}
	else
	{*/
		return -1;
//	}
}

int CController::SetBitRate(const LongLong& lFriendID, int bitRate)
{
	return -1;
}

int CController::CheckDeviceCapability(const LongLong& lFriendID, int iHeight, int iWidth)
{
	if (m_pDeviceCapabilityCheckBuffer->GetQueueSize() == 0)
		m_pDeviceCapabilityCheckThread->StartDeviceCapabilityCheckThread();

	m_pDeviceCapabilityCheckBuffer->Queue(lFriendID, START_DEVICE_CHECK, DEVICE_CHECK_STARTING, iHeight, iWidth);
    
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

bool CController::StopTestVideoCall(const LongLong& lFriendID)
{
	CLogPrinter_Write(CLogPrinter::ERRORS, "CController::StopVideoCall() called.");
//	CLogPrinter_WriteLog(CLogPrinter::INFO, INSTENT_TEST_LOG, "StopVideo call operation started");

	Locker lock1(*m_pVideoSendMutex);
//	CLogPrinter_WriteLog(CLogPrinter::INFO, INSTENT_TEST_LOG, "StopVideo call After first lock");
	Locker lock2(*m_pVideoReceiveMutex);
//	CLogPrinter_WriteLog(CLogPrinter::INFO, INSTENT_TEST_LOG, "StopVideo call After Second lock");

	CVideoCallSession *m_pSession;

	m_pSession = m_pCommonElementsBucket->m_pVideoCallSessionList->GetFromVideoSessionList(lFriendID);

	if (NULL == m_pSession)
	{
		CLogPrinter_Write(CLogPrinter::ERRORS, "CController::StopVideoCall() Session Does not Exist.");
		return false;
	}

	bool bReturnedValue = m_pCommonElementsBucket->m_pVideoCallSessionList->RemoveFromVideoSessionList(lFriendID);

	CLogPrinter_Write(CLogPrinter::ERRORS, "CController::StopVideoall() ended " + m_Tools.IntegertoStringConvert(bReturnedValue));

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


void CController::UninitializeLibrary()
{
	CLogPrinter_Write(CLogPrinter::INFO, "CController::UninitializeLibrary() for all friend and all media");

	m_pCommonElementsBucket->m_pVideoCallSessionList->ClearAllFromVideoSessionList();
	m_pCommonElementsBucket->m_pVideoEncoderList->ClearAllFromVideoEncoderList();
}

void CController::SetNotifyClientWithPacketCallback(void(*callBackFunctionPointer)(LongLong, unsigned char*, int))
{
    m_EventNotifier.SetNotifyClientWithPacketCallback(callBackFunctionPointer);
}

void CController::SetNotifyClientWithVideoDataCallback(void(*callBackFunctionPointer)(LongLong, unsigned char*, int, int, int, int))
{
	m_EventNotifier.SetNotifyClientWithVideoDataCallback(callBackFunctionPointer);
}

void CController::SetNotifyClientWithVideoNotificationCallback(void(*callBackFunctionPointer)(LongLong, int))
{
	m_EventNotifier.SetNotifyClientWithVideoNotificationCallback(callBackFunctionPointer);
}

void CController::SetNotifyClientWithAudioDataCallback(void(*callBackFunctionPointer)(LongLong, short*, int))
{
    m_EventNotifier.SetNotifyClientWithAudioDataCallback(callBackFunctionPointer);
}

void CController::SetNotifyClientWithAudioPacketDataCallback(void(*callBackFunctionPointer)(IPVLongType, unsigned char*, int))
{
    m_EventNotifier.SetNotifyClientWithAudioPacketDataCallback(callBackFunctionPointer);
}

void CController::SetSendFunctionPointer(void(*callBackFunctionPointer)(IPVLongType, int, unsigned char*, int))
{
    m_pCommonElementsBucket->SetSendFunctionPointer(callBackFunctionPointer);
}

