#include "VideoCallSession.h"
#include "CommonElementsBucket.h"
#include "LogPrinter.h"
#include "Tools.h"
#include "Globals.h"
#include "ResendingBuffer.h"
//#include "Helper_IOS.h"

#ifdef RETRANSMITTED_FRAME_USAGE_STATISTICS_ENABLED
map<int, int> g_TraceRetransmittedFrame;
#endif



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

int g_OppNotifiedByterate = 0;

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
m_bMegSlotCounterShouldStop(true),
m_bsetBitrateCalled(false),
m_iConsecutiveGoodMegaSlot(0),
m_iPreviousByterate(BITRATE_MAX / 8),
m_LastSendingSlot(0),
m_iDePacketizeCounter(0),
m_TimeFor100Depacketize(0)
{
	m_miniPacketBandCounter = 0;

#ifdef RETRANSMITTED_FRAME_USAGE_STATISTICS_ENABLED
	g_TraceRetransmittedFrame.clear();
#endif

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

#ifdef RETRANSMITTED_FRAME_USAGE_STATISTICS_ENABLED
	g_TraceRetransmittedFrame.clear();
#endif



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

	m_pEncodedFrameDepacketizer = NULL;
	m_pEncodedFramePacketizer = new CEncodedFramePacketizer(sharedObject);
	m_pEncodedFrameDepacketizer = new CEncodedFrameDepacketizer(sharedObject, this);

	m_BitRateController = new BitRateController();
	m_EncodingBuffer = new CEncodingBuffer();
	m_RenderingBuffer = new CRenderingBuffer();

	m_pVideoPacketQueue = new CVideoPacketQueue();
	m_pRetransVideoPacketQueue = new CVideoPacketQueue();
	m_pMiniPacketQueue = new CVideoPacketQueue();

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

	StopDepacketizationThread();
	StopDecodingThread();
	StopEncodingThread();
	StopRenderingThread();

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
	friendID = -1;

	SHARED_PTR_DELETE(m_pSessionMutex);
}

LongLong CVideoCallSession::GetFriendID()
{
	return friendID;
}

void CVideoCallSession::InitializeVideoSession(LongLong lFriendID, int iVideoHeight, int iVideoWidth)
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

	m_pVideoEncodingThread = new CVideoEncodingThread(lFriendID, m_EncodingBuffer, m_BitRateController, m_pColorConverter, m_pVideoEncoder, m_pEncodedFramePacketizer);
	m_pVideoRenderingThread = new CVideoRenderingThread(lFriendID, m_RenderingBuffer, m_pCommonElementsBucket);
	m_pVideoDecodingThread = new CVideoDecodingThread(m_pEncodedFrameDepacketizer, m_RenderingBuffer, m_pVideoDecoder, m_pColorConverter, &g_FPSController);
	m_pVideoDepacketizationThread = new CVideoDepacketizationThread(lFriendID, m_pVideoPacketQueue, m_pRetransVideoPacketQueue, m_pMiniPacketQueue, m_BitRateController, m_pEncodedFrameDepacketizer, m_pCommonElementsBucket, &m_miniPacketBandCounter);

	m_pCommonElementsBucket->m_pVideoEncoderList->AddToVideoEncoderList(lFriendID, m_pVideoEncoder);

	m_ClientFrameCounter = 0;
	m_EncodingFrameCounter = 0;

	StartRenderingThread();
	StartEncodingThread();
	StartDepacketizationThread();
	StartDecodingThread();

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
			}

			m_ByteRcvInBandSlot = NowRecvHeader.getPacketLength() - PACKET_HEADER_LENGTH_WITH_MEDIA_TYPE;

			//printf("VampireEnggUpt--> m_SlotLeft, m_SlotRight = (%d, %d)........ m_ByteReceived = %d\nCurr(FN,PN) = (%d,%d)\n", m_SlotResetLeftRange/MAX_PACKET_NUMBER, m_SlotResetRightRange/MAX_PACKET_NUMBER, m_ByteRcvInBandSlot, NowRecvHeader.getFrameNumber(), NowRecvHeader.getPacketNumber());


		}

#endif



		m_pVideoPacketQueue->Queue(in_data, in_size);
	}

	return true;
}

int CVideoCallSession::PushIntoBufferForEncoding(unsigned char *in_data, unsigned int in_size)
{
	CLogPrinter_Write(CLogPrinter::INFO, "CVideoCallSession::PushIntoBufferForEncoding");

	LongLong currentTimeStamp = m_Tools.CurrentTimestamp();

	if (m_ClientFrameCounter++)
	{
		m_ClientFPSDiffSum += currentTimeStamp - m_LastTimeStampClientFPS;


		{//Block for LOCK
			Locker lock(*m_pSessionMutex);
			g_FPSController.SetClientFPS(1000 / (m_ClientFPSDiffSum / m_ClientFrameCounter));
			//		m_ClientFPS = 1000 / (m_ClientFPSDiffSum / m_ClientFrameCounter);
			//		m_ClientFPS = 1000/(currentTimeStamp - m_LastTimeStampClientFPS);
		}

		m_DropSum = 0;
	}

	m_LastTimeStampClientFPS = currentTimeStamp;

#if defined(TARGET_OS_IPHONE) || defined(TARGET_IPHONE_SIMULATOR)

	CLogPrinter_Write(CLogPrinter::INFO, "CVideoCallSession::PushIntoBufferForEncoding 2");
	//this->m_pColorConverter->ConvertNV12ToI420(m_EncodingFrame);

	CLogPrinter_Write(CLogPrinter::DEBUGS, "CVideoCallSession::PushIntoBufferForEncoding Converted to 420");

#endif

	int returnedValue = m_EncodingBuffer->Queue(in_data, in_size);

	CLogPrinter_WriteInstentTestLog(CLogPrinter::INFO, "CVideoCallSession::PushIntoBufferForEncoding Queue packetSize " + Tools::IntegertoStringConvert(in_size));

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

void CVideoCallSession::StopEncodingThread()
{
	m_pVideoEncodingThread->StopEncodingThread();
	/*
	//if (pInternalThread.get())
	{

	bEncodingThreadRunning = false;

	while (!bEncodingThreadClosed)
	{
	m_Tools.SOSleep(5);
	}
	}

	//pInternalThread.reset();
	*/
}

void CVideoCallSession::StartEncodingThread()
{
	m_pVideoEncodingThread->StartEncodingThread();

	/*
	if (pEncodingThread.get())
	{
	CLogPrinter_Write(CLogPrinter::INFO, "CVideoCallSession::StartedInternalThread 2");
	pEncodingThread.reset();
	CLogPrinter_Write(CLogPrinter::INFO, "CVideoCallSession::StartDepacketizationThread 3");
	return;
	}
	CLogPrinter_Write(CLogPrinter::INFO, "CVideoCallSession::StartedInternalThread 4");
	bEncodingThreadRunning = true;
	bEncodingThreadClosed = false;
	CLogPrinter_Write(CLogPrinter::INFO, "CVideoCallSession::StartedInternalThread 5");

	#if defined(TARGET_OS_IPHONE) || defined(TARGET_IPHONE_SIMULATOR)

	dispatch_queue_t EncodeThreadQ = dispatch_queue_create("EncodeThreadQ",DISPATCH_QUEUE_CONCURRENT);
	dispatch_async(EncodeThreadQ, ^{
	this->EncodingThreadProcedure();
	});

	#else

	std::thread myThread(CreateVideoEncodingThread, this);
	myThread.detach();

	#endif

	CLogPrinter_Write(CLogPrinter::INFO, "CVideoCallSession::StartedInternalThread Encoding Thread started");

	return;
	*/

}

void *CVideoCallSession::CreateVideoEncodingThread(void* param)
{
	/*
	CVideoCallSession *pThis = (CVideoCallSession*)param;
	pThis->EncodingThreadProcedure();
	*/
	return NULL;
}

void CVideoCallSession::EncodingThreadProcedure()
{

	/*
	CLogPrinter_Write(CLogPrinter::DEBUGS, "CVideoCallSession::EncodingThreadProcedure() Started EncodingThreadProcedure.");
	Tools toolsObject;
	int frameSize, encodedFrameSize;
	long long encodingTime, encodingTimeStamp, nMaxEncodingTime = 0, currentTimeStamp;
	double dbTotalEncodingTime=0;
	int iEncodedFrameCounter=0;
	encodingTimeStamp = toolsObject.CurrentTimestamp();
	long long encodingTimeFahadTest = 0;

	long long iterationtime = toolsObject.CurrentTimestamp();

	while (bEncodingThreadRunning)
	{
	//CLogPrinter_Write(CLogPrinter::INFO, "CVideoCallSession::InternalThreadImpl");

	if (m_EncodingBuffer->GetQueueSize() == 0)
	toolsObject.SOSleep(10);
	else
	{
	long long sleepTimeStamp1 = toolsObject.CurrentTimestamp();//total time

	int timeDiff;
	frameSize = m_EncodingBuffer->DeQueue(m_EncodingFrame, timeDiff);
	CLogPrinter_WriteForQueueTime(CLogPrinter::INFO, " &*&*&* m_EncodingBuffer ->" + toolsObject.IntegertoStringConvert(timeDiff));
	//			CLogPrinter_WriteSpecific(CLogPrinter::INFO, "Before Processable");
	if(!g_FPSController.IsProcessableFrame())
	{
	toolsObject.SOSleep(10);
	continue;
	}

	if(m_bFirstFrame)
	{
	m_ll1stFrameTimeStamp = toolsObject.CurrentTimestamp();
	m_bFirstFrame = false;
	}

	m_iTimeStampDiff = toolsObject.CurrentTimestamp() - m_ll1stFrameTimeStamp;


	m_FrameCounterbeforeEncoding++;
	m_BitRateController->UpdateBitrate();

	//			if(m_FrameCounterbeforeEncoding%FRAME_RATE == 0 && g_OppNotifiedByterate>0 && m_bsetBitrateCalled == false)
	//			{
	//
	//                int iRet = -1, iRet2 = -1;
	//                int iCurrentBitRate = g_OppNotifiedByterate* 8 - nFirstTimeDecrease;
	//				nFirstTimeDecrease = 0;
	//
	//				CLogPrinter_WriteSpecific2(CLogPrinter::DEBUGS, " $$$*( SET BITRATE :"+ m_Tools.IntegertoStringConvert(iCurrentBitRate)+"  Pre: "+ m_Tools.IntegertoStringConvert(m_iPreviousByterate));
	//                //printf("VampireEngg--> iCurrentBitRate = %d, g_OppNotifiedByteRate = %d\n", iCurrentBitRate, g_OppNotifiedByterate);
	//
	//                if(iCurrentBitRate < m_pVideoEncoder->GetBitrate())
	//                {
	//                    iRet = m_pVideoEncoder->SetBitrate(iCurrentBitRate);
	//
	//                    if(iRet == 0) //First Initialization Successful
	//                        iRet2 = m_pVideoEncoder->SetMaxBitrate(iCurrentBitRate);
	//
	//                }
	//                else
	//                {
	//                    iRet = m_pVideoEncoder->SetMaxBitrate(iCurrentBitRate);
	//
	//                    if(iRet == 0) //First Initialization Successful
	//                        iRet2 = m_pVideoEncoder->SetBitrate(iCurrentBitRate);
	//                }
	//
	//                if(iRet == 0 && iRet2 ==0) //We are intentionally skipping status of setbitrate operation success
	//                {
	//                    m_iPreviousByterate = iCurrentBitRate/8;
	//
	//                    m_bMegSlotCounterShouldStop = false;
	//                }
	//
	//                m_bsetBitrateCalled = true;
	//
	//            }


	//			CLogPrinter_WriteSpecific(CLogPrinter::INFO, "$ENCODEING$");
	#if defined(TARGET_OS_IPHONE) || defined(TARGET_IPHONE_SIMULATOR)

	currentTimeStamp = CLogPrinter_WriteForOperationTime(CLogPrinter::DEBUGS, "");
	this->m_pColorConverter->ConvertNV12ToI420(m_EncodingFrame);
	CLogPrinter_WriteForOperationTime(CLogPrinter::DEBUGS, " ConvertNV12ToI420 ", currentTimeStamp);

	currentTimeStamp = CLogPrinter_WriteForOperationTime(CLogPrinter::DEBUGS, "");
	encodedFrameSize = m_pVideoEncoder->EncodeAndTransfer(m_EncodingFrame, frameSize, m_EncodedFrame);

	CLogPrinter_WriteForOperationTime(CLogPrinter::DEBUGS, " Encode ", currentTimeStamp);

	CLogPrinter_Write(CLogPrinter::DEBUGS, "CVideoCallSession::EncodingThreadProcedure video data encoded");
	#elif defined(_DESKTOP_C_SHARP_)


	currentTimeStamp = CLogPrinter_WriteForOperationTime(CLogPrinter::DEBUGS, "");
	frameSize = this->m_pColorConverter->ConvertYUY2ToI420(m_EncodingFrame, m_ConvertedEncodingFrame);
	//printf("WinD--> CVideoCallSession::EncodingThreadProcedure frameSIze: %d\n", frameSize);
	encodedFrameSize = m_pVideoEncoder->EncodeAndTransfer(m_ConvertedEncodingFrame, frameSize, m_EncodedFrame);
	//printf("WinD--> CVideoCallSession::EncodingThreadProcedure encodedFrameSize: %d\n", encodedFrameSize);
	CLogPrinter_WriteForOperationTime(CLogPrinter::DEBUGS, " Encode ", currentTimeStamp);

	#elif defined(TARGET_OS_WINDOWS_PHONE)

	currentTimeStamp = CLogPrinter_WriteForOperationTime(CLogPrinter::DEBUGS, "");
	if (orientation_type == ORIENTATION_90_MIRRORED)
	{
	this->m_pColorConverter->mirrorRotateAndConvertNV12ToI420(m_EncodingFrame, m_ConvertedEncodingFrame);
	}
	else if (orientation_type == ORIENTATION_0_MIRRORED)
	{
	CLogPrinter_Write(CLogPrinter::DEBUGS, "orientation_type : " + m_Tools.IntegertoStringConvert(orientation_type) + " ORIENTATION_0_MIRRORED ");
	this->m_pColorConverter->mirrorRotateAndConvertNV12ToI420ForBackCam(m_EncodingFrame, m_ConvertedEncodingFrame);
	}
	CLogPrinter_WriteForOperationTime(CLogPrinter::DEBUGS, " ConvertNV12ToI420 ", currentTimeStamp);

	long long enctime = m_Tools.CurrentTimestamp();

	currentTimeStamp = CLogPrinter_WriteForOperationTime(CLogPrinter::DEBUGS, "");
	encodedFrameSize = m_pVideoEncoder->EncodeAndTransfer(m_ConvertedEncodingFrame, frameSize, m_EncodedFrame);
	CLogPrinter_WriteForOperationTime(CLogPrinter::DEBUGS, " Encode ", currentTimeStamp);

	//printf("enctime = %lld\n", m_Tools.CurrentTimestamp() - enctime);

	CLogPrinter_Write(CLogPrinter::DEBUGS, "CVideoCallSession::EncodingThreadProcedure video data encoded");
	#else

	CLogPrinter_Write(CLogPrinter::DEBUGS, "orientation_type : "+  m_Tools.IntegertoStringConvert(orientation_type));

	currentTimeStamp = CLogPrinter_WriteForOperationTime(CLogPrinter::DEBUGS, "");
	if(orientation_type == ORIENTATION_90_MIRRORED)
	{
	CLogPrinter_Write(CLogPrinter::DEBUGS, "orientation_type : "+  m_Tools.IntegertoStringConvert(orientation_type)+ "  ORIENTATION_90_MIRRORED");
	this->m_pColorConverter->mirrorRotateAndConvertNV21ToI420(m_EncodingFrame, m_ConvertedEncodingFrame);
	}
	else if(orientation_type == ORIENTATION_0_MIRRORED)
	{
	CLogPrinter_Write(CLogPrinter::DEBUGS, "orientation_type : "+  m_Tools.IntegertoStringConvert(orientation_type) + " ORIENTATION_0_MIRRORED ");
	this->m_pColorConverter->mirrorRotateAndConvertNV21ToI420ForBackCam(m_EncodingFrame, m_ConvertedEncodingFrame);
	}
	CLogPrinter_WriteForOperationTime(CLogPrinter::DEBUGS, " ConvertNV12ToI420 ", currentTimeStamp);

	CLogPrinter_Write(CLogPrinter::DEBUGS, "CVideoCallSession::EncodingThreadProcedure Converted to 420");
	encodingTimeStamp = toolsObject.CurrentTimestamp();
	currentTimeStamp = CLogPrinter_WriteForOperationTime(CLogPrinter::DEBUGS, "");
	encodedFrameSize = m_pVideoEncoder->EncodeAndTransfer(m_ConvertedEncodingFrame, frameSize, m_EncodedFrame);
	CLogPrinter_WriteForOperationTime(CLogPrinter::DEBUGS, " Encode ", currentTimeStamp);
	encodingTime  = toolsObject.CurrentTimestamp() - encodingTimeStamp;

	encodeTimeStampFor15 += encodingTime;


	long long sleepTimeStamp3 = toolsObject.CurrentTimestamp(); //packetization time

	countFrameSize = countFrameSize + encodedFrameSize;
	if(countFrame >= 15)
	{
	encodingTimeFahadTest  = toolsObject.CurrentTimestamp() - encodingTimeFahadTest;
	CLogPrinter_WriteSpecific3(CLogPrinter::DEBUGS, "Encoded "+Tools::IntegertoStringConvert(countFrame)+" frames Size: " + Tools::IntegertoStringConvert(countFrameSize*8) + " encodeTimeStampFor15 : " + Tools::IntegertoStringConvert(encodeTimeStampFor15)+  " Full_Lop: " + Tools::IntegertoStringConvert(encodingTimeFahadTest));
	encodingTimeFahadTest = toolsObject.CurrentTimestamp();
	countFrame = 0;
	countFrameSize= 0;
	encodeTimeStampFor15 = 0;
	}
	countFrame++;

	dbTotalEncodingTime += encodingTime;
	++ iEncodedFrameCounter;
	nMaxEncodingTime = max(nMaxEncodingTime,encodingTime);


	#endif
	m_BitRateController->NotifyEncodedFrame(encodedFrameSize);

	//            m_ByteSendInSlotInverval+=encodedFrameSize;
	//            if(m_FrameCounterbeforeEncoding % FRAME_RATE == 0)
	//            {
	//
	//
	//                int ratioHelperIndex = (m_FrameCounterbeforeEncoding - FRAME_RATE) / FRAME_RATE;
	//                if(m_bMegSlotCounterShouldStop == false)
	//                {
	//                    //printf("VampireEngg--> ***************m_ByteSendInSlotInverval = (%d, %d)\n", ratioHelperIndex, m_ByteSendInSlotInverval);
	//					m_LastSendingSlot = ratioHelperIndex;
	//                    m_BandWidthRatioHelper.insert(ratioHelperIndex, m_ByteSendInSlotInverval);
	//                }
	//
	//                m_ByteSendInSlotInverval = 0;
	//            }


	//			CLogPrinter_WriteSpecific(CLogPrinter::INFO, "CVideoCallSession::EncodingThreadProcedure m_iFrameNumber : "+ m_Tools.IntegertoStringConvert(m_iFrameNumber) + " :: encodedFrameSize: " + m_Tools.IntegertoStringConvert(encodedFrameSize));
	//			CLogPrinter_WriteSpecific(CLogPrinter::INFO, "$ENCODEING$ To Parser");
	//m_pVideoEncoder->GetEncodedFramePacketizer()->Packetize(friendID,m_EncodedFrame, encodedFrameSize, m_iFrameNumber, m_iTimeStampDiff);
	currentTimeStamp = CLogPrinter_WriteForOperationTime(CLogPrinter::DEBUGS, "");
	//- (void)WriteToFile:(const char *)path withData:(unsigned char *)data dataLength:(int)datalen


	//    if(m_iFrameNumber<200)
	//    {
	//        string str ="/Encode/"+m_Tools.IntegertoStringConvert(m_iFrameNumber) + "_" + m_Tools.IntegertoStringConvert(encodedFrameSize);
	//        str+=".dump";
	//        [[Helper_IOS GetInstance] WriteToFile:str.c_str() withData:m_EncodedFrame dataLength:encodedFrameSize];
	//    }



	m_pEncodedFramePacketizer->Packetize(friendID,m_EncodedFrame, encodedFrameSize, m_iFrameNumber, m_iTimeStampDiff);
	CLogPrinter_WriteForOperationTime(CLogPrinter::DEBUGS, " Packetize " , currentTimeStamp);
	++m_iFrameNumber;
	//CLogPrinter_WriteSpecific(CLogPrinter::INFO, "CVideoCallSession::EncodingThreadProcedure2 m_iFrameNumber : "+ m_Tools.IntegertoStringConvert(CVideoCallSession::m_iFrameNumber) + " :: encodedFrameSize: " + m_Tools.IntegertoStringConvert(encodedFrameSize));

	toolsObject.SOSleep(1);

	}
	}

	bEncodingThreadClosed = true;

	CLogPrinter_Write(CLogPrinter::DEBUGS, "CVideoCallSession::EncodingThreadProcedure() Stopped EncodingThreadProcedure");
	*/
}

void CVideoCallSession::StopDepacketizationThread()
{
	m_pVideoDepacketizationThread->StopDepacketizationThread();

/*
	//if (pDepacketizationThread.get())
	{
		bDepacketizationThreadRunning = false;
		Tools toolsObject;

		while (!bDepacketizationThreadClosed)
		{
			m_Tools.SOSleep(5);
		}
	}

	//pDepacketizationThread.reset();
*/
}

void CVideoCallSession::StartDepacketizationThread()
{
	m_pVideoDepacketizationThread->StartDepacketizationThread();

/*
	CLogPrinter_Write(CLogPrinter::INFO, "CVideoCallSession::StartDepacketizationThread 1");
	if (pDepacketizationThread.get())
	{
		//		CLogPrinter_Write(CLogPrinter::INFO, "CVideoCallSession::StartDepacketizationThread 2");
		pDepacketizationThread.reset();
		//		CLogPrinter_Write(CLogPrinter::INFO, "CVideoCallSession::StartDepacketizationThread 3");
		return;
	}
	//	CLogPrinter_Write(CLogPrinter::INFO, "CVideoCallSession::StartDepacketizationThread 4");
	bDepacketizationThreadRunning = true;
	bDepacketizationThreadClosed = false;
	CLogPrinter_Write(CLogPrinter::INFO, "CVideoCallSession::StartDepacketizationThread 5");

#if defined(TARGET_OS_IPHONE) || defined(TARGET_IPHONE_SIMULATOR)

	dispatch_queue_t DecodeThreadQ = dispatch_queue_create("DecodeThreadQ", DISPATCH_QUEUE_CONCURRENT);
	dispatch_async(DecodeThreadQ, ^{
		this->DepacketizationThreadProcedure();
	});

#else

	std::thread myThread(CreateVideoDepacketizationThread, this);
	myThread.detach();

#endif

	CLogPrinter_Write(CLogPrinter::INFO, "CVideoCallSession::StartDepacketizationThread Decoding Thread started");

	return;
*/
}

void *CVideoCallSession::CreateVideoDepacketizationThread(void* param)
{
/*
	CVideoCallSession *pThis = (CVideoCallSession*)param;
	pThis->DepacketizationThreadProcedure();
*/
	return NULL;
}

//int iValuableFrameUsedCounter = 0;

int CVideoCallSession::DecodeAndSendToClient(unsigned char *in_data, unsigned int frameSize, int nFramNumber, unsigned int nTimeStampDiff)
{
	/*
	//printf("Wind--> DecodeAndSendToClient 0\n");
	#ifdef RETRANSMITTED_FRAME_USAGE_STATISTICS_ENABLED
	if (g_TraceRetransmittedFrame[nFramNumber] == 1)
	{
	CLogPrinter_WriteSpecific2(CLogPrinter::INFO, "Very valuable frame used " + m_Tools.IntegertoStringConvert(nFramNumber) + ", counter =  " + m_Tools.IntegertoStringConvert(iValuableFrameUsedCounter));
	iValuableFrameUsedCounter++;
	}
	CLogPrinter_WriteSpecific2(CLogPrinter::INFO, "$$$Very Valuable Retransmission packet used counter =  " + m_Tools.IntegertoStringConvert(iValuableFrameUsedCounter));

	#endif

	long long currentTimeStamp = CLogPrinter_WriteForOperationTime(CLogPrinter::DEBUGS, "");
	m_decodedFrameSize = m_pVideoDecoder->Decode(in_data, frameSize, m_DecodedFrame, m_decodingHeight, m_decodingWidth);

	CLogPrinter_WriteForOperationTime(CLogPrinter::DEBUGS, " Decode ", currentTimeStamp);

	if (1 > m_decodedFrameSize)
	return -1;

	currentTimeStamp = CLogPrinter_WriteForOperationTime(CLogPrinter::DEBUGS, " ConvertI420ToNV21 ");
	#if defined(TARGET_OS_IPHONE) || defined(TARGET_IPHONE_SIMULATOR)

	this->m_pColorConverter->ConvertI420ToNV12(m_DecodedFrame, m_decodingHeight, m_decodingWidth);
	#elif defined(_DESKTOP_C_SHARP_)
	//	CLogPrinter_WriteSpecific(CLogPrinter::DEBUGS, "DepacketizationThreadProcedure() For Desktop");
	m_decodedFrameSize = this->m_pColorConverter->ConverterYUV420ToRGB24(m_DecodedFrame, m_RenderingRGBFrame, m_decodingHeight, m_decodingWidth);
	#elif defined(TARGET_OS_WINDOWS_PHONE)
	this->m_pColorConverter->ConvertI420ToYV12(m_DecodedFrame, m_decodingHeight, m_decodingWidth);
	#else

	this->m_pColorConverter->ConvertI420ToNV21(m_DecodedFrame, m_decodingHeight, m_decodingWidth);
	#endif
	CLogPrinter_WriteForOperationTime(CLogPrinter::DEBUGS, " ConvertI420ToNV21 ", currentTimeStamp);
	#if defined(_DESKTOP_C_SHARP_)
	m_RenderingBuffer->Queue(nFramNumber, m_RenderingRGBFrame, m_decodedFrameSize, nTimeStampDiff, m_decodingHeight, m_decodingWidth);
	return m_decodedFrameSize;
	#else

	m_RenderingBuffer->Queue(nFramNumber, m_DecodedFrame, m_decodedFrameSize, nTimeStampDiff, m_decodingHeight, m_decodingWidth);
	return m_decodedFrameSize;
	#endif
	*/

	return 0;
}


void CVideoCallSession::DepacketizationThreadProcedure()		//Merging Thread
{
/*
	CLogPrinter_Write(CLogPrinter::DEBUGS, "CVideoCallSession::DepacketizationThreadProcedure() Started DepacketizationThreadProcedure method.");
	Tools toolsObject;
	unsigned char temp;
	int frameSize, queSize = 0, retQueuSize = 0, miniPacketQueueSize = 0, consicutiveRetransmittedPkt = 0;
	int frameNumber, packetNumber;
	bool bIsMiniPacket;
	m_iCountRecResPack = 0;
	int iPacketType = NORMAL_PACKET;

	while (bDepacketizationThreadRunning)
	{
		bIsMiniPacket = false;
		//CLogPrinter_Write(CLogPrinter::INFO, "CVideoCallSession::DepacketizationThreadProcedure");
		queSize = m_pVideoPacketQueue->GetQueueSize();
#ifdef	RETRANSMISSION_ENABLED
		retQueuSize = m_pRetransVideoPacketQueue->GetQueueSize();
		miniPacketQueueSize = m_pMiniPacketQueue->GetQueueSize();
		//		CLogPrinter_WriteSpecific(CLogPrinter::DEBUGS, "SIZE "+ m_Tools.IntegertoStringConvert(retQueuSize)+"  "+ m_Tools.IntegertoStringConvert(queSize));
#endif
		if (0 == queSize && 0 == retQueuSize && 0 == miniPacketQueueSize)
			toolsObject.SOSleep(10);
		else
		{
		//	g_iPacketCounterSinceNotifying++;
#ifdef	RETRANSMISSION_ENABLED
			if (miniPacketQueueSize != 0)
			{
				frameSize = m_pMiniPacketQueue->DeQueue(m_PacketToBeMerged);
			}
			else if (retQueuSize>0 && consicutiveRetransmittedPkt<2)
			{
				//	CLogPrinter_WriteSpecific(CLogPrinter::DEBUGS, "RT QueueSize"+ m_Tools.IntegertoStringConvert(retQueuSize));
				frameSize = m_pRetransVideoPacketQueue->DeQueue(m_PacketToBeMerged);
				++consicutiveRetransmittedPkt;
			}
			else if (queSize>0){
#endif
				frameSize = m_pVideoPacketQueue->DeQueue(m_PacketToBeMerged);

#ifdef	RETRANSMISSION_ENABLED
				consicutiveRetransmittedPkt = 0;
			}
			else
			{
				frameSize = m_pRetransVideoPacketQueue->DeQueue(m_PacketToBeMerged);
				++consicutiveRetransmittedPkt;
			}
			m_RcvdPacketHeader.setPacketHeader(m_PacketToBeMerged);
			CLogPrinter_WriteSpecific4(CLogPrinter::DEBUGS, "!@# Versions: " + m_Tools.IntegertoStringConvert(g_uchSendPacketVersion));
			//			CLogPrinter_WriteSpecific2(CLogPrinter::INFO, "VC..>>>  FN: "+ m_Tools.IntegertoStringConvert(m_RcvdPacketHeader.getFrameNumber()) + "  pk: "+ m_Tools.IntegertoStringConvert(m_RcvdPacketHeader.getPacketNumber())
			//														  + " tmDiff : " + m_Tools.IntegertoStringConvert(m_RcvdPacketHeader.getTimeStamp()));

			bool bRetransmitted = (m_PacketToBeMerged[RETRANSMISSION_SIG_BYTE_INDEX_WITHOUT_MEDIA] >> BIT_INDEX_RETRANS_PACKET) & 1;
			bool bMiniPacket = (m_PacketToBeMerged[RETRANSMISSION_SIG_BYTE_INDEX_WITHOUT_MEDIA] >> BIT_INDEX_MINI_PACKET) & 1;
			m_PacketToBeMerged[RETRANSMISSION_SIG_BYTE_INDEX_WITHOUT_MEDIA] = 0;

			if (bMiniPacket && m_RcvdPacketHeader.getPacketNumber() == INVALID_PACKET_NUMBER)
			{
				m_BitRateController->HandleBitrateMiniPacket(m_RcvdPacketHeader);
				toolsObject.SOSleep(1);
				continue;
			}

			if (!bRetransmitted && !bMiniPacket)
			{

				iPacketType = NORMAL_PACKET;




				int iNumberOfPackets = m_RcvdPacketHeader.getNumberOfPacket();
				pair<int, int> currentFramePacketPair = make_pair(m_RcvdPacketHeader.getFrameNumber(), m_RcvdPacketHeader.getPacketNumber());


				if (currentFramePacketPair != ExpectedFramePacketPair && !m_pVideoPacketQueue->PacketExists(ExpectedFramePacketPair.first, ExpectedFramePacketPair.second)) //Out of order frame found, need to retransmit
				{
					
				//	string sMsg = "CVideoCallSession::Current(FN,PN) = ("
				//	+ m_Tools.IntegertoStringConvert(currentFramePacketPair.first)
				//	+ ","
				//	+ m_Tools.IntegertoStringConvert(currentFramePacketPair.second)
				//	+ ") and Expected(FN,PN) = ("
				//	+ m_Tools.IntegertoStringConvert(ExpectedFramePacketPair.first)
				//	+ ","
				//	+ m_Tools.IntegertoStringConvert(ExpectedFramePacketPair.second)
				//	+ ")" ;
				//	printf("%s\n", sMsg.c_str());
					

				//	if (g_iPacketCounterSinceNotifying >= FPS_SIGNAL_IDLE_FOR_PACKETS)
				//	{
						//						g_FPSController.NotifyFrameDropped(currentFramePacketPair.first);
				//		g_iPacketCounterSinceNotifying = 0;
				//		gbStopFPSSending = false;
				//	}
				//	else
				//	{
				//		gbStopFPSSending = true;
				//	}


					if (currentFramePacketPair.first != ExpectedFramePacketPair.first) //different frame received
					{
						if (currentFramePacketPair.first - ExpectedFramePacketPair.first == 2) //one complete frame missed, maybe it was a mini frame containing only 1 packet
						{
							CLogPrinter_WriteSpecific(CLogPrinter::DEBUGS, "ExpectedFramePacketPair case 1");
							CreateAndSendMiniPacket(ExpectedFramePacketPair.first, ExpectedFramePacketPair.second);
							pair<int, int> requestFramePacketPair;
							requestFramePacketPair.first = currentFramePacketPair.first;
							requestFramePacketPair.second = 0;

							int iSendCounter = 0;
							while (requestFramePacketPair.second < currentFramePacketPair.second) //
							{
								//if (iSendCounter && requestFramePacketPair.first %8 ==0) m_Tools.SOSleep(1);
								if (iSendCounter) m_Tools.SOSleep(1);
								if (!m_pVideoPacketQueue->PacketExists(requestFramePacketPair.first, requestFramePacketPair.second))
								{
									CreateAndSendMiniPacket(requestFramePacketPair.first, requestFramePacketPair.second);
								}
								iSendCounter++;
								requestFramePacketPair.second++;
							}
						}
						else if (currentFramePacketPair.first - ExpectedFramePacketPair.first == 1) //last packets from last frame and some packets from current misssed
						{
							CLogPrinter_WriteSpecific(CLogPrinter::DEBUGS, "ExpectedFramePacketPair case 2");
							pair<int, int> requestFramePacketPair;
							requestFramePacketPair.first = ExpectedFramePacketPair.first;
							requestFramePacketPair.second = ExpectedFramePacketPair.second;

							int iSendCounter = 0;
							while (requestFramePacketPair.second < iNumberOfPacketsInCurrentFrame)
							{
								if (iSendCounter) m_Tools.SOSleep(1);
								if (!m_pVideoPacketQueue->PacketExists(requestFramePacketPair.first, requestFramePacketPair.second))
								{
									CreateAndSendMiniPacket(requestFramePacketPair.first, requestFramePacketPair.second);
								}
								iSendCounter++;
								requestFramePacketPair.second++;
							}

							requestFramePacketPair.first = currentFramePacketPair.first;
							requestFramePacketPair.second = 0;

							iSendCounter = 0;
							while (requestFramePacketPair.second < currentFramePacketPair.second)
							{
								if (iSendCounter) m_Tools.SOSleep(1);
								if (!m_pVideoPacketQueue->PacketExists(requestFramePacketPair.first, requestFramePacketPair.second))
								{
									CreateAndSendMiniPacket(requestFramePacketPair.first, requestFramePacketPair.second);
								}
								iSendCounter++;
								requestFramePacketPair.second++;
							}

						}
						else//we dont handle burst frame miss, but 1st packets of the current frame should come, only if it is an iFrame
						{
							CLogPrinter_WriteSpecific(CLogPrinter::DEBUGS, "ExpectedFramePacketPair case 3-- killed previous frames");
							if (currentFramePacketPair.first % I_INTRA_PERIOD == 0)
							{
								pair<int, int> requestFramePacketPair;
								requestFramePacketPair.first = currentFramePacketPair.first;
								requestFramePacketPair.second = 0;

								int iSendCounter = 0;
								while (requestFramePacketPair.second < currentFramePacketPair.second)
								{
									if (iSendCounter) m_Tools.SOSleep(1);
									if (!m_pVideoPacketQueue->PacketExists(requestFramePacketPair.first, requestFramePacketPair.second))
									{
										CreateAndSendMiniPacket(requestFramePacketPair.first, requestFramePacketPair.second);
									}
									iSendCounter++;
									requestFramePacketPair.second++;
								}
							}
						}

					}
					else //packet missed from same frame
					{
						CLogPrinter_WriteSpecific(CLogPrinter::DEBUGS, "ExpectedFramePacketPair case 4");
						pair<int, int> requestFramePacketPair;
						requestFramePacketPair.first = ExpectedFramePacketPair.first;
						requestFramePacketPair.second = ExpectedFramePacketPair.second;

						int iSendCounter = 0;
						while (requestFramePacketPair.second < currentFramePacketPair.second)
						{
							if (iSendCounter) m_Tools.SOSleep(1);
							if (!m_pVideoPacketQueue->PacketExists(requestFramePacketPair.first, requestFramePacketPair.second))
							{
								CreateAndSendMiniPacket(requestFramePacketPair.first, requestFramePacketPair.second);
							}
							iSendCounter++;
							requestFramePacketPair.second++;
						}
					}
				}
				UpdateExpectedFramePacketPair(currentFramePacketPair, iNumberOfPackets);

			}
			else if (bRetransmitted)
			{
				iPacketType = RETRANSMITTED_PACKET;
				int iNumberOfPackets = m_RcvdPacketHeader.getNumberOfPacket();
#ifdef RETRANSMITTED_FRAME_USAGE_STATISTICS_ENABLED

				g_TraceRetransmittedFrame[m_RcvdPacketHeader.getFrameNumber()] = 1;
#endif
				CLogPrinter_WriteSpecific(CLogPrinter::DEBUGS, "CVideoCallSession::ReTransmitted: FrameNumber: " + m_Tools.IntegertoStringConvert(m_RcvdPacketHeader.getFrameNumber())
					+ " PacketNumber. : " + m_Tools.IntegertoStringConvert(m_RcvdPacketHeader.getPacketNumber()));
			}
			else if (bMiniPacket)
			{
				int iNumberOfPackets = m_RcvdPacketHeader.getNumberOfPacket();
				CLogPrinter_WriteSpecific(CLogPrinter::DEBUGS, "CVideoCallSession::Minipacket: FrameNumber: " + m_Tools.IntegertoStringConvert(m_RcvdPacketHeader.getFrameNumber())
					+ " PacketNumber. : " + m_Tools.IntegertoStringConvert(m_RcvdPacketHeader.getPacketNumber()));
				bIsMiniPacket = true;
			}
#endif
			int CurrentPacketType = NORMAL_PACKET_TYPE;
			if (bIsMiniPacket)
				CurrentPacketType = MINI_PACKET_TYPE;
			else if (bRetransmitted)
				CurrentPacketType = RETRANSMITTED_PACKET_TYPE;

			m_pEncodedFrameDepacketizer->Depacketize(m_PacketToBeMerged, frameSize, CurrentPacketType, m_RcvdPacketHeader);
			toolsObject.SOSleep(1);
		}
	}

	bDepacketizationThreadClosed = true;

	CLogPrinter_Write(CLogPrinter::DEBUGS, "CVideoCallSession::DepacketizationThreadProcedure() Stopped DepacketizationThreadProcedure method.");
*/
}


void CVideoCallSession::StopDecodingThread()
{
	m_pVideoDecodingThread->StopDecodingThread();

	/*
	//if (pDepacketizationThread.get())
	{
	bDecodingThreadRunning = false;

	while (!bDecodingThreadClosed)
	{
	m_Tools.SOSleep(5);
	}
	}
	//pDepacketizationThread.reset();
	*/
}

void CVideoCallSession::StartDecodingThread()
{

	m_pVideoDecodingThread->StartDecodingThread();

	/*
	CLogPrinter_Write(CLogPrinter::INFO, "CVideoCallSession::StartDepacketizationThread 1");

	if (pDecodingThread.get())
	{
	//		CLogPrinter_Write(CLogPrinter::INFO, "CVideoCallSession::StartDepacketizationThread 2");
	pDecodingThread.reset();
	//		CLogPrinter_Write(CLogPrinter::INFO, "CVideoCallSession::StartDepacketizationThread 3");
	return;
	}

	//	CLogPrinter_Write(CLogPrinter::INFO, "CVideoCallSession::StartDepacketizationThread 4");

	bDecodingThreadRunning = true;
	bDecodingThreadClosed = false;

	//	CLogPrinter_Write(CLogPrinter::INFO, "CVideoCallSession::StartDepacketizationThread 5");

	#if defined(TARGET_OS_IPHONE) || defined(TARGET_IPHONE_SIMULATOR)

	dispatch_queue_t PacketizationThreadQ = dispatch_queue_create("PacketizationThreadQ", DISPATCH_QUEUE_CONCURRENT);
	dispatch_async(PacketizationThreadQ, ^{
	this->DecodingThreadProcedure();
	});

	#else

	std::thread myThread(CreateDecodingThread, this);
	myThread.detach();

	#endif

	CLogPrinter_Write(CLogPrinter::INFO, "CVideoCallSession::StartDepacketizationThread Decoding Thread started");

	return;

	*/
}

void *CVideoCallSession::CreateDecodingThread(void* param)
{
	/*
	CVideoCallSession *pThis = (CVideoCallSession*)param;
	pThis->DecodingThreadProcedure();
	*/
	return NULL;
}

void CVideoCallSession::DecodingThreadProcedure()
{
	/*
	CLogPrinter_Write(CLogPrinter::DEBUGS, "CVideoCallSession::DepacketizationThreadProcedure() Started DepacketizationThreadProcedure method.");
	Tools toolsObject;

	int frameSize, nFrameNumber, intervalTime, nFrameLength, nEncodingTime;
	unsigned int nTimeStampDiff = 0;
	long long nTimeStampBeforeDecoding, nFirstFrameDecodingTime, nFirstFrameEncodingTime, currentTime, nShiftedTime;
	long long nMaxDecodingTime = 0;
	int RenderFaildCounter = 0;
	int nExpectedTime;

	int nDecodingStatus, fps = -1;
	double dbAverageDecodingTime = 0, dbTotalDecodingTime = 0;
	int nOponnentFPS, nMaxProcessableByMine;
	m_iDecodedFrameCounter = 0;

	nFirstFrameDecodingTime = -1;
	nExpectedTime = -1;
	long long maxDecodingTime = 0, framCounter = 0, decodingTime, nBeforeDecodingTime;
	double decodingTimeAverage = 0;

	while (bDecodingThreadRunning)
	{


	currentTime = toolsObject.CurrentTimestamp();
	if (-1 != nFirstFrameDecodingTime)
	nExpectedTime = currentTime - nShiftedTime;


	nFrameLength = m_pEncodedFrameDepacketizer->GetReceivedFrame(m_PacketizedFrame, nFrameNumber, nEncodingTime, nExpectedTime, 0);
	//printf("FrameLength:  %d\n", nFrameLength);

	decodingTime = toolsObject.CurrentTimestamp() - currentTime;

	if (nFrameLength>-1)
	CLogPrinter_WriteSpecific(CLogPrinter::DEBUGS, " GetReceivedFrame # Get Time: " + m_Tools.IntegertoStringConvert(decodingTime) + "  Len: " + m_Tools.IntegertoStringConvert(nFrameLength) + "  FrameNo: " + m_Tools.IntegertoStringConvert(nFrameNumber));


	if (-1 == nFrameLength) {
	toolsObject.SOSleep(10);
	}
	else
	{
	nBeforeDecodingTime = toolsObject.CurrentTimestamp();
	if (-1 == nFirstFrameDecodingTime)
	nTimeStampBeforeDecoding = nBeforeDecodingTime;

	nOponnentFPS = g_FPSController.GetOpponentFPS();
	nMaxProcessableByMine = g_FPSController.GetMaxOwnProcessableFPS();

	if (nOponnentFPS > 1 + nMaxProcessableByMine && (nFrameNumber & 7) > 3) {
	CLogPrinter_WriteSpecific(CLogPrinter::DEBUGS, "Force:: Frame: " + m_Tools.IntegertoStringConvert(nFrameNumber) + "  FPS: " + m_Tools.IntegertoStringConvert(nOponnentFPS) + " ~" + toolsObject.IntegertoStringConvert(nMaxProcessableByMine));
	toolsObject.SOSleep(5);
	continue;
	}


	//	if(nFrameNumber<200)
	//	{
	//	string str = "/Decode/" + m_Tools.IntegertoStringConvert(nFrameNumber) + "_" + m_Tools.IntegertoStringConvert(nFrameLength);
	//	str+=".dump";
	//	[[Helper_IOS GetInstance] WriteToFile:str.c_str() withData:m_PacketizedFrame dataLength:nFrameLength];
	//	}



	nDecodingStatus = DecodeAndSendToClient(m_PacketizedFrame, nFrameLength, nFrameNumber, nTimeStampDiff);
	//printf("decode:  %d, nDecodingStatus %d\n", nFrameNumber, nDecodingStatus);
	//			toolsObject.SOSleep(100);

	if (nDecodingStatus > 0) {
	decodingTime = toolsObject.CurrentTimestamp() - nBeforeDecodingTime;
	dbTotalDecodingTime += decodingTime;
	++m_iDecodedFrameCounter;
	nMaxDecodingTime = max(nMaxDecodingTime, decodingTime);
	if (0 == (m_iDecodedFrameCounter & 3))
	{
	dbAverageDecodingTime = dbTotalDecodingTime / m_iDecodedFrameCounter;
	dbAverageDecodingTime *= 1.5;
	fps = 1000 / dbAverageDecodingTime;

	if (fps<FPS_MAXIMUM)
	g_FPSController.SetMaxOwnProcessableFPS(fps);
	}
	CLogPrinter_WriteSpecific(CLogPrinter::DEBUGS, "Force:: AVG Decoding Time:" + m_Tools.DoubleToString(dbAverageDecodingTime) + "  Max Decoding-time: " + m_Tools.IntegertoStringConvert(nMaxDecodingTime) + "  MaxOwnProcessable: " + m_Tools.IntegertoStringConvert(fps));
	}

	if (-1 == nFirstFrameDecodingTime)
	{
	nFirstFrameDecodingTime = nTimeStampBeforeDecoding;
	nFirstFrameEncodingTime = nEncodingTime;
	nShiftedTime = nFirstFrameDecodingTime - nEncodingTime;
	}

	toolsObject.SOSleep(5);
	}
	}

	bDecodingThreadClosed = true;

	CLogPrinter_Write(CLogPrinter::DEBUGS, "CVideoCallSession::DepacketizationThreadProcedure() Stopped DepacketizationThreadProcedure method.");
	*/
}

CEncodedFrameDepacketizer * CVideoCallSession::GetEncodedFrameDepacketizer()
{
	return m_pEncodedFrameDepacketizer;
}

void CVideoCallSession::UpdateExpectedFramePacketPair(pair<int, int> currentFramePacketPair, int iNumberOfPackets)
{
/*
	int iFrameNumber = currentFramePacketPair.first;
	int iPackeNumber = currentFramePacketPair.second;
	if (iPackeNumber == iNumberOfPackets - 1)//Last Packet In a Frame
	{
		iNumberOfPacketsInCurrentFrame = 1;//next frame has at least 1 packet, it will be updated when a packet is received
		ExpectedFramePacketPair.first = iFrameNumber + 1;
		ExpectedFramePacketPair.second = 0;
	}
	else
	{
		iNumberOfPacketsInCurrentFrame = iNumberOfPackets;
		ExpectedFramePacketPair.first = iFrameNumber;
		ExpectedFramePacketPair.second = iPackeNumber + 1;
	}

	//CLogPrinter_WriteSpecific(CLogPrinter::DEBUGS, "CController::UpdateExpectedFramePacketPair: ExFrameNumber: "+ m_Tools.IntegertoStringConvert(ExpectedFramePacketPair.first) + " ExPacketNo. : "+  m_Tools.IntegertoStringConvert(ExpectedFramePacketPair.second)+ " ExNumberOfPacket : "+  m_Tools.IntegertoStringConvert(iNumberOfPacketsInCurrentFrame));
*/
}

void CVideoCallSession::CreateAndSendMiniPacket(int resendFrameNumber, int resendPacketNumber)
{
	unsigned char uchVersion = g_uchSendPacketVersion;

	//    if(INVALID_PACKET_NUMBER !=resendPacketNumber && resendFrameNumber % I_INTRA_PERIOD != 0 ) //
	if (INVALID_PACKET_NUMBER != resendPacketNumber) //
	{
		return;
	}

	int numberOfPackets = 1000; //dummy numberOfPackets

	CPacketHeader PacketHeader;
	if (resendPacketNumber == INVALID_PACKET_NUMBER) {
		//m_miniPacketBandCounter++;
		if (0 == uchVersion) return;

		PacketHeader.setPacketHeader(uchVersion, m_miniPacketBandCounter/*SlotID*/, 0, resendPacketNumber/*Invalid_Packet*/, resendFrameNumber/*BandWidth*/, 0, 0, 0);
	}
	else {
		PacketHeader.setPacketHeader(uchVersion, resendFrameNumber, numberOfPackets, resendPacketNumber, 0, 0, 0, 0);
		g_timeInt.setTime(resendFrameNumber, resendPacketNumber);
	}

	m_miniPacket[0] = (int)VIDEO_PACKET_MEDIA_TYPE;

	PacketHeader.GetHeaderInByteArray(m_miniPacket + 1);

	m_miniPacket[RETRANSMISSION_SIG_BYTE_INDEX_WITHOUT_MEDIA + 1] |= 1 << BIT_INDEX_MINI_PACKET; //MiniPacket Flag

	if (uchVersion)
		m_pCommonElementsBucket->SendFunctionPointer(friendID, 2, m_miniPacket, PACKET_HEADER_LENGTH + 1);
	else
		m_pCommonElementsBucket->SendFunctionPointer(friendID, 2, m_miniPacket, PACKET_HEADER_LENGTH_NO_VERSION + 1);

	//m_SendingBuffer.Queue(frameNumber, miniPacket, PACKET_HEADER_LENGTH_WITH_MEDIA_TYPE);
}






















void CVideoCallSession::StopRenderingThread()
{
	m_pVideoRenderingThread->StopRenderingThread();

	/*
	//if (pInternalThread.get())
	{

	bRenderingThreadRunning = false;

	while (!bRenderingThreadClosed)
	{
	m_Tools.SOSleep(5);
	}
	}

	//pInternalThread.reset();
	*/
}

void CVideoCallSession::StartRenderingThread()
{
	m_pVideoRenderingThread->StartRenderingThread();

	/*
	if (pRenderingThread.get())
	{
	pRenderingThread.reset();
	return;
	}

	bRenderingThreadRunning = true;
	bRenderingThreadClosed = false;

	#if defined(TARGET_OS_IPHONE) || defined(TARGET_IPHONE_SIMULATOR)

	dispatch_queue_t RenderThreadQ = dispatch_queue_create("RenderThreadQ",DISPATCH_QUEUE_CONCURRENT);
	dispatch_async(RenderThreadQ, ^{
	this->RenderingThreadProcedure();
	});

	#else

	std::thread myThread(CreateVideoRenderingThread, this);
	myThread.detach();

	#endif

	CLogPrinter_Write(CLogPrinter::INFO, "CVideoCallSession::StartRenderingThread Rendering Thread started");

	return;
	*/
}

void *CVideoCallSession::CreateVideoRenderingThread(void* param)
{
	/*
	CVideoCallSession *pThis = (CVideoCallSession*)param;
	pThis->RenderingThreadProcedure();
	*/
	return NULL;
}

void CVideoCallSession::RenderingThreadProcedure()
{
	/*
	CLogPrinter_Write(CLogPrinter::DEBUGS, "CVideoCallSession::RenderingThreadProcedure() Started EncodingThreadProcedure.");
	Tools toolsObject;
	int frameSize,nFrameNumber,intervalTime;
	unsigned int nTimeStampDiff;
	long long currentFrameTime,decodingTime,firstFrameEncodingTime;
	int videoHeight, videoWidth;
	long long currentTimeStamp;
	int prevTimeStamp=0;
	int minTimeGap = 51;

	while (bRenderingThreadRunning)
	{
	//CLogPrinter_Write(CLogPrinter::INFO, "CVideoCallSession::RenderingThreadProcedure");

	if (m_RenderingBuffer->GetQueueSize() == 0)
	toolsObject.SOSleep(10);
	else
	{

	int timeDiffForQueue;

	frameSize = m_RenderingBuffer->DeQueue(nFrameNumber, nTimeStampDiff, m_RenderingFrame, videoHeight, videoWidth, timeDiffForQueue);
	CLogPrinter_WriteForQueueTime(CLogPrinter::DEBUGS, " m_RenderingBuffer "+ toolsObject.IntegertoStringConvert(timeDiffForQueue));

	currentFrameTime = toolsObject.CurrentTimestamp();

	if(m_b1stDecodedFrame)
	{
	m_ll1stDecodedFrameTimeStamp = currentFrameTime;
	firstFrameEncodingTime = nTimeStampDiff;
	m_b1stDecodedFrame = false;
	}
	else
	{
	minTimeGap = nTimeStampDiff - prevTimeStamp ;
	}

	prevTimeStamp = nTimeStampDiff;

	if(frameSize<1 && minTimeGap < 50)
	continue;

	toolsObject.SOSleep(5);

	m_pCommonElementsBucket->m_pEventNotifier->fireVideoEvent(friendID, nFrameNumber, frameSize, m_RenderingFrame, videoHeight, videoWidth);
	}
	}

	bRenderingThreadClosed = true;

	CLogPrinter_Write(CLogPrinter::DEBUGS, "CVideoCallSession::RenderingThreadProcedure() Stopped EncodingThreadProcedure");
	*/
}

int CVideoCallSession::GetUniquePacketID(int fn, int pn)
{
	return fn*MAX_PACKET_NUMBER + pn;
}

int CVideoCallSession::NeedToChangeBitRate(double dataReceivedRatio)
{
	m_SlotCounter++;

	if (dataReceivedRatio < NORMAL_BITRATE_RATIO_IN_MEGA_SLOT)
	{
		m_iGoodSlotCounter = 0;
		m_iNormalSlotCounter = 0;
		m_SlotCounter = 0;

		//m_iConsecutiveGoodMegaSlot = 0;
		m_PrevMegaSlotStatus = dataReceivedRatio;
		return BITRATE_CHANGE_DOWN;

	}
	else if (dataReceivedRatio >= NORMAL_BITRATE_RATIO_IN_MEGA_SLOT && dataReceivedRatio <= GOOD_BITRATE_RATIO_IN_MEGA_SLOT)
	{
		m_iNormalSlotCounter++;
	}
	else if (dataReceivedRatio > GOOD_BITRATE_RATIO_IN_MEGA_SLOT)
	{
		m_iGoodSlotCounter++;
		/*m_iConsecutiveGoodMegaSlot++;
		if(m_iConsecutiveGoodMegaSlot == GOOD_MEGASLOT_TO_UP)
		{
		m_iConsecutiveGoodMegaSlot = 0;
		return BITRATE_CHANGE_UP;
		}*/


	}
	else
	{
		//      m_iConsecutiveGoodMegaSlot = 0;
	}


	if (m_SlotCounter >= GOOD_MEGASLOT_TO_UP)
	{
		int temp = GOOD_MEGASLOT_TO_UP * 0.9;

		if (m_iGoodSlotCounter >= temp && m_PrevMegaSlotStatus>GOOD_BITRATE_RATIO_IN_MEGA_SLOT)
		{
			m_iGoodSlotCounter = 0;
			m_iNormalSlotCounter = 0;
			m_SlotCounter = 0;

			m_PrevMegaSlotStatus = dataReceivedRatio;

			return BITRATE_CHANGE_UP;
		}

	}


	m_PrevMegaSlotStatus = dataReceivedRatio;
	return BITRATE_CHANGE_NO;
}








