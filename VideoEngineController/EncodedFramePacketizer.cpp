#include "EncodedFramePacketizer.h"
#include "CommonElementsBucket.h"
#include "LogPrinter.h"
#include "Tools.h"
#include "VideoPacketBuffer.h"
#include "ResendingBuffer.h"
#include "Globals.h"
#include "BandwidthController.h"

#if defined(TARGET_OS_IPHONE) || defined(TARGET_IPHONE_SIMULATOR)
#include <dispatch/dispatch.h>
#endif

extern bool g_bIsVersionDetectableOpponent;
extern unsigned char g_uchSendPacketVersion;
extern int g_uchOpponentVersion;
BandwidthController g_BandWidthController;

extern deque<pair<int, int>> ExpectedFramePacketDeQueue;
extern CResendingBuffer g_ResendBuffer;
extern CFPSController g_FPSController;

CEncodedFramePacketizer::CEncodedFramePacketizer(CCommonElementsBucket* sharedObject, CSendingBuffer* pSendingBuffer) :
m_PacketSize(MAX_PACKET_SIZE_WITHOUT_HEADER),
m_pCommonElementsBucket(sharedObject)
{
	CLogPrinter_Write(CLogPrinter::INFO, "CEncodedFramePacketizer::CEncodedFramePacketizer");

	m_pEncodedFrameParsingMutex.reset(new CLockHandler);
	m_pEncodedFrameParsingThread = NULL;

	m_SendingBuffer = pSendingBuffer;
//	m_SendingBuffer = new CSendingBuffer();
//	m_pSendingThread = new CSendingThread(sharedObject, m_SendingBuffer, &g_FPSController);

//	StartSendingThread();

	CLogPrinter_Write(CLogPrinter::DEBUGS, "CEncodedFramePacketizer::CEncodedFramePacketizer Created");
}

CEncodedFramePacketizer::~CEncodedFramePacketizer()
{
	/*	if (m_pEncodedFrameParsingThread)
	{
	delete m_pEncodedFrameParsingThread;
	m_pEncodedFrameParsingThread = NULL;
	}*/
//
//	StopSendingThread();

//	if (NULL != m_pSendingThread)
//	{
//		delete m_pSendingThread;
//		m_pSendingThread = NULL;
//	}

//	if (NULL != m_SendingBuffer)
//	{
//		delete m_SendingBuffer;
//		m_SendingBuffer = NULL;
//	}

	SHARED_PTR_DELETE(m_pEncodedFrameParsingMutex);
}

int CEncodedFramePacketizer::Packetize(LongLong lFriendID, unsigned char *in_data, unsigned int in_size, int frameNumber,
	unsigned int iTimeStampDiff)
{
	CLogPrinter_Write(CLogPrinter::DEBUGS, "CEncodedFramePacketizer::Packetize parsing started");

	unsigned char uchOpponentVersion = g_uchSendPacketVersion;

	int nHeaderLenWithoutMedia = PACKET_HEADER_LENGTH_NO_VERSION;

	if (uchOpponentVersion)
		nHeaderLenWithoutMedia = PACKET_HEADER_LENGTH;

	int nPacketHeaderLenghtWithMedia = nHeaderLenWithoutMedia + 1;


	m_PacketSize = MAX_VIDEO_PACKET_SIZE - nPacketHeaderLenghtWithMedia;
	int packetizedSize = m_PacketSize;

	int readPacketLength = 0;

	int numberOfPackets = (in_size + m_PacketSize - 1) / m_PacketSize;

	if (numberOfPackets > MAX_NUMBER_OF_PACKETS)
		return -1;

	CLogPrinter_Write(CLogPrinter::INFO, "CEncodedFramePacketizer::Packetize in_size " + m_Tools.IntegertoStringConvert(in_size) + " m_PacketSize " + m_Tools.IntegertoStringConvert(m_PacketSize));

	for (int packetNumber = 0; readPacketLength < in_size; packetNumber++, readPacketLength += m_PacketSize)
	{
		if (m_PacketSize + readPacketLength > in_size)
			m_PacketSize = in_size - readPacketLength;

		//m_PacketHeader.setPacketHeader(uchOpponentVersion, frameNumber, numberOfPackets, packetNumber, iTimeStampDiff, 0, 0, m_PacketSize + nPacketHeaderLenghtWithMedia);

		m_PacketHeader.setPacketHeader(uchOpponentVersion, frameNumber, numberOfPackets, packetNumber, iTimeStampDiff, 0, 0, uchOpponentVersion ? m_PacketSize + nPacketHeaderLenghtWithMedia : m_PacketSize);


		m_PacketHeader.GetHeaderInByteArray(m_Packet + 1);

		m_Packet[0] = VIDEO_PACKET_MEDIA_TYPE;
		memcpy(m_Packet + nPacketHeaderLenghtWithMedia, in_data + readPacketLength, m_PacketSize);

		//m_pCommonElementsBucket->m_pEventNotifier->firePacketEvent(m_pCommonElementsBucket->m_pEventNotifier->ENCODED_PACKET, frameNumber, numberOfPackets, packetNumber, m_PacketSize, nPacketHeaderLenghtWithMedia + m_PacketSize, m_Packet);

		//		m_PacketHeader.setPacketHeader(m_Packet+1);
		//		CLogPrinter_WriteSpecific2(CLogPrinter::INFO, "$$--> Lenght "+m_Tools.IntegertoStringConvert(m_PacketHeader.getPacketLength())+"  # TS: "+ m_Tools.IntegertoStringConvert(m_PacketHeader.getTimeStamp()));

//		CLogPrinter_WriteInstentTestLog(CLogPrinter::INFO, "CEncodedFramePacketizer::Packetize Queue lFriendID " + Tools::IntegertoStringConvert(lFriendID) + " packetSize " + Tools::IntegertoStringConvert(nPacketHeaderLenghtWithMedia + m_PacketSize));
		m_SendingBuffer->Queue(lFriendID, m_Packet, nPacketHeaderLenghtWithMedia + m_PacketSize, frameNumber, packetNumber);
		g_ResendBuffer.Queue(m_Packet, nPacketHeaderLenghtWithMedia + m_PacketSize, frameNumber, packetNumber);//enqueue(pchPacketToResend);
		//    m_pCommonElementsBucket->SendFunctionPointer(lFriendID,2,m_Packet,nPacketHeaderLenghtWithMedia + m_PacketSize);
	}

	return 1;
}

void *CEncodedFramePacketizer::CreateEncodedFrameParsingThread(void* param)
{
	CEncodedFramePacketizer *pThis = (CEncodedFramePacketizer*)param;

	return NULL;
}

void CEncodedFramePacketizer::StartEncodedFrameParsingThread()
{
	std::thread t(CreateEncodedFrameParsingThread, this);

	t.detach();

	return;
}

void CEncodedFramePacketizer::StopEncodedFrameParsingThread()
{
	if (m_pEncodedFrameParsingThread != NULL)
	{
		Locker lock(*m_pEncodedFrameParsingMutex);

		/*delete m_pEncodedFrameParsingThread;

		m_pEncodedFrameParsingThread = NULL;*/
	}

}









/*
void CEncodedFramePacketizer::StopSendingThread()
{
	m_pSendingThread->StopSendingThread();


	//if (pInternalThread.get())
	{
	bSendingThreadRunning = false;

	while (!bSendingThreadClosed)
	m_Tools.SOSleep(5);
	}

	//pInternalThread.reset();

}
*/

/*
void CEncodedFramePacketizer::StartSendingThread()
{
	m_pSendingThread->StartSendingThread();


//	CLogPrinter_WriteInstentTestLog(CLogPrinter::INFO, "CEncodedFramePacketizer::StartedInternalThread 1");

	if (pSendingThread.get())
	{
//	CLogPrinter_WriteInstentTestLog(CLogPrinter::INFO, "CEncodedFramePacketizer::StartedInternalThread 2");
	pSendingThread.reset();
//	CLogPrinter_WriteInstentTestLog(CLogPrinter::INFO, "CEncodedFramePacketizer::StartDecodingThread 3");
	return;
	}
//	CLogPrinter_WriteInstentTestLog(CLogPrinter::INFO, "CEncodedFramePacketizer::StartedInternalThread 4");
	bSendingThreadRunning = true;
	bSendingThreadClosed = false;
//	CLogPrinter_WriteInstentTestLog(CLogPrinter::INFO, "CEncodedFramePacketizer::StartedInternalThread 5");

	#if defined(TARGET_OS_IPHONE) || defined(TARGET_IPHONE_SIMULATOR)

	dispatch_queue_t SendingThreadQ = dispatch_queue_create("SendingThreadQ",DISPATCH_QUEUE_CONCURRENT);
	dispatch_async(SendingThreadQ, ^{
	this->SendingThreadProcedure();
	});

	#else

	std::thread myThread(CreateVideoSendingThread, this);
	myThread.detach();

	#endif

//	CLogPrinter_WriteInstentTestLog(CLogPrinter::INFO, "CEncodedFramePacketizer::StartedInternalThread Encoding Thread started");

	return;

}
*/

void *CEncodedFramePacketizer::CreateVideoSendingThread(void* param)
{
	/*
	CEncodedFramePacketizer *pThis = (CEncodedFramePacketizer*)param;
	pThis->SendingThreadProcedure();
	*/
	return NULL;
}

void CEncodedFramePacketizer::SendingThreadProcedure()
{
	/*
//	CLogPrinter_WriteInstentTestLog(CLogPrinter::DEBUGS, "CEncodedFramePacketizer::EncodingThreadProcedure() Started EncodingThreadProcedure.");

	Tools toolsObject;
	int packetSize;
	LongLong lFriendID;
	int startFraction = SIZE_OF_INT_MINUS_8;
	int fractionInterval = BYTE_SIZE;
	int fpsSignal, frameNumber, packetNumber;
	CPacketHeader packetHeader;

	#ifdef  BANDWIDTH_CONTROLLING_TEST
	g_BandWidthList.push_back(500*1024);    g_TimePeriodInterval.push_back(20*1000);
	g_BandWidthList.push_back(8*1024);    g_TimePeriodInterval.push_back(20*1000);
	g_BandWidthList.push_back(3*1024);    g_TimePeriodInterval.push_back(100*1000);
	g_BandWidthList.push_back(5*1024);    g_TimePeriodInterval.push_back(2*1000);
	g_BandWidthList.push_back(500*1024);    g_TimePeriodInterval.push_back(20*1000);
	g_BandWidthList.push_back(5*1024);    g_TimePeriodInterval.push_back(2*1000);
	g_BandWidthList.push_back(500*1024);    g_TimePeriodInterval.push_back(20*1000);
	g_BandWidthList.push_back(5*1024);    g_TimePeriodInterval.push_back(2*1000);
	g_BandWidthList.push_back(500*1024);    g_TimePeriodInterval.push_back(20*1000);
	g_BandWidthList.push_back(5*1024);    g_TimePeriodInterval.push_back(2*1000);
	g_BandWidthList.push_back(500*1024);    g_TimePeriodInterval.push_back(20*1000);
	g_BandWidthList.push_back(5*1024);    g_TimePeriodInterval.push_back(2*1000);
	g_BandWidthList.push_back(500*1024);    g_TimePeriodInterval.push_back(20*1000);
	g_BandWidthList.push_back(5*1024);    g_TimePeriodInterval.push_back(2*1000);
	g_BandWidthList.push_back(500*1024);    g_TimePeriodInterval.push_back(20*1000);
	g_BandWidthList.push_back(5*1024);    g_TimePeriodInterval.push_back(2*1000);
	g_BandWidthController.SetTimeInterval(g_BandWidthList,g_TimePeriodInterval);
	#endif

	while (bSendingThreadRunning)
	{
//	CLogPrinter_WriteInstentTestLog(CLogPrinter::INFO, "CEncodedFramePacketizer::SendingThreadProcedure");

	if (m_SendingBuffer->GetQueueSize() == 0)
	toolsObject.SOSleep(10);
	else
	{
	int timeDiffForQueue;

	packetSize = m_SendingBuffer->DeQueue(lFriendID, m_EncodedFrame, frameNumber, packetNumber, timeDiffForQueue);

//	CLogPrinter_WriteInstentTestLog(CLogPrinter::INFO, "CEncodedFramePacketizer::SendingThreadProcedure Deque lFriendID " + Tools::IntegertoStringConvert(lFriendID) + " packetSize " + Tools::IntegertoStringConvert(packetSize));

	CLogPrinter_WriteForQueueTime(CLogPrinter::INFO, " m_SendingBuffer " + toolsObject.IntegertoStringConvert(timeDiffForQueue));

	int startPoint = RESEND_INFO_START_BYTE_WITH_MEDIA_TYPE;
	pair<int,int> FramePacketToSend = {-1, -1};


	packetHeader.setPacketHeader(m_EncodedFrame+1);

	//		if(ExpectedFramePacketDeQueue.size() > 0)
	//		{
	//			FramePacketToSend = ExpectedFramePacketDeQueue.front();
	//			ExpectedFramePacketDeQueue.pop_front();
	//		}
	#ifdef	RETRANSMISSION_ENABLED
	//		for (int f = startFraction; f >= 0; f -= fractionInterval)//ResendFrameNumber
	//		{
	//			m_EncodedFrame[startPoint ++] = (FramePacketToSend.first >> f) & 0xFF;
	//		}
	//		for (int f = startFraction; f >= 0; f -= fractionInterval)//ResendPacketNumber
	//		{
	//			m_EncodedFrame[startPoint ++] = (FramePacketToSend.second >> f) & 0xFF;
	//		}
	#endif
	//			CLogPrinter_WriteSpecific(CLogPrinter::DEBUGS, " Before Bye SIGBYTE: ");

	unsigned char signal = g_FPSController.GetFPSSignalByte();
	m_EncodedFrame[ 1 + SIGNAL_BYTE_INDEX_WITHOUT_MEDIA] = signal;

	//			CLogPrinter_WriteSpecific(CLogPrinter::DEBUGS, " Bye SIGBYTE: "+ m_Tools.IntegertoStringConvert(signal));



	#ifdef PACKET_SEND_STATISTICS_ENABLED

	int iNumberOfPackets = -1;

	iNumberOfPackets = packetHeader.getNumberOfPacket();

	//   pair<int, int> FramePacketPair = toolsObject.GetFramePacketFromHeader(m_EncodedFrame + 1, iNumberOfPackets);
	pair<int, int> FramePacketPair = make_pair(packetHeader.getFrameNumber(), packetHeader.getPacketNumber());

	if (FramePacketPair.first != iPrevFrameNumer)
	{
	//CLogPrinter_WriteSpecific(CLogPrinter::DEBUGS,"iNumberOfPacketsActuallySentFromLastFrame = %d, iNumberOfPacketsInLastFrame = %d, currentframenumber = %d\n",
	//	iNumberOfPacketsActuallySentFromLastFrame, iNumberOfPacketsInLastFrame, FramePacketPair.first);

	if (iNumberOfPacketsActuallySentFromLastFrame != iNumberOfPacketsInLastFrame)
	{
	CLogPrinter_WriteSpecific2(CLogPrinter::INFO,"$$-->******* iNumberOfPacketsActuallySentFromLastFrame = "
	+ m_Tools.IntegertoStringConvert(iNumberOfPacketsActuallySentFromLastFrame)
	+ " iNumberOfPacketsInLastFrame = "
	+ m_Tools.IntegertoStringConvert(iNumberOfPacketsInLastFrame)
	+ " currentframenumber = "
	+ m_Tools.IntegertoStringConvert(FramePacketPair.first)
	+ " m_SendingBuffersize = "
	+ m_Tools.IntegertoStringConvert(m_SendingBuffer->GetQueueSize()));

	}


	iNumberOfPacketsInLastFrame = iNumberOfPackets;
	iNumberOfPacketsActuallySentFromLastFrame = 1;
	iPrevFrameNumer = FramePacketPair.first;
	}
	else
	{
	iNumberOfPacketsActuallySentFromLastFrame++;
	}
	#endif


	//			CLogPrinter_WriteSpecific2(CLogPrinter::INFO, "Parsing..>>>  FN: "+ m_Tools.IntegertoStringConvert(packetHeader.getFrameNumber())
	//														  + "  pNo : "+ m_Tools.IntegertoStringConvert(packetHeader.getPacketNumber())
	//														  + "  Npkt : "+ m_Tools.IntegertoStringConvert(packetHeader.getNumberOfPacket())
	//														  + "  FPS : "+ m_Tools.IntegertoStringConvert(packetHeader.getFPS())
	//														  + "  Rt : "+ m_Tools.IntegertoStringConvert(packetHeader.getRetransSignal())
	//														  + "  Len : "+ m_Tools.IntegertoStringConvert(packetHeader.getPacketLength())
	//														  + " tmDiff : " + m_Tools.IntegertoStringConvert(packetHeader.getTimeStamp()));


	#ifdef  BANDWIDTH_CONTROLLING_TEST
	if(g_BandWidthController.IsSendeablePacket(packetSize)) {
	#endif

	//printf("WIND--> SendFunctionPointer with size  = %d\n", packetSize);
//	CLogPrinter_WriteInstentTestLog(CLogPrinter::INFO, "CEncodedFramePacketizer::SendingThreadProcedure lFriendID " + Tools::IntegertoStringConvert(lFriendID) + " packetSize " + Tools::IntegertoStringConvert(packetSize));
	m_pCommonElementsBucket->SendFunctionPointer(lFriendID, 2, m_EncodedFrame, packetSize);
//	CLogPrinter_WriteInstentTestLog(CLogPrinter::INFO, "CEncodedFramePacketizer::SendingThreadProcedure sent");
	CLogPrinter_WriteForPacketLossInfo(CLogPrinter::DEBUGS, " &*&*Sending frameNumber: "+ toolsObject.IntegertoStringConvert(frameNumber) + " :: PacketNo: "+ toolsObject.IntegertoStringConvert(packetNumber));

	//toolsObject.SOSleep((int)(SENDING_INTERVAL_FOR_15_FPS * MAX_FPS * 1.0) / (g_FPSController.GetOwnFPS()  * 1.0));

	toolsObject.SOSleep(GetSleepTime());

	#ifdef  BANDWIDTH_CONTROLLING_TEST
	}
	#endif

	}
	}

	bSendingThreadClosed = true;

//	CLogPrinter_WriteInstentTestLog(CLogPrinter::DEBUGS, "CEncodedFramePacketizer::EncodingThreadProcedure() Stopped EncodingThreadProcedure");
	*/
}

int CEncodedFramePacketizer::GetSleepTime()
{
	/*
	int SleepTimeDependingOnFPS = (SENDING_INTERVAL_FOR_15_FPS * FPS_MAXIMUM * 1.0) / (g_FPSController.GetOwnFPS()  * 1.0);
	int SleepTimeDependingOnQueueSize = 1000 * 1.0 / (m_SendingBuffer->GetQueueSize() + 1.0);

	if (SleepTimeDependingOnFPS < SleepTimeDependingOnQueueSize)
	{
	return SleepTimeDependingOnFPS;
	}
	else
	{
	return SleepTimeDependingOnQueueSize;
	}
	*/

	return 0;
}





