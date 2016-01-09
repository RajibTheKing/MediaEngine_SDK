#include "EncodedFrameParser.h"
#include "CommonElementsBucket.h"
#include "LogPrinter.h"
#include "Tools.h"
#include "VideoPacketBuffer.h"
#include "ResendingBuffer.h"
#include "Globals.h"

#if defined(TARGET_OS_IPHONE) || defined(TARGET_IPHONE_SIMULATOR)
#include <dispatch/dispatch.h>
#endif

//extern int isFPSChange();

extern deque<pair<int,int>> ExpectedFramePacketDeQueue;
CResendingBuffer g_ResendBuffer;
extern CFPSController g_FPSController;

CEncodedFrameParser::CEncodedFrameParser(CCommonElementsBucket* sharedObject) :
		m_PacketSize(MAX_PACKET_SIZE_WITHOUT_HEADER),
		m_pCommonElementsBucket(sharedObject)
{
	CLogPrinter::Write(CLogPrinter::INFO, "CEncodedFrameParser::CEncodedFrameParser");

	m_pEncodedFrameParsingMutex.reset(new CLockHandler);
	m_pEncodedFrameParsingThread = NULL;
    
    StartSendingThread();

	CLogPrinter::Write(CLogPrinter::DEBUGS, "CEncodedFrameParser::CEncodedFrameParser Created");
}

CEncodedFrameParser::~CEncodedFrameParser()
{
/*	if (m_pEncodedFrameParsingThread)
	{
		delete m_pEncodedFrameParsingThread;
		m_pEncodedFrameParsingThread = NULL;
	}*/
    
    StopSendingThread();

	SHARED_PTR_DELETE(m_pEncodedFrameParsingMutex);
}

int CEncodedFrameParser::ParseFrameIntoPackets(LongLong lFriendID, unsigned char *in_data, unsigned int in_size, int frameNumber,
											   unsigned int iTimeStampDiff)
{
	CLogPrinter::Write(CLogPrinter::DEBUGS, "CEncodedFrameParser::ParseFrameIntoPackets parsing started");

	int packetizedSize = m_PacketSize;

	int startFraction = SIZE_OF_INT_MINUS_8;
	int fractionInterval = BYTE_SIZE;
	int packetHeaderSize = PACKET_HEADER_LENGTH_WITH_MEDIA_TYPE;
	int readPacketLength;

	int numberOfPackets = in_size / m_PacketSize;

	CLogPrinter::Write(CLogPrinter::INFO, "CEncodedFrameParser::ParseFrameIntoPackets in_size " + m_Tools.IntegertoStringConvert(in_size) + " m_PacketSize " + m_Tools.IntegertoStringConvert(m_PacketSize));

	if (in_size % m_PacketSize != 0 )
	{
		CLogPrinter::Write(CLogPrinter::INFO, "CEncodedFrameParser::ParseFrameIntoPackets increased" );

		numberOfPackets++;
	}

	int packetNumber;

	for (packetNumber = 0, readPacketLength = 0; (unsigned int)packetizedSize <= in_size; packetNumber++, readPacketLength += m_PacketSize)
	{
		int startPoint = 1;

		if(packetNumber>MAX_NUMBER_OF_PACKETS)
			return -1;

		for (int f = startFraction; f >= 0; f -= fractionInterval)
		{
			m_Packet[startPoint++] = (frameNumber >> f) & 0xFF;
		}

		for (int f = startFraction; f >= 0; f -= fractionInterval)
		{
			m_Packet[startPoint++] = (numberOfPackets >> f) & 0xFF;
		}

		for (int f = startFraction; f >= 0; f -= fractionInterval)
		{
			m_Packet[startPoint++] = (packetNumber >> f) & 0xFF;
		}

		for (int f = startFraction; f >= 0; f -= fractionInterval)
		{
			m_Packet[startPoint++] = (m_PacketSize >> f) & 0xFF;
		}

#ifdef	RETRANSMISSION_ENABLED
		startPoint += RESEND_INFO_SIZE;

		for (int f = startFraction; f >= 0; f -= fractionInterval)
		{
			m_Packet[startPoint++] = (iTimeStampDiff >> f) & 0xFF;
		}
#endif
		startPoint = PACKET_HEADER_LENGTH+1;
		memcpy(m_Packet + startPoint, in_data + readPacketLength, m_PacketSize);
		startPoint += m_PacketSize;
		packetizedSize += m_PacketSize;

		//m_pCommonElementsBucket->m_pEventNotifier->firePacketEvent(m_pCommonElementsBucket->m_pEventNotifier->ENCODED_PACKET, frameNumber, numberOfPackets, packetNumber, m_PacketSize, packetHeaderSize + m_PacketSize, m_Packet);

		CLogPrinter::WriteSpecific(CLogPrinter::INFO, "Parsing..>>>");
        m_Packet[0] = VIDEO_PACKET_MEDIA_TYPE;

		m_SendingBuffer.Queue(lFriendID, m_Packet, packetHeaderSize + m_PacketSize);
        g_ResendBuffer.Queue(m_Packet, packetHeaderSize + m_PacketSize, frameNumber, packetNumber);//enqueue(pchPacketToResend);
    //    m_pCommonElementsBucket->SendFunctionPointer(lFriendID,2,m_Packet,packetHeaderSize + m_PacketSize);
	}

	//CLogPrinter::Write(CLogPrinter::INFO, "CEncodedFrameParser::ParseFrameIntoPackets packetSize " + m_Tools.IntegertoStringConvert(packetNumber) + " " + m_Tools.IntegertoStringConvert(numberOfPackets) + " " + m_Tools.IntegertoStringConvert(packetizedSize) + " " + m_Tools.IntegertoStringConvert(m_PacketSize) + " " + m_Tools.IntegertoStringConvert(in_size));

	if (packetNumber < numberOfPackets )
	{
		int startPoint = 1;
		int packetSize = in_size - ( packetizedSize - m_PacketSize );

		//CLogPrinter::Write(CLogPrinter::INFO, "CEncodedFrameParser::ParseFrameIntoPackets packetSize " + m_Tools.IntegertoStringConvert(packetSize) + " " + m_Tools.IntegertoStringConvert(packetizedSize) + " " + m_Tools.IntegertoStringConvert(m_PacketSize) + " " + m_Tools.IntegertoStringConvert(in_size));

		for (int f = startFraction; f >= 0; f -= fractionInterval)
		{
			m_Packet[startPoint++] = (frameNumber >> f) & 0xFF;
		}

		for (int f = startFraction; f >= 0; f -= fractionInterval)
		{
			m_Packet[startPoint++] = (numberOfPackets >> f) & 0xFF;
		}

		for (int f = startFraction; f >= 0; f -= fractionInterval)
		{
			m_Packet[startPoint++] = (packetNumber >> f) & 0xFF;
		}

		for (int f = startFraction; f >= 0; f -= fractionInterval)
		{
			m_Packet[startPoint++] = (packetSize >> f) & 0xFF;
		}
#ifdef	RETRANSMISSION_ENABLED
		startPoint += RESEND_INFO_SIZE;
		for (int f = startFraction; f >= 0; f -= fractionInterval)
		{
			m_Packet[startPoint++] = (iTimeStampDiff >> f) & 0xFF;
		}
#endif
		startPoint = PACKET_HEADER_LENGTH+1;
		memcpy(m_Packet + startPoint, in_data + readPacketLength, packetSize);
		startPoint +=  packetSize;

		//m_pCommonElementsBucket->m_pEventNotifier->firePacketEvent(m_pCommonElementsBucket->m_pEventNotifier->ENCODED_PACKET, frameNumber, numberOfPackets, packetNumber, packetSize, packetHeaderSize + packetSize, m_Packet);

		m_Packet[0] = VIDEO_PACKET_MEDIA_TYPE;

		m_SendingBuffer.Queue(lFriendID, m_Packet, packetHeaderSize + packetSize);
    	
		g_ResendBuffer.Queue(m_Packet, packetHeaderSize + packetSize, frameNumber, packetNumber);//enqueue(pchPacketToResend);
    //     m_pCommonElementsBucket->SendFunctionPointer(lFriendID,2,m_Packet,packetHeaderSize + packetSize);
	}

	return 1;
}

void *CEncodedFrameParser::CreateEncodedFrameParsingThread(void* param)
{
	CEncodedFrameParser *pThis = (CEncodedFrameParser*)param;

	return NULL;
}

void CEncodedFrameParser::StartEncodedFrameParsingThread()
{
	std::thread t(CreateEncodedFrameParsingThread, this);

	t.detach();

	return;
}

void CEncodedFrameParser::StopEncodedFrameParsingThread()
{
	if (m_pEncodedFrameParsingThread != NULL)
	{
		Locker lock(*m_pEncodedFrameParsingMutex);

		/*delete m_pEncodedFrameParsingThread;

		m_pEncodedFrameParsingThread = NULL;*/
	}

}










void CEncodedFrameParser::StopSendingThread()
{
    //if (pInternalThread.get())
    {
        bSendingThreadRunning = false;
        
        while (!bSendingThreadClosed)
            m_Tools.SOSleep(5);
    }
    
    //pInternalThread.reset();
}

void CEncodedFrameParser::StartSendingThread()
{
    CLogPrinter::Write(CLogPrinter::INFO, "CEncodedFrameParser::StartedInternalThread 1");
    
    if (pSendingThread.get())
    {
        CLogPrinter::Write(CLogPrinter::INFO, "CEncodedFrameParser::StartedInternalThread 2");
        pSendingThread.reset();
        CLogPrinter::Write(CLogPrinter::INFO, "CEncodedFrameParser::StartDecodingThread 3");
        return;
    }
    CLogPrinter::Write(CLogPrinter::INFO, "CEncodedFrameParser::StartedInternalThread 4");
    bSendingThreadRunning = true;
    bSendingThreadClosed = false;
    CLogPrinter::Write(CLogPrinter::INFO, "CEncodedFrameParser::StartedInternalThread 5");
    
#if defined(TARGET_OS_IPHONE) || defined(TARGET_IPHONE_SIMULATOR)
    
    dispatch_queue_t SendingThreadQ = dispatch_queue_create("SendingThreadQ",DISPATCH_QUEUE_CONCURRENT);
    dispatch_async(SendingThreadQ, ^{
        this->SendingThreadProcedure();
    });
    
#else
    
    std::thread myThread(CreateVideoSendingThread, this);
    myThread.detach();
    
#endif
    
    CLogPrinter::Write(CLogPrinter::INFO, "CEncodedFrameParser::StartedInternalThread Encoding Thread started");
    
    return;
}

void *CEncodedFrameParser::CreateVideoSendingThread(void* param)
{
    CEncodedFrameParser *pThis = (CEncodedFrameParser*)param;
    pThis->SendingThreadProcedure();
    
    return NULL;
}

void CEncodedFrameParser::SendingThreadProcedure()
{
    CLogPrinter::Write(CLogPrinter::DEBUGS, "CEncodedFrameParser::EncodingThreadProcedure() Started EncodingThreadProcedure.");

	Tools toolsObject;
    int packetSize;
    LongLong lFriendID;
	int startFraction = SIZE_OF_INT_MINUS_8;
	int fractionInterval = BYTE_SIZE;
	int fpsSignal;
    
    while (bSendingThreadRunning)
    {
        //CLogPrinter::Write(CLogPrinter::INFO, "CVideoCallSession::InternalThreadImpl");
        
        if (m_SendingBuffer.GetQueueSize() == 0)
			toolsObject.SOSleep(10);
        else
        {
            packetSize = m_SendingBuffer.DeQueue(lFriendID, m_EncodedFrame);

			int startPoint = RESEND_INFO_START_BYTE_WITH_MEDIA_TYPE;
			pair<int,int> FramePacketToSend = {-1, -1};

			/*if(ExpectedFramePacketDeQueue.size() > 0)
			{
				FramePacketToSend = ExpectedFramePacketDeQueue.front();
				ExpectedFramePacketDeQueue.pop_front();
			}*/
#ifdef	RETRANSMISSION_ENABLED
			for (int f = startFraction; f >= 0; f -= fractionInterval)//ResendFrameNumber
			{
				m_EncodedFrame[startPoint ++] = (FramePacketToSend.first >> f) & 0xFF;
			}
			for (int f = startFraction; f >= 0; f -= fractionInterval)//ResendPacketNumber
			{
				m_EncodedFrame[startPoint ++] = (FramePacketToSend.second >> f) & 0xFF;
			}
#endif
//			CLogPrinter::WriteSpecific(CLogPrinter::DEBUGS, " Before Bye SIGBYTE: ");

#ifdef	FPS_CHANGE_SIGNALING
			unsigned char signal = g_FPSController.GetFPSSignalByte();
			m_EncodedFrame[1+SIGNAL_BYTE_INDEX] = signal;
#endif
//			CLogPrinter::WriteSpecific(CLogPrinter::DEBUGS, " Bye SIGBYTE: "+ m_Tools.IntegertoStringConvert(signal));

            m_pCommonElementsBucket->SendFunctionPointer(lFriendID, 2, m_EncodedFrame, packetSize);

			toolsObject.SOSleep(5);
        }
    }
    
    bSendingThreadClosed = true;
    
    CLogPrinter::Write(CLogPrinter::DEBUGS, "CEncodedFrameParser::EncodingThreadProcedure() Stopped EncodingThreadProcedure");
}


