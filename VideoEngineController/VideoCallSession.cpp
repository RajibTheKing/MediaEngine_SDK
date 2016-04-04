#include "VideoCallSession.h"
#include "CommonElementsBucket.h"
#include "LogPrinter.h"
#include "Tools.h"
#include "Globals.h"
#include "ResendingBuffer.h"
//#include "Helper_IOS.h"

#if defined(TARGET_OS_IPHONE) || defined(TARGET_IPHONE_SIMULATOR)
#include <dispatch/dispatch.h>
#endif
//#include <android/log.h>

//#define LOG_TAG "NewTest"
//#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

//int FPS=0;
//int fpsCnt=0;

deque<pair<int, int>> ExpectedFramePacketDeQueue;
extern long long g_FriendID;
extern CFPSController g_FPSController;


//int countFrame = 0;
//int countFrameFor15 = 0;
//int countFrameSize = 0;
//long long encodeTimeStampFor15;
//int g_iPacketCounterSinceNotifying = FPS_SIGNAL_IDLE_FOR_PACKETS;
//bool gbStopFPSSending = false;

#define ORIENTATION_0_MIRRORED 1
#define ORIENTATION_90_MIRRORED 2
#define ORIENTATION_180_MIRRORED 3
#define ORIENTATION_270_MIRRORED 4
#define ORIENTATION_0_NOT_MIRRORED 5
#define ORIENTATION_90_NOT_MIRRORED 6
#define ORIENTATION_180_NOT_MIRRORED 7
#define ORIENTATION_270_NOT_MIRRORED 8

extern bool g_bIsVersionDetectableOpponent;
extern unsigned char g_uchSendPacketVersion;
extern int g_uchOpponentVersion;
extern CResendingBuffer g_ResendBuffer;

//extern int g_MY_FPS;

CVideoCallSession *g_VideoCallSession;

CVideoCallSession::CVideoCallSession(LongLong fname, CCommonElementsBucket* sharedObject) :

m_pCommonElementsBucket(sharedObject),
m_iFrameNumber(0),
m_ClientFPS(FPS_BEGINNING),
m_ClientFPSDiffSum(0),
m_ClientFrameCounter(0),
m_EncodingFrameCounter(0),
m_ll1stFrameTimeStamp(0),
m_bFirstFrame(true),
m_iTimeStampDiff(0),
m_b1stDecodedFrame(true),
m_ll1stDecodedFrameTimeStamp(0),
m_pEncodedFramePacketizer(NULL),
m_ByteRcvInBandSlot(0),
m_SlotResetLeftRange(GetUniquePacketID(0, 0)),
m_SlotResetRightRange(GetUniquePacketID(FRAME_RATE, 0)),
m_pVideoEncoder(NULL),
m_bSkipFirstByteCalculation(true),
m_bGotOppBandwidth(0),
m_ByteRcvInSlotInverval(0),
m_ByteSendInSlotInverval(0),
m_RecvMegaSlotInvervalCounter(0),
m_SendMegaSlotInervalCounter(0),
m_ByteSendInMegaSlotInverval(0),
m_ByteRecvInMegaSlotInterval(0),
m_SlotIntervalCounter(0),
m_bsetBitrateCalled(false),
m_iConsecutiveGoodMegaSlot(0),
m_iPreviousByterate(BITRATE_MAX / 8),
m_LastSendingSlot(0),
m_iDePacketizeCounter(0),
m_TimeFor100Depacketize(0)
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
	//	countFrame = 0;
	//	countFrameFor15 = 0;
	//	countFrameSize = 0;
	//	encodeTimeStampFor15 = 0;
//	g_iPacketCounterSinceNotifying = FPS_SIGNAL_IDLE_FOR_PACKETS;
	g_ResendBuffer.Reset();
	//gbStopFPSSending = false;

	fpsCnt = 0;
	g_FPSController.Reset();
	//	g_MY_FPS =
	opponentFPS = ownFPS = FPS_BEGINNING;
	m_iCountReQResPack = 0;

	//	FPS=10;

	CLogPrinter_Write(CLogPrinter::INFO, "CVideoCallSession::CVideoCallSession");
	m_pSessionMutex.reset(new CLockHandler);
	friendID = fname;
	sessionMediaList.ClearAllFromVideoEncoderList();


	m_SendingBuffer = new CSendingBuffer();
	m_EncodingBuffer = new CEncodingBuffer();
	m_RenderingBuffer = new CRenderingBuffer();

	m_pVideoPacketQueue = new CVideoPacketQueue();
	m_pRetransVideoPacketQueue = new CVideoPacketQueue();
	m_pMiniPacketQueue = new CVideoPacketQueue();

	m_pEncodedFramePacketizer = new CEncodedFramePacketizer(sharedObject, m_SendingBuffer);
	m_pEncodedFrameDepacketizer = new CEncodedFrameDepacketizer(sharedObject, this);

	m_BitRateController = new BitRateController();

	g_FriendID = fname;

	ExpectedFramePacketPair.first = 0;
	ExpectedFramePacketPair.second = 0;
	iNumberOfPacketsInCurrentFrame = 0;

	g_VideoCallSession = this;

	m_FrameCounterbeforeEncoding = 0;

	m_iGoodSlotCounter = 0;
	m_iNormalSlotCounter = 0;
	m_SlotCounter = 0;

	m_BitRateController->SetSharedObject(sharedObject);

	m_BandWidthRatioHelper.clear();
	CLogPrinter_Write(CLogPrinter::DEBUGS, "CVideoCallSession::CVideoCallSession created");
}

CVideoCallSession::~CVideoCallSession()
{

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

	friendID = -1;

	SHARED_PTR_DELETE(m_pSessionMutex);
}

LongLong CVideoCallSession::GetFriendID()
{
	return friendID;
}

void CVideoCallSession::InitializeVideoSession(LongLong lFriendID, int iVideoHeight, int iVideoWidth, int iNetworkType)
{
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

	this->m_pVideoDecoder = new CVideoDecoder(m_pCommonElementsBucket, &m_DecodingBuffer);

	m_pVideoDecoder->CreateVideoDecoder();

	this->m_pColorConverter = new CColorConverter(iVideoHeight, iVideoWidth);

	m_pSendingThread = new CSendingThread(m_pCommonElementsBucket, m_SendingBuffer, &g_FPSController);
	m_pVideoEncodingThread = new CVideoEncodingThread(lFriendID, m_EncodingBuffer, m_BitRateController, m_pColorConverter, m_pVideoEncoder, m_pEncodedFramePacketizer);
	m_pVideoRenderingThread = new CVideoRenderingThread(lFriendID, m_RenderingBuffer, m_pCommonElementsBucket);
	m_pVideoDecodingThread = new CVideoDecodingThread(m_pEncodedFrameDepacketizer, m_RenderingBuffer, m_pVideoDecoder, m_pColorConverter, &g_FPSController);
	m_pVideoDepacketizationThread = new CVideoDepacketizationThread(lFriendID, m_pVideoPacketQueue, m_pRetransVideoPacketQueue, m_pMiniPacketQueue, m_BitRateController, m_pEncodedFrameDepacketizer, m_pCommonElementsBucket, &m_miniPacketBandCounter);

	m_pCommonElementsBucket->m_pVideoEncoderList->AddToVideoEncoderList(lFriendID, m_pVideoEncoder);

	m_ClientFrameCounter = 0;
	m_EncodingFrameCounter = 0;

	m_pSendingThread->StartSendingThread();
	
	m_pVideoEncodingThread->StartEncodingThread();
	
	m_pVideoRenderingThread->StartRenderingThread();	
	m_pVideoDepacketizationThread->StartDepacketizationThread();
	m_pVideoDecodingThread->StartDecodingThread();

	m_BitRateController->m_iOwnNetworkType = iNetworkType;
	CreateAndSendMiniPacket(iNetworkType, INVALID_PACKET_NUMBER_FOR_NETWORK_TYPE);

	CLogPrinter_Write(CLogPrinter::INFO, "CVideoCallSession::InitializeVideoSession session initialized");
}

CVideoEncoder* CVideoCallSession::GetVideoEncoder()
{
	//	return sessionMediaList.GetFromVideoEncoderList(mediaName);

	return m_pVideoEncoder;
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

#ifdef	RETRANSMISSION_ENABLED
	if (((in_data[RETRANSMISSION_SIG_BYTE_INDEX_WITHOUT_MEDIA] >> BIT_INDEX_RETRANS_PACKET) & 1) /* ||  ((in_data[4] >> 6) & 1) */) //If MiniPacket or RetransMitted packet
	{
		CLogPrinter_WriteSpecific2(CLogPrinter::INFO, "PKTTYPE --> GOT RETRANSMITTED PACKET");
		m_pRetransVideoPacketQueue->Queue(in_data, in_size);
	}
	else if (((in_data[RETRANSMISSION_SIG_BYTE_INDEX_WITHOUT_MEDIA] >> BIT_INDEX_MINI_PACKET) & 1)) // It is a minipacket
	{
		CLogPrinter_WriteSpecific2(CLogPrinter::INFO, "PKTTYPE --> GOT MINI PACKET");
		m_pMiniPacketQueue->Queue(in_data, in_size);
	}
	else
#endif
	{
		CLogPrinter_WriteSpecific2(CLogPrinter::INFO, "PKTTYPE --> GOT Original PACKET");

		/*
		int frameNumber = ((int)in_data[1]<<16) + ((int)in_data[2]<<8) + in_data[3];
		int length ;

		if(g_bIsVersionDetectableOpponent && in_data[VERSION_BYTE_INDEX])
		{
		length = ((int)in_data[12]<<8) + in_data[13];

		}
		else
		{
		length = ((int)in_data[14]<<8) + in_data[15];
		}
		*/




#ifdef BITRATE_CONTROL_BASED_ON_BANDWIDTH
		CPacketHeader NowRecvHeader;
		NowRecvHeader.setPacketHeader(in_data);

		if (GetUniquePacketID(NowRecvHeader.getFrameNumber(), NowRecvHeader.getPacketNumber()) >= m_SlotResetLeftRange
			&& GetUniquePacketID(NowRecvHeader.getFrameNumber(), NowRecvHeader.getPacketNumber()) < m_SlotResetRightRange)
		{
			m_ByteRcvInBandSlot += (NowRecvHeader.getPacketLength() - PACKET_HEADER_LENGTH_WITH_MEDIA_TYPE);
		}
		else if (GetUniquePacketID(NowRecvHeader.getFrameNumber(), NowRecvHeader.getPacketNumber()) >= m_SlotResetRightRange)
		{

			int SlotResetLeftRangeInFrame = (NowRecvHeader.getFrameNumber() - (NowRecvHeader.getFrameNumber() % FRAME_RATE));
			m_SlotResetLeftRange = GetUniquePacketID(SlotResetLeftRangeInFrame, 0);


			int SlotResetRightRangeInFrame = SlotResetLeftRangeInFrame + FRAME_RATE;
			m_SlotResetRightRange = GetUniquePacketID(SlotResetRightRangeInFrame, 0);


			if (m_bSkipFirstByteCalculation == true)
			{
				m_bSkipFirstByteCalculation = false;
			}
			else
			{
				m_miniPacketBandCounter = SlotResetLeftRangeInFrame - FRAME_RATE;//if we miss all frames of the previous slot it will be wrong
				m_miniPacketBandCounter = m_miniPacketBandCounter / FRAME_RATE;
				CreateAndSendMiniPacket((m_ByteRcvInBandSlot), INVALID_PACKET_NUMBER);

				CLogPrinter_WriteSpecific5(CLogPrinter::DEBUGS, "VampireEnggUpt--> m_SlotLeft, m_SlotRight = (" + m_Tools.IntegertoStringConvert(m_SlotResetLeftRange/MAX_PACKET_NUMBER)
																+", "+m_Tools.IntegertoStringConvert(m_SlotResetRightRange/MAX_PACKET_NUMBER)+")........ m_ByteReceived = "+m_Tools.IntegertoStringConvert(m_ByteRcvInBandSlot)
																+" Curr(FN,PN) = ("+m_Tools.IntegertoStringConvert(NowRecvHeader.getFrameNumber())+","+m_Tools.IntegertoStringConvert(NowRecvHeader.getPacketNumber())+")");
			}

			m_ByteRcvInBandSlot = NowRecvHeader.getPacketLength() - PACKET_HEADER_LENGTH_WITH_MEDIA_TYPE;


		}

#endif



		m_pVideoPacketQueue->Queue(in_data, in_size);
	}

	return true;
}

long long g_EncodingTimeDiff = 0;
int CVideoCallSession::PushIntoBufferForEncoding(unsigned char *in_data, unsigned int in_size)
{
	CLogPrinter_Write(CLogPrinter::INFO, "CVideoCallSession::PushIntoBufferForEncoding");

	LongLong currentTimeStamp = m_Tools.CurrentTimestamp();

	if (m_ClientFrameCounter++)
	{
		m_ClientFPSDiffSum += currentTimeStamp - m_LastTimeStampClientFPS;

		{//Block for LOCK
			int  nApproximateAverageFrameInterval = m_ClientFPSDiffSum / m_ClientFrameCounter;
			if(nApproximateAverageFrameInterval > 10) {
				Locker lock(*m_pSessionMutex);
				g_FPSController.SetClientFPS(1000 / nApproximateAverageFrameInterval);
			}
		}

		m_DropSum = 0;
	}

	m_LastTimeStampClientFPS = currentTimeStamp;

#if defined(TARGET_OS_IPHONE) || defined(TARGET_IPHONE_SIMULATOR)

	CLogPrinter_Write(CLogPrinter::INFO, "CVideoCallSession::PushIntoBufferForEncoding 2");
	//this->m_pColorConverter->ConvertNV12ToI420(m_EncodingFrame);

	CLogPrinter_Write(CLogPrinter::DEBUGS, "CVideoCallSession::PushIntoBufferForEncoding Converted to 420");

#endif
    
    CLogPrinter_WriteLog(CLogPrinter::INFO, INSTENT_TEST_LOG, "TheKing--> Encoding TimeDiff = " + m_Tools.LongLongtoStringConvert(m_Tools.CurrentTimestamp() - g_EncodingTimeDiff));
	int returnedValue = m_EncodingBuffer->Queue(in_data, in_size);
    g_EncodingTimeDiff = m_Tools.CurrentTimestamp();
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

void CVideoCallSession::CreateAndSendMiniPacket(int resendFrameNumber, int resendPacketNumber)
{
	unsigned char uchVersion = g_uchSendPacketVersion;

//    if(INVALID_PACKET_NUMBER !=resendPacketNumber && resendFrameNumber % I_INTRA_PERIOD != 0 ) //
	if(INVALID_PACKET_NUMBER !=resendPacketNumber  && INVALID_PACKET_NUMBER_FOR_NETWORK_TYPE !=resendPacketNumber) //
	{
		return;
	}

	int numberOfPackets = 1000; //dummy numberOfPackets

	CPacketHeader PacketHeader;
	if (resendPacketNumber == INVALID_PACKET_NUMBER) {
		//m_miniPacketBandCounter++;
		if(0 == uchVersion) return;

		PacketHeader.setPacketHeader(uchVersion, m_miniPacketBandCounter/*SlotID*/, 0, resendPacketNumber/*Invalid_Packet*/, resendFrameNumber/*BandWidth*/, 0, 0, 0);
	}
	else if (resendPacketNumber == INVALID_PACKET_NUMBER_FOR_NETWORK_TYPE) {
		//m_miniPacketBandCounter++;
		//if(0 == uchVersion) return;

		CLogPrinter_WriteSpecific5(CLogPrinter::DEBUGS, " send INVALID-->> PACKET_NUMBER_FOR_NETWORK_TYPE ");
		PacketHeader.setPacketHeader(uchVersion, m_miniPacketBandCounter/*SlotID*/, 0, resendPacketNumber/*Invalid_Packet*/, resendFrameNumber/*BandWidth*/, 0, 0, 0);
	}
	else {
		PacketHeader.setPacketHeader(uchVersion, resendFrameNumber, numberOfPackets, resendPacketNumber, 0, 0, 0, 0);
		g_timeInt.setTime(resendFrameNumber,resendPacketNumber);
	}

	m_miniPacket[0] = (int)VIDEO_PACKET_MEDIA_TYPE;

	PacketHeader.GetHeaderInByteArray(m_miniPacket + 1);

	m_miniPacket[RETRANSMISSION_SIG_BYTE_INDEX_WITHOUT_MEDIA + 1] |= 1<<BIT_INDEX_MINI_PACKET; //MiniPacket Flag

	if(uchVersion)
		m_pCommonElementsBucket->SendFunctionPointer(friendID, 2, m_miniPacket,PACKET_HEADER_LENGTH + 1);
	else
		m_pCommonElementsBucket->SendFunctionPointer(friendID, 2, m_miniPacket,PACKET_HEADER_LENGTH_NO_VERSION + 1);

	//m_SendingBuffer.Queue(frameNumber, miniPacket, PACKET_HEADER_LENGTH_WITH_MEDIA_TYPE);
}


int CVideoCallSession::GetUniquePacketID(int fn, int pn)
{
	return fn*MAX_PACKET_NUMBER + pn;
}









