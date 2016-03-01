
#include "DepacketizationThread.h"
#include "Size.h"
#include "CommonElementsBucket.h"

int g_iPacketCounterSinceNotifying = FPS_SIGNAL_IDLE_FOR_PACKETS;			// bring
bool gbStopFPSSending = false;												// bring
extern unsigned char g_uchSendPacketVersion;								// bring check

CVideoDepacketizationThread::CVideoDepacketizationThread(LongLong friendID, CVideoPacketQueue *VideoPacketQueue, CVideoPacketQueue *RetransVideoPacketQueue, CVideoPacketQueue *MiniPacketQueue, BitRateController *BitRateController, CEncodedFrameDepacketizer *EncodedFrameDepacketizer, CCommonElementsBucket* CommonElementsBucket, int *miniPacketBandCounter) :

m_FriendID(friendID),
m_pVideoPacketQueue(VideoPacketQueue),
m_pRetransVideoPacketQueue(RetransVideoPacketQueue),
m_pMiniPacketQueue(MiniPacketQueue),
m_BitRateController(BitRateController),
m_pEncodedFrameDepacketizer(EncodedFrameDepacketizer),
m_pCommonElementsBucket(CommonElementsBucket),
m_miniPacketBandCounter(miniPacketBandCounter)

{
	g_iPacketCounterSinceNotifying = FPS_SIGNAL_IDLE_FOR_PACKETS;
	gbStopFPSSending = false;

	ExpectedFramePacketPair.first = 0;
	ExpectedFramePacketPair.second = 0;

	iNumberOfPacketsInCurrentFrame = 0;
}

CVideoDepacketizationThread::~CVideoDepacketizationThread()
{

}

void CVideoDepacketizationThread::StopDepacketizationThread()
{

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
}

void CVideoDepacketizationThread::StartDepacketizationThread()
{
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
}

void *CVideoDepacketizationThread::CreateVideoDepacketizationThread(void* param)
{
	CVideoDepacketizationThread *pThis = (CVideoDepacketizationThread*)param;
	pThis->DepacketizationThreadProcedure();

	return NULL;
}

void CVideoDepacketizationThread::DepacketizationThreadProcedure()		//Merging Thread
{
	CLogPrinter_Write(CLogPrinter::DEBUGS, "CVideoCallSession::DepacketizationThreadProcedure() Started DepacketizationThreadProcedure method.");
	Tools toolsObject;
	unsigned char temp;
	int frameSize, queSize = 0, retQueuSize = 0, miniPacketQueueSize = 0, consicutiveRetransmittedPkt = 0;
	int frameNumber, packetNumber;
	bool bIsMiniPacket;
	//m_iCountRecResPack = 0;			// unnecessary removed
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
			g_iPacketCounterSinceNotifying++;
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
					/*
					string sMsg = "CVideoCallSession::Current(FN,PN) = ("
					+ m_Tools.IntegertoStringConvert(currentFramePacketPair.first)
					+ ","
					+ m_Tools.IntegertoStringConvert(currentFramePacketPair.second)
					+ ") and Expected(FN,PN) = ("
					+ m_Tools.IntegertoStringConvert(ExpectedFramePacketPair.first)
					+ ","
					+ m_Tools.IntegertoStringConvert(ExpectedFramePacketPair.second)
					+ ")" ;
					printf("%s\n", sMsg.c_str());
					*/

					if (g_iPacketCounterSinceNotifying >= FPS_SIGNAL_IDLE_FOR_PACKETS)
					{
						//						g_FPSController.NotifyFrameDropped(currentFramePacketPair.first);
						g_iPacketCounterSinceNotifying = 0;
						gbStopFPSSending = false;
					}
					else
					{
						gbStopFPSSending = true;
					}


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
								if (iSendCounter /*&& requestFramePacketPair.first %8 ==0*/) m_Tools.SOSleep(1);
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
								if (iSendCounter /*&& requestFramePacketPair.first %8 ==0*/) m_Tools.SOSleep(1);
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
								if (iSendCounter /*&& requestFramePacketPair.first %8 ==0*/) m_Tools.SOSleep(1);
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
									if (iSendCounter /*&& requestFramePacketPair.first %8 ==0*/) m_Tools.SOSleep(1);
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
							if (iSendCounter /* && requestFramePacketPair.first %8 ==0*/) m_Tools.SOSleep(1);
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
}

void CVideoDepacketizationThread::CreateAndSendMiniPacket(int resendFrameNumber, int resendPacketNumber)
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

		PacketHeader.setPacketHeader(uchVersion, *m_miniPacketBandCounter/*SlotID*/, 0, resendPacketNumber/*Invalid_Packet*/, resendFrameNumber/*BandWidth*/, 0, 0, 0);
	}
	else {
		PacketHeader.setPacketHeader(uchVersion, resendFrameNumber, numberOfPackets, resendPacketNumber, 0, 0, 0, 0);
		g_timeInt.setTime(resendFrameNumber, resendPacketNumber);
	}

	m_miniPacket[0] = (int)VIDEO_PACKET_MEDIA_TYPE;

	PacketHeader.GetHeaderInByteArray(m_miniPacket + 1);

	m_miniPacket[RETRANSMISSION_SIG_BYTE_INDEX_WITHOUT_MEDIA + 1] |= 1 << BIT_INDEX_MINI_PACKET; //MiniPacket Flag

	if (uchVersion)
		m_pCommonElementsBucket->SendFunctionPointer(m_FriendID, 2, m_miniPacket, PACKET_HEADER_LENGTH + 1);
	else
		m_pCommonElementsBucket->SendFunctionPointer(m_FriendID, 2, m_miniPacket, PACKET_HEADER_LENGTH_NO_VERSION + 1);

	//m_SendingBuffer.Queue(frameNumber, miniPacket, PACKET_HEADER_LENGTH_WITH_MEDIA_TYPE);
}

void CVideoDepacketizationThread::UpdateExpectedFramePacketPair(pair<int, int> currentFramePacketPair, int iNumberOfPackets)
{
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
}