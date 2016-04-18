
#include "VideoCallSession.h"
#include "CommonElementsBucket.h"
#include "LogPrinter.h"
#include "Globals.h"

#if defined(TARGET_OS_IPHONE) || defined(TARGET_IPHONE_SIMULATOR)
#include <dispatch/dispatch.h>
#endif

extern CFPSController g_FPSController;

extern bool g_bIsVersionDetectableOpponent;
extern unsigned char g_uchSendPacketVersion;
extern long long g_llFirstFrameReceiveTime;
CVideoCallSession::CVideoCallSession(LongLong fname, CCommonElementsBucket* sharedObject) :

m_pCommonElementsBucket(sharedObject),
m_ClientFPS(FPS_BEGINNING),
m_ClientFPSDiffSum(0),
m_ClientFrameCounter(0),
m_EncodingFrameCounter(0),
m_pEncodedFramePacketizer(NULL),
m_ByteRcvInBandSlot(0),
m_SlotResetLeftRange(0),
m_SlotResetRightRange(FRAME_RATE),
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
m_bReinitialized(false)
{
	m_miniPacketBandCounter = 0;

#ifdef FIRST_BUILD_COMPATIBLE
	g_bIsVersionDetectableOpponent = false;
	g_uchSendPacketVersion = 0;
#else
	g_bIsVersionDetectableOpponent = true;
	g_uchSendPacketVersion = 1;
#endif

	//Resetting Global Variables.

	g_FPSController.Reset();

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

	m_BitRateController = new BitRateController();

	m_BitRateController->SetSharedObject(sharedObject);

	CLogPrinter_Write(CLogPrinter::DEBUGS, "CVideoCallSession::CVideoCallSession created");
}

CVideoCallSession::~CVideoCallSession()
{
    CLogPrinter_WriteLog(CLogPrinter::INFO, INSTENT_TEST_LOG, "Video call session destructor 1");
	m_pVideoEncodingThread->StopEncodingThread();
    CLogPrinter_WriteLog(CLogPrinter::INFO, INSTENT_TEST_LOG, "Video call session destructor 2");
	m_pSendingThread->StopSendingThread();
    CLogPrinter_WriteLog(CLogPrinter::INFO, INSTENT_TEST_LOG, "Video call session destructor 3");
	m_pVideoDepacketizationThread->StopDepacketizationThread();
    CLogPrinter_WriteLog(CLogPrinter::INFO, INSTENT_TEST_LOG, "Video call session destructor 4");
	m_pVideoDecodingThread->StopDecodingThread();
    CLogPrinter_WriteLog(CLogPrinter::INFO, INSTENT_TEST_LOG, "Video call session destructor 5");
	m_pVideoRenderingThread->StopRenderingThread();
    CLogPrinter_WriteLog(CLogPrinter::INFO, INSTENT_TEST_LOG, "Video call session destructor 6");

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

	m_lfriendID = -1;

	SHARED_PTR_DELETE(m_pVideoCallSessionMutex);
}

LongLong CVideoCallSession::GetFriendID()
{
	return m_lfriendID;
}

void CVideoCallSession::InitializeVideoSession(LongLong lFriendID, int iVideoHeight, int iVideoWidth, int iNetworkType)
{
    g_llFirstFrameReceiveTime = 0;
	CLogPrinter_Write(CLogPrinter::INFO, "CVideoCallSession::InitializeVideoSession");

	if (sessionMediaList.IsVideoEncoderExist(iVideoHeight, iVideoWidth))
	{
		return;
	}

	CLogPrinter_Write(CLogPrinter::INFO, "CVideoCallSession::InitializeVideoSession 2");

	this->m_pVideoEncoder = new CVideoEncoder(m_pCommonElementsBucket);

	m_pVideoEncoder->CreateVideoEncoder(iVideoHeight, iVideoWidth);

	g_FPSController.SetEncoder(m_pVideoEncoder);
	m_BitRateController->SetEncoder(m_pVideoEncoder);

	this->m_pVideoDecoder = new CVideoDecoder(m_pCommonElementsBucket);

	m_pVideoDecoder->CreateVideoDecoder();

	this->m_pColorConverter = new CColorConverter(iVideoHeight, iVideoWidth);

	m_pSendingThread = new CSendingThread(m_pCommonElementsBucket, m_SendingBuffer, &g_FPSController, this);
	m_pVideoEncodingThread = new CVideoEncodingThread(lFriendID, m_EncodingBuffer, m_BitRateController, m_pColorConverter, m_pVideoEncoder, m_pEncodedFramePacketizer, this);
	m_pVideoRenderingThread = new CVideoRenderingThread(lFriendID, m_RenderingBuffer, m_pCommonElementsBucket, this);
	m_pVideoDecodingThread = new CVideoDecodingThread(m_pEncodedFrameDepacketizer, m_RenderingBuffer, m_pVideoDecoder, m_pColorConverter, &g_FPSController, this);
	m_pVideoDepacketizationThread = new CVideoDepacketizationThread(lFriendID, m_pVideoPacketQueue, m_pRetransVideoPacketQueue, m_pMiniPacketQueue, m_BitRateController, m_pEncodedFrameDepacketizer, m_pCommonElementsBucket, &m_miniPacketBandCounter);

	m_pCommonElementsBucket->m_pVideoEncoderList->AddToVideoEncoderList(lFriendID, m_pVideoEncoder);

	m_ClientFrameCounter = 0;
	m_EncodingFrameCounter = 0;
	m_llFirstFrameCapturingTimeStamp = -1;
	m_pSendingThread->StartSendingThread();
	
	m_pVideoEncodingThread->StartEncodingThread();
	
	m_pVideoRenderingThread->StartRenderingThread();	
	m_pVideoDepacketizationThread->StartDepacketizationThread();
	m_pVideoDecodingThread->StartDecodingThread();

	m_BitRateController->SetOwnNetworkType(iNetworkType);
	CreateAndSendMiniPacket(iNetworkType, NETWORK_TYPE_MINIPACKET);

	CLogPrinter_Write(CLogPrinter::INFO, "CVideoCallSession::InitializeVideoSession session initialized");
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

void CVideoCallSession::SetFirstFrameEncodingTime(int time){
	m_nFirstFrameEncodingTimeDiff = time;
}

int CVideoCallSession::GetFirstFrameEncodingTime(){
	return m_nFirstFrameEncodingTimeDiff;
}

bool CVideoCallSession::PushPacketForMerging(unsigned char *in_data, unsigned int in_size)
{
    

#ifdef FIRST_BUILD_COMPATIBLE
	if (!g_bIsVersionDetectableOpponent && (in_data[SIGNAL_BYTE_INDEX_WITHOUT_MEDIA] & 0xC0) == 0xC0)
	{
		g_bIsVersionDetectableOpponent = true;
		g_uchSendPacketVersion = VIDEO_VERSION_CODE;
		//CLogPrinter_WriteSpecific(CLogPrinter::INFO, "$$$# ######################################## Version #################################################");		
	}
#endif

	if (((in_data[RETRANSMISSION_SIG_BYTE_INDEX_WITHOUT_MEDIA] >> BIT_INDEX_MINI_PACKET) & 1)) // It is a minipacket
	{
		CLogPrinter_WriteSpecific2(CLogPrinter::INFO, "PKTTYPE --> GOT MINI PACKET");
		m_pMiniPacketQueue->Queue(in_data, in_size);
	}
	else
	{
        OperationForResolutionControl(in_data,in_size);
        
		unsigned int unFrameNumber = m_PacketHeader.GetFrameNumberDirectly(in_data);

		if (unFrameNumber >= m_SlotResetLeftRange && unFrameNumber < m_SlotResetRightRange)
		{
			m_ByteRcvInBandSlot += (in_size - PACKET_HEADER_LENGTH);
		}
		else
		{
			if (m_bSkipFirstByteCalculation == true)
			{
				m_bSkipFirstByteCalculation = false;
			}
			else
			{
				m_miniPacketBandCounter = m_SlotResetLeftRange / FRAME_RATE;
                
                CLogPrinter_WriteLog(CLogPrinter::INFO, INSTENT_TEST_LOG, "ReceivingSide: SlotIndex = " + m_Tools.IntegertoStringConvert(m_miniPacketBandCounter) + ", ReceivedBytes = " + m_Tools.IntegertoStringConvert(m_ByteRcvInBandSlot));

				CreateAndSendMiniPacket(m_ByteRcvInBandSlot, BITRATE_TYPE_MINIPACKET);
			}

			m_SlotResetLeftRange = unFrameNumber - (unFrameNumber % FRAME_RATE);
            
            
			m_SlotResetRightRange = m_SlotResetLeftRange + FRAME_RATE;

			m_ByteRcvInBandSlot = in_size - PACKET_HEADER_LENGTH;
		}

		m_pVideoPacketQueue->Queue(in_data, in_size);
	}

	return true;
}

long long g_EncodingTimeDiff = 0;
int CVideoCallSession::PushIntoBufferForEncoding(unsigned char *in_data, unsigned int in_size)
{
	if (m_pVideoEncodingThread->IsThreadStarted() == false)
		return 1;

    if(g_llFirstFrameReceiveTime == 0) g_llFirstFrameReceiveTime = m_Tools.CurrentTimestamp();
    
	CLogPrinter_Write(CLogPrinter::INFO, "CVideoCallSession::PushIntoBufferForEncoding");

	LongLong currentTimeStamp = m_Tools.CurrentTimestamp();

	if (m_ClientFrameCounter++)
	{
		m_ClientFPSDiffSum += currentTimeStamp - m_LastTimeStampClientFPS;

		{//Block for LOCK
			int  nApproximateAverageFrameInterval = m_ClientFPSDiffSum / m_ClientFrameCounter;
			if(nApproximateAverageFrameInterval > 10) {
				Locker lock(*m_pVideoCallSessionMutex);
				g_FPSController.SetClientFPS(1000 / nApproximateAverageFrameInterval);
			}
		}
	}

	m_LastTimeStampClientFPS = currentTimeStamp;

#if defined(TARGET_OS_IPHONE) || defined(TARGET_IPHONE_SIMULATOR)

	CLogPrinter_Write(CLogPrinter::INFO, "CVideoCallSession::PushIntoBufferForEncoding 2");
	//this->m_pColorConverter->ConvertNV12ToI420(m_EncodingFrame);

	CLogPrinter_Write(CLogPrinter::DEBUGS, "CVideoCallSession::PushIntoBufferForEncoding Converted to 420");

#endif

	if(-1 == m_llFirstFrameCapturingTimeStamp)
		m_llFirstFrameCapturingTimeStamp = currentTimeStamp;

	int nCaptureTimeDiff = currentTimeStamp - m_llFirstFrameCapturingTimeStamp;
    
    
	int returnedValue = m_EncodingBuffer->Queue(in_data, in_size, nCaptureTimeDiff);
    
    //CLogPrinter_WriteLog(CLogPrinter::INFO, OPERATION_TIME_LOG || INSTENT_TEST_LOG, " nCaptureTimeDiff = " +  m_Tools.LongLongtoStringConvert(m_Tools.CurrentTimestamp() - mt_llCapturePrevTime));
    mt_llCapturePrevTime = m_Tools.CurrentTimestamp();
    
    
    
//	CLogPrinter_WriteInstentTestLog(CLogPrinter::INFO, "CVideoCallSession::PushIntoBufferForEncoding Queue packetSize " + Tools::IntegertoStringConvert(in_size));

	CLogPrinter_Write(CLogPrinter::DEBUGS, "CVideoCallSession::PushIntoBufferForEncoding pushed to encoder queue");

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
	unsigned char uchVersion = g_uchSendPacketVersion;
	CPacketHeader PacketHeader;

	if (nMiniPacketType == BITRATE_TYPE_MINIPACKET)
	{
		if(0 == uchVersion) return;

		PacketHeader.setPacketHeader(uchVersion, m_miniPacketBandCounter/*SlotID*/, 0, nMiniPacketType, nByteReceivedOrNetworkType/*Byte Received*/, 0, 0, 0);
	}
	else if (nMiniPacketType == NETWORK_TYPE_MINIPACKET)
	{
		//if(0 == uchVersion) return;

		PacketHeader.setPacketHeader(uchVersion, m_miniPacketBandCounter/*SlotID*/, 0, nMiniPacketType, nByteReceivedOrNetworkType/*Network Type*/, 0, 0, 0);
	}

	m_miniPacket[0] = (int)VIDEO_PACKET_MEDIA_TYPE;

	PacketHeader.GetHeaderInByteArray(m_miniPacket + 1);

	m_miniPacket[RETRANSMISSION_SIG_BYTE_INDEX_WITHOUT_MEDIA + 1] |= 1<<BIT_INDEX_MINI_PACKET; 

	if(uchVersion)
		m_pCommonElementsBucket->SendFunctionPointer(m_lfriendID, 2, m_miniPacket,PACKET_HEADER_LENGTH + 1);
	else
		m_pCommonElementsBucket->SendFunctionPointer(m_lfriendID, 2, m_miniPacket,PACKET_HEADER_LENGTH_NO_VERSION + 1);
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
        //Eikhan thekee amra HighResolated video support diyee dibo
        CLogPrinter_WriteLog(CLogPrinter::INFO, INSTENT_TEST_LOG, "Decision Supported = " + m_Tools.IntegertoStringConvert(bValue));
        
    }
    else
    {
        //Not Supported
        m_bHighResolutionSupportedForOwn = false;
        
        m_pCommonElementsBucket->m_pEventNotifier->fireVideoNotificationEvent(m_lfriendID, m_pCommonElementsBucket->m_pEventNotifier->SET_CAMERA_RESOLUTION_352x288_OR_320x240);
        ReInitializeVideoLibrary(352, 288);
        CLogPrinter_WriteLog(CLogPrinter::INFO, INSTENT_TEST_LOG, "Decision NotSupported = " + m_Tools.IntegertoStringConvert(bValue));
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
    CPacketHeader RcvPakcetHeader;
    int gotResSt = RcvPakcetHeader.GetOpponentResolution(in_data);
    
    if(gotResSt == 2)
    {
        //CLogPrinter_WriteLog(CLogPrinter::INFO, INSTENT_TEST_LOG, "Opponent High supported, flag =  "+ m_Tools.IntegertoStringConvert(gotResSt) );
        
        SetOpponentHighResolutionSupportStatus(true);
        
    }
    else if(gotResSt == 1)
    {
        //CLogPrinter_WriteLog(CLogPrinter::INFO, INSTENT_TEST_LOG, "Opponent High NotSupported = "+ m_Tools.IntegertoStringConvert(gotResSt));
        SetOpponentHighResolutionSupportStatus(false);
        
        if(m_bResolationCheck == true)
        {
            if(m_bHighResolutionSupportedForOwn == false || m_bHighResolutionSupportedForOpponent == false)
            {
                if(m_bReinitialized == false)
                {
                    printf("m_bReinitialized SET_CAMERA_RESOLUTION_352x288_OR_320x240  = %d\n", m_bReinitialized);
                    m_bReinitialized = true;
                    
                    
                    
                    m_pCommonElementsBucket->m_pEventNotifier->fireVideoNotificationEvent(m_lfriendID, m_pCommonElementsBucket->m_pEventNotifier->SET_CAMERA_RESOLUTION_352x288_OR_320x240);
                    ReInitializeVideoLibrary(352, 288);
                    
                    
                }
                
                
            }
        }
        
    }
    else
    {
        CLogPrinter_WriteLog(CLogPrinter::INFO, INSTENT_TEST_LOG, "Opponent not set = " + m_Tools.IntegertoStringConvert(gotResSt));
    }
    
    
    //End of  Opponent resolution support checking
}
void CVideoCallSession::ReInitializeVideoLibrary(int iHeight, int iWidth)
{
    printf("Reinitializing........\n");
    long long llReinitializationStartTime = m_Tools.CurrentTimestamp();
    
    m_bReinitialized = true;
    
    CLogPrinter_WriteLog(CLogPrinter::INFO, INSTENT_TEST_LOG, "Video call session destructor 1");
    m_pVideoEncodingThread->StopEncodingThread();
    CLogPrinter_WriteLog(CLogPrinter::INFO, INSTENT_TEST_LOG, "Video call session destructor 2");
    m_pVideoDepacketizationThread->StopDepacketizationThread();
    CLogPrinter_WriteLog(CLogPrinter::INFO, INSTENT_TEST_LOG, "Video call session destructor 4");
    //m_pVideoDecodingThread->StopDecodingThread();
    CLogPrinter_WriteLog(CLogPrinter::INFO, INSTENT_TEST_LOG, "Video call session destructor 5");
    m_pVideoRenderingThread->StopRenderingThread();
    CLogPrinter_WriteLog(CLogPrinter::INFO, INSTENT_TEST_LOG, "Video call session destructor 6");
    

    m_pVideoEncoder->CreateVideoEncoder(iHeight , iWidth);
	m_pColorConverter->SetHeightWidth(iHeight, iWidth);

	m_SendingBuffer->ResetBuffer();
	g_FPSController.Reset();
	m_EncodingBuffer->ResetBuffer();
	//m_BitRateController
	m_RenderingBuffer->ResetBuffer();
	m_pVideoPacketQueue->ResetBuffer();
	m_pMiniPacketQueue->ResetBuffer();

	m_BitRateController = new BitRateController();
    m_BitRateController->SetEncoder(m_pVideoEncoder);
    m_BitRateController->SetSharedObject(m_pCommonElementsBucket);
    
    m_pVideoEncodingThread->ResetVideoEncodingThread(m_BitRateController);
    
//  m_pSendingThread = new CSendingThread(m_pCommonElementsBucket, m_SendingBuffer, &g_FPSController, this);
//	m_pVideoEncodingThread = new CVideoEncodingThread(m_lfriendID, m_EncodingBuffer, m_BitRateController, m_pColorConverter, m_pVideoEncoder, m_pEncodedFramePacketizer, this);
//	m_pVideoRenderingThread = new CVideoRenderingThread(m_lfriendID, m_RenderingBuffer, m_pCommonElementsBucket, this);
//   m_pVideoDecodingThread = new CVideoDecodingThread(m_pEncodedFrameDepacketizer, m_RenderingBuffer, m_pVideoDecoder, m_pColorConverter, &g_FPSController, this);
    
	m_pVideoDepacketizationThread = new CVideoDepacketizationThread(m_lfriendID, m_pVideoPacketQueue, m_pRetransVideoPacketQueue, m_pMiniPacketQueue, m_BitRateController, m_pEncodedFrameDepacketizer, m_pCommonElementsBucket, &m_miniPacketBandCounter);
    

    
    m_pVideoEncodingThread->StartEncodingThread();
    m_pVideoRenderingThread->StartRenderingThread();
    m_pVideoDepacketizationThread->StartDepacketizationThread();
    //m_pVideoDecodingThread->StartDecodingThread();
    
    
    
    printf("TheKing--> Reinitialization time = %lld\n",m_Tools.CurrentTimestamp() - llReinitializationStartTime);
    
    
}

