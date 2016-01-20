#include "EncodedFramePacketizer.h"
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

CEncodedFramePacketizer::CEncodedFramePacketizer(CCommonElementsBucket* sharedObject) :
		m_PacketSize(MAX_PACKET_SIZE_WITHOUT_HEADER),
		m_pCommonElementsBucket(sharedObject)
{
	CLogPrinter_Write(CLogPrinter::INFO, "CEncodedFramePacketizer::CEncodedFramePacketizer");

	m_pEncodedFrameParsingMutex.reset(new CLockHandler);
	m_pEncodedFrameParsingThread = NULL;
    
    StartSendingThread();

	CLogPrinter_Write(CLogPrinter::DEBUGS, "CEncodedFramePacketizer::CEncodedFramePacketizer Created");
}

CEncodedFramePacketizer::~CEncodedFramePacketizer()
{
/*	if (m_pEncodedFrameParsingThread)
	{
		delete m_pEncodedFrameParsingThread;
		m_pEncodedFrameParsingThread = NULL;
	}*/
    
    StopSendingThread();

	SHARED_PTR_DELETE(m_pEncodedFrameParsingMutex);
}

int CEncodedFramePacketizer::Packetize(LongLong lFriendID, unsigned char *in_data, unsigned int in_size, int frameNumber,
											   unsigned int iTimeStampDiff)
{
	CLogPrinter_Write(CLogPrinter::DEBUGS, "CEncodedFramePacketizer::Packetize parsing started");

	int packetizedSize = m_PacketSize;

	int startFraction = SIZE_OF_INT_MINUS_8;
	int fractionInterval = BYTE_SIZE;
	int packetHeaderSize = PACKET_HEADER_LENGTH_WITH_MEDIA_TYPE;
	int readPacketLength;

	int numberOfPackets = in_size / m_PacketSize;

	CLogPrinter_Write(CLogPrinter::INFO, "CEncodedFramePacketizer::Packetize in_size " + m_Tools.IntegertoStringConvert(in_size) + " m_PacketSize " + m_Tools.IntegertoStringConvert(m_PacketSize));

	if (in_size % m_PacketSize != 0 )
	{
		CLogPrinter_Write(CLogPrinter::INFO, "CEncodedFramePacketizer::Packetize increased" );

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

        
        for (int f = startFraction; f >= 0; f -= fractionInterval)
        {
            m_Packet[startPoint++] = (iTimeStampDiff >> f) & 0xFF;
        }
        
		startPoint = PACKET_HEADER_LENGTH+1;
		memcpy(m_Packet + startPoint, in_data + readPacketLength, m_PacketSize);
		startPoint += m_PacketSize;
		packetizedSize += m_PacketSize;

		//m_pCommonElementsBucket->m_pEventNotifier->firePacketEvent(m_pCommonElementsBucket->m_pEventNotifier->ENCODED_PACKET, frameNumber, numberOfPackets, packetNumber, m_PacketSize, packetHeaderSize + m_PacketSize, m_Packet);

		CLogPrinter_WriteSpecific(CLogPrinter::INFO, "Parsing..>>>");
        m_Packet[0] = VIDEO_PACKET_MEDIA_TYPE;

		m_SendingBuffer.Queue(lFriendID, m_Packet, packetHeaderSize + m_PacketSize);
        g_ResendBuffer.Queue(m_Packet, packetHeaderSize + m_PacketSize, frameNumber, packetNumber);//enqueue(pchPacketToResend);
    //    m_pCommonElementsBucket->SendFunctionPointer(lFriendID,2,m_Packet,packetHeaderSize + m_PacketSize);
	}

	//CLogPrinter_Write(CLogPrinter::INFO, "CEncodedFramePacketizer::Packetize packetSize " + m_Tools.IntegertoStringConvert(packetNumber) + " " + m_Tools.IntegertoStringConvert(numberOfPackets) + " " + m_Tools.IntegertoStringConvert(packetizedSize) + " " + m_Tools.IntegertoStringConvert(m_PacketSize) + " " + m_Tools.IntegertoStringConvert(in_size));

	if (packetNumber < numberOfPackets )
	{
		int startPoint = 1;
		int packetSize = in_size - ( packetizedSize - m_PacketSize );

		//CLogPrinter_Write(CLogPrinter::INFO, "CEncodedFramePacketizer::Packetize packetSize " + m_Tools.IntegertoStringConvert(packetSize) + " " + m_Tools.IntegertoStringConvert(packetizedSize) + " " + m_Tools.IntegertoStringConvert(m_PacketSize) + " " + m_Tools.IntegertoStringConvert(in_size));

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
        
        for (int f = startFraction; f >= 0; f -= fractionInterval)
        {
            m_Packet[startPoint++] = (iTimeStampDiff >> f) & 0xFF;
        }
        
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










void CEncodedFramePacketizer::StopSendingThread()
{
    //if (pInternalThread.get())
    {
        bSendingThreadRunning = false;
        
        while (!bSendingThreadClosed)
            m_Tools.SOSleep(5);
    }
    
    //pInternalThread.reset();
}

void CEncodedFramePacketizer::StartSendingThread()
{
    CLogPrinter_Write(CLogPrinter::INFO, "CEncodedFramePacketizer::StartedInternalThread 1");
    
    if (pSendingThread.get())
    {
        CLogPrinter_Write(CLogPrinter::INFO, "CEncodedFramePacketizer::StartedInternalThread 2");
        pSendingThread.reset();
        CLogPrinter_Write(CLogPrinter::INFO, "CEncodedFramePacketizer::StartDecodingThread 3");
        return;
    }
    CLogPrinter_Write(CLogPrinter::INFO, "CEncodedFramePacketizer::StartedInternalThread 4");
    bSendingThreadRunning = true;
    bSendingThreadClosed = false;
    CLogPrinter_Write(CLogPrinter::INFO, "CEncodedFramePacketizer::StartedInternalThread 5");
    
#if defined(TARGET_OS_IPHONE) || defined(TARGET_IPHONE_SIMULATOR)
    
    dispatch_queue_t SendingThreadQ = dispatch_queue_create("SendingThreadQ",DISPATCH_QUEUE_CONCURRENT);
    dispatch_async(SendingThreadQ, ^{
        this->SendingThreadProcedure();
    });
    
#else
    
    std::thread myThread(CreateVideoSendingThread, this);
    myThread.detach();
    
#endif
    
    CLogPrinter_Write(CLogPrinter::INFO, "CEncodedFramePacketizer::StartedInternalThread Encoding Thread started");
    
    return;
}

void *CEncodedFramePacketizer::CreateVideoSendingThread(void* param)
{
    CEncodedFramePacketizer *pThis = (CEncodedFramePacketizer*)param;
    pThis->SendingThreadProcedure();
    
    return NULL;
}

void CEncodedFramePacketizer::SendingThreadProcedure()
{
    CLogPrinter_Write(CLogPrinter::DEBUGS, "CEncodedFramePacketizer::EncodingThreadProcedure() Started EncodingThreadProcedure.");

	Tools toolsObject;
    int packetSize;
    LongLong lFriendID;
	int startFraction = SIZE_OF_INT_MINUS_8;
	int fractionInterval = BYTE_SIZE;
	int fpsSignal;
    
    while (bSendingThreadRunning)
    {
        //CLogPrinter_Write(CLogPrinter::INFO, "CVideoCallSession::InternalThreadImpl");
        
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
			/*for (int f = startFraction; f >= 0; f -= fractionInterval)//ResendFrameNumber
			{
				m_EncodedFrame[startPoint ++] = (FramePacketToSend.first >> f) & 0xFF;
			}
			for (int f = startFraction; f >= 0; f -= fractionInterval)//ResendPacketNumber
			{
				m_EncodedFrame[startPoint ++] = (FramePacketToSend.second >> f) & 0xFF;
			}*/
#endif
//			CLogPrinter_WriteSpecific(CLogPrinter::DEBUGS, " Before Bye SIGBYTE: ");

#ifdef	FPS_CHANGE_SIGNALING
			unsigned char signal = g_FPSController.GetFPSSignalByte();
			m_EncodedFrame[1+SIGNAL_BYTE_INDEX] = signal;
#endif
//			CLogPrinter_WriteSpecific(CLogPrinter::DEBUGS, " Bye SIGBYTE: "+ m_Tools.IntegertoStringConvert(signal));

            m_pCommonElementsBucket->SendFunctionPointer(lFriendID, 2, m_EncodedFrame, packetSize);
            
            toolsObject.SOSleep((int)(SENDING_INTERVAL_FOR_15_FPS * MAX_FPS * 1.0) / (g_FPSController.GetOwnFPS()  * 1.0));

			
        }
    }
    
    bSendingThreadClosed = true;
    
    CLogPrinter_Write(CLogPrinter::DEBUGS, "CEncodedFramePacketizer::EncodingThreadProcedure() Stopped EncodingThreadProcedure");
}


