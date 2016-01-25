#include "EncodedFrameDepacketizer.h"
#include "CommonElementsBucket.h"
#include "LogPrinter.h"
#include "Tools.h"
#include "ResendingBuffer.h"
#include "Globals.h"

//#include <android/log.h>

//#define LOG_TAG "jniEngine"
//#define LOG_TAG "dbg1"
//#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)


extern CResendingBuffer g_ResendBuffer;
extern PairMap g_timeInt;
extern CFPSController g_FPSController;

int rtCnt;
double rtSum,rtAvg;

long long g_FriendID = -1;


CEncodedFrameDepacketizer::CEncodedFrameDepacketizer(CCommonElementsBucket* sharedObject,CVideoCallSession *pVideoCallSession) :
		m_FrontFrame(0),
		m_Counter(0),
		m_BufferSize(DEPACKETIZATION_BUFFER_SIZE),
		m_pCommonElementsBucket(sharedObject),
		m_iMaxFrameNumRecvd(0)
{
	m_pEncodedFrameDepacketizerMutex.reset(new CLockHandler);
	g_FPSController.Reset();
	rtCnt=0;
	rtSum=0;
	m_iMaxFrameNumRecvdOld=0;
	m_iIntervalDroppedFPS=0;
	m_nIntervalLastFrameNumber = 0;
	m_iRetransPktDrpd=0;
	m_iRetransPktUsed=0;
	m_iCountResendPktSent = 0;
	m_iCountReqResendPacket = 0;

	m_LastDecoderSentTime = m_Tools.CurrentTimestamp();
	lastTimeStamp = m_Tools.CurrentTimestamp();

	fpsCompleteFrame=0;
	CLogPrinter_Write(CLogPrinter::INFO, "CEncodedFrameDepacketizer::CEncodedFrameDepacketizer");
	
	m_pPacketToResend = new unsigned char[MAX_VIDEO_PACKET_SIZE];

	for (int i = 0; i <= m_BufferSize; i++)
	{
		m_AvailableIndexes.insert(i);
	}

	m_BackFrame = m_FrontFrame + m_BufferSize;

	for (int frame = m_FrontFrame; frame <= m_BackFrame; frame++)
	{
		CreateNewIndex(frame);
	}

	//packetHeader();

	m_VideoCallSession = pVideoCallSession;

	CLogPrinter_Write(CLogPrinter::DEBUGS, "CEncodedFrameDepacketizer::CEncodedFrameDepacketizer created");
}

CEncodedFrameDepacketizer::~CEncodedFrameDepacketizer()
{
/*	if (NULL != m_pEncodedFrameDepacketizerThread)
	{
		delete m_pEncodedFrameDepacketizerThread;
		m_pEncodedFrameDepacketizerThread = NULL;
	}*/

	if(NULL != m_pPacketToResend)
	{
		delete m_pPacketToResend;
	}
	SHARED_PTR_DELETE(m_pEncodedFrameDepacketizerMutex);
}



int CEncodedFrameDepacketizer::Depacketize(unsigned char *in_data, unsigned int in_size, bool bIsMiniPacket, CPacketHeader &packetHeader)
{
    bool bIsRetransmitted = false;
    
    int firstByte = 0;
    if(!bIsMiniPacket)
    {
        firstByte = in_data[SIGNAL_BYTE_INDEX_WITHOUT_MEDIA];
        
        if(in_data[SIGNAL_BYTE_INDEX_WITHOUT_MEDIA]&(1<<4))
        {
            bIsRetransmitted = true;
        }
        else
        {
#ifdef FPS_CHANGE_SIGNALING
            g_FPSController.SetFPSSignalByte(in_data[SIGNAL_BYTE_INDEX_WITHOUT_MEDIA]);
            m_VideoCallSession->ownFPS = g_FPSController.GetOwnFPS();
            m_VideoCallSession->opponentFPS = g_FPSController.GetOpponentFPS();
#endif
        }
    }

	int frameNumber = packetHeader.getFrameNumber();
	int numberOfPackets = packetHeader.getNumberOfPacket();
	int packetNumber = packetHeader.getPacketNumber();
	int packetLength = packetHeader.getPacketLength();
    unsigned int timeStampDiff = 0;

    if(!bIsMiniPacket)
    {
        timeStampDiff = packetHeader.getTimeStamp();
        CLogPrinter::WriteSpecific(CLogPrinter::DEBUGS, "CVideoCallSession::timeStampDiff "+ m_Tools.IntegertoStringConvert(timeStampDiff));
    }

	CLogPrinter_WriteSpecific2(CLogPrinter::DEBUGS, "FN: " + m_Tools.IntegertoStringConvert(frameNumber)
													+ " NOP: "+ m_Tools.IntegertoStringConvert(numberOfPackets)+ " PN: "+ m_Tools.IntegertoStringConvert(packetNumber) + " timedif: "+ m_Tools.IntegertoStringConvert(timeStampDiff));
    

	int index;

	if(in_size>PACKET_HEADER_LENGTH && frameNumber > m_iMaxFrameNumRecvd)
	{
		m_iMaxFrameNumRecvd = frameNumber;
	}

#ifdef	RETRANSMISSION_ENABLED
	if(bIsRetransmitted)
	{
		long long td = g_timeInt.getTimeDiff(frameNumber,packetNumber);
		if(td!=-1)
		{
			rtSum+=td;
			rtCnt++;
			rtAvg = rtSum/rtCnt;
			CLogPrinter_WriteSpecific(CLogPrinter::DEBUGS, "PushPacketForDecoding:: $#()() Retransmitted Time:"+m_Tools.DoubleToString(rtAvg)+"  This: "+m_Tools.IntegertoStringConvert(td));
		}
	}
    

	if(bIsMiniPacket) //This block is for resending packets and has no relation with the packet passed to this function
	{
        CLogPrinter_WriteSpecific(CLogPrinter::DEBUGS, "CEncodedFrameDepacketizer::King-->PushPacketForDecoding Resend Packet Found resendframe: "+
                                   m_Tools.IntegertoStringConvert(frameNumber) + " resendpacket: "+ m_Tools.IntegertoStringConvert(packetNumber)+
                                   "   in_size: "+m_Tools.IntegertoStringConvert(in_size));

		++m_iCountReqResendPacket;
		int resendPacketLength = g_ResendBuffer.DeQueue(m_pPacketToResend ,frameNumber, packetNumber );

		if(resendPacketLength != -1)
		{
			m_pPacketToResend[SIGNAL_BYTE_INDEX_WITHOUT_MEDIA + 1] |= (1<<7); //Retransmitted packet flag added

			if(g_FriendID != -1)
			{
				m_iCountResendPktSent++;
				/*CLogPrinter_WriteSpecific(CLogPrinter::DEBUGS, "CEncodedFrameDepacketizer::PushPacketForDecoding Resend Packet Found resendframe: "+
																m_Tools.IntegertoStringConvert(resendframe) + " resendpacket: "+ m_Tools.IntegertoStringConvert(resendpacket)+
																" resendpacketLenght: "+ m_Tools.IntegertoStringConvert(resendPacketLength));*/

				m_pCommonElementsBucket->SendFunctionPointer(g_FriendID, 2, m_pPacketToResend, PACKET_HEADER_LENGTH + resendPacketLength);
				CLogPrinter_WriteSpecific(CLogPrinter::DEBUGS, "PushPacketForDecoding:: $#() RetransPKT USED = " + m_Tools.IntegertoStringConvert(m_iRetransPktUsed) + " DROPED = " + m_Tools.IntegertoStringConvert(m_iRetransPktDrpd) );
			}
			else
			{
				CLogPrinter_WriteSpecific(CLogPrinter::DEBUGS, "CEncodedFrameDepacketizer::PushPacketForDecoding g_FriendID == -1" );
			}
		}
        return -1;
	}
#endif
	//if(frameNumber%50==0)
	{
		CLogPrinter_WriteSpecific2(CLogPrinter::DEBUGS, "PushPacketForDecoding:: $$$#( fram: " +
														m_Tools.IntegertoStringConvert(
																frameNumber) + " pkt: " +
														m_Tools.IntegertoStringConvert(
																packetNumber) + " ~ " +
														m_Tools.IntegertoStringConvert(
																numberOfPackets) + " #FPS own:" +
														m_Tools.IntegertoStringConvert(
																m_VideoCallSession->ownFPS) +
														" Oppo: " + m_Tools.IntegertoStringConvert(
				m_VideoCallSession->opponentFPS)
														+ " #Resend Request:" +
														m_Tools.IntegertoStringConvert(
																m_iCountReqResendPacket) +
														" SENT: " + m_Tools.IntegertoStringConvert(
				m_iCountResendPktSent)+" -> SIGBYTE: "+m_Tools.IntegertoStringConvert(firstByte));
		CLogPrinter_WriteSpecific(CLogPrinter::DEBUGS, "CEncodedFrameDepacketizer:: MaxFrameNumber = "+ m_Tools.IntegertoStringConvert(m_iMaxFrameNumRecvd));
	}

	if (frameNumber < m_FrontFrame)		//Very old frame
	{
		if(bIsRetransmitted)
        {
            ++m_iRetransPktDrpd;
            CLogPrinter_WriteSpecific(CLogPrinter::DEBUGS, "CEncodedFrameDepacketizer:: m_iRetransPktDrpd = "+m_Tools.IntegertoStringConvert(m_iRetransPktDrpd) );
        }
        
		return -1;
	}
	else if (frameNumber > m_BackFrame)
	{
		int previousBackFrame = m_BackFrame;

		m_BackFrame = frameNumber;

		if (m_FrontFrame + m_BufferSize < m_BackFrame)
		{
			int previousFrontFrame = m_FrontFrame;
			m_FrontFrame = m_BackFrame - m_BufferSize;

			int frame;

			for (frame = previousFrontFrame; frame < m_FrontFrame; frame++)		//Remove all old frames fromm Merging buffer
			{
				std::map<int, int>::iterator it = m_FrameTracker.find(frame);

				if (it == m_FrameTracker.end())
					break;
				else
				{
					ClearAndDeliverFrame(frame);
				}
			}

			if (previousBackFrame >=m_FrontFrame)
				frame = previousBackFrame + 1;
			else
				frame = m_FrontFrame;

			for (; frame < m_BackFrame; frame++)
			{
				CreateNewIndex(frame);
			}
		}

		int newIndex = CreateNewIndex(frameNumber);
		m_CVideoPacketBuffer[newIndex].SetNumberOfPackets(numberOfPackets);
		index = newIndex;
	}
	else
	{
		index = SafeFinder(frameNumber);

		if(index != -1)
			m_CVideoPacketBuffer[index].SetNumberOfPackets(numberOfPackets);
		else
			CLogPrinter_WriteSpecific(CLogPrinter::DEBUGS, "CEncodedFrameDepacketizer:: Invalid Index : "+m_Tools.IntegertoStringConvert(index) );
	}

	if(bIsRetransmitted)
    {
        ++m_iRetransPktUsed;
        CLogPrinter_WriteSpecific(CLogPrinter::DEBUGS, "CEncodedFrameDepacketizer:: m_iRetransPktUsed = "+m_Tools.IntegertoStringConvert(m_iRetransPktUsed) );
    }
    
    
    
	int isCompleteFrame = m_CVideoPacketBuffer[index].PushVideoPacket(in_data, packetLength, packetNumber);
//	CLogPrinter_WriteSpecific(CLogPrinter::DEBUGS, "CEncodedFrameDepacketizer:: m_FrontFrame : "+m_Tools.IntegertoStringConvert(m_FrontFrame) + " :: m_iMaxFrameNumRecvd: "+ m_Tools.IntegertoStringConvert(m_iMaxFrameNumRecvd) + " :: diff: "+ m_Tools.IntegertoStringConvert(m_iMaxFrameNumRecvd-m_FrontFrame));

	index = SafeFinder(m_FrontFrame);
	if(index==-1)	return -1;

#ifdef RETRANSMISSION_ENABLED
	if(m_FrontFrame + TIME_DELAY_FOR_RETRANSMISSION * m_VideoCallSession->opponentFPS >= m_iMaxFrameNumRecvd) return -1;


	long long curTime = m_Tools.CurrentTimestamp();
	int minTimeDiff = 1000/m_VideoCallSession->opponentFPS;
//	if(m_iMaxFrameNumRecvdOld >= m_iMaxFrameNumRecvd )	return -1;
//	if(curTime<=minTimeDiff +m_LastDecoderSentTime)	return -1;
//
	m_iMaxFrameNumRecvdOld = m_iMaxFrameNumRecvd;
	m_LastDecoderSentTime = curTime;

	if(!m_CVideoPacketBuffer[index].IsComplete())
	{
		g_FPSController.NotifyFrameDropped(m_FrontFrame);
		MoveForward(m_FrontFrame);
		return 1;
	}

	timeStamp = m_mFrameTimeStamp[m_FrontFrame];

	CLogPrinter_WriteSpecific(CLogPrinter::DEBUGS, "PushPacketForDecoding:: timeStamp: " + m_Tools.IntegertoStringConvert(timeStamp) + " m_FrontFrame: "+ m_Tools.IntegertoStringConvert(m_FrontFrame));

	m_VideoCallSession->PushFrameForDecoding( m_CVideoPacketBuffer[index].m_pFrameData, m_CVideoPacketBuffer[index].m_FrameSize, m_FrontFrame, timeStampDiff );
	g_FPSController.NotifyFrameComplete(m_FrontFrame);
	MoveForward(m_FrontFrame);

	return 1;

#else

	if(isCompleteFrame<1)	return -1;

	if (frameNumber == m_FrontFrame)
	{
		timeStamp = m_mFrameTimeStamp[frameNumber];
		m_VideoCallSession->PushFrameForDecoding(m_CVideoPacketBuffer[index].m_pFrameData, m_CVideoPacketBuffer[index].m_FrameSize,m_FrontFrame, timeStampDiff);	//Send front frame
		g_FPSController.NotifyFrameComplete(frameNumber);
		MoveForward(frameNumber);

		for (int frame = m_FrontFrame; frame < m_BackFrame; frame++)	//Send all consecutive complete frames after front frame.
		{
			int inIndex = SafeFinder(frame);
			if(-1 == inIndex)
				break;

			if (m_CVideoPacketBuffer[inIndex].IsComplete())
			{
				timeStamp = m_mFrameTimeStamp[frame];
				m_VideoCallSession->PushFrameForDecoding(m_CVideoPacketBuffer[inIndex].m_pFrameData, m_CVideoPacketBuffer[inIndex].m_FrameSize,frame, timeStampDiff);
				g_FPSController.NotifyFrameComplete(frame);
				MoveForward(frame);
			}
			else
				break;
		}

		return 1;
	}

	index = SafeFinder(frameNumber);
	if(-1 == index)
		return 1;

//	if(0 == frameNumber%8 && m_CVideoPacketBuffer[index].IsIFrame()==false)
//		CLogPrinter_WriteSpecific(CLogPrinter::DEBUGS, " if--MIS -- MATCH ______________________________________" + m_Tools.IntegertoStringConvert(frameNumber));
//	else if(0 != frameNumber%8 && m_CVideoPacketBuffer[index].IsIFrame())
//		CLogPrinter_WriteSpecific(CLogPrinter::DEBUGS, " Else if --MIS -- MATCH ______________________________________" + m_Tools.IntegertoStringConvert(frameNumber));


	if(0 == frameNumber%I_INTRA_PERIOD)		//If frameNumber is a I-Frame
	{
		for (int frame = m_FrontFrame; frame < frameNumber; frame++)		//Remove all frames before I-Frame
		{
			int inIndex = SafeFinder(frame);
			if(-1 == inIndex)
				continue;

			if(!m_CVideoPacketBuffer[inIndex].IsComplete())
				g_FPSController.NotifyFrameDropped(frame);

			MoveForward(frame);
		}

		timeStamp = m_mFrameTimeStamp[frameNumber];
		m_VideoCallSession->PushFrameForDecoding(m_CVideoPacketBuffer[index].m_pFrameData, m_CVideoPacketBuffer[index].m_FrameSize,frameNumber, timeStampDiff);		//Send I-Frame
		g_FPSController.NotifyFrameComplete(frameNumber);
		MoveForward(frameNumber);

		for (int frame = frameNumber; frame < m_BackFrame; frame++)			//Send all consecutive complete frames after the I-Frame
		{
			int inIndex = SafeFinder(frame);
			if(-1 == inIndex)
				break;

			if (m_CVideoPacketBuffer[inIndex].IsComplete())
			{
				timeStamp = m_mFrameTimeStamp[frame];
				m_VideoCallSession->PushFrameForDecoding(m_CVideoPacketBuffer[inIndex].m_pFrameData, m_CVideoPacketBuffer[inIndex].m_FrameSize, frame, timeStampDiff);
				g_FPSController.NotifyFrameComplete(frame);
				MoveForward(frame);
			}
			else
				break;
		}
	}

	return 1;

#endif
}

int CEncodedFrameDepacketizer::SafeFinder(int Data)
{
#ifdef CRASH_CHECK
	std::map<int, int>::iterator it = m_FrameTracker.find(Data);
	if(it == m_FrameTracker.end())
	{
		CLogPrinter_WriteSpecific(CLogPrinter::DEBUGS, "To learn how to use maps: search Isearch.SafeFinder.net. SafeFinder:: Invalid Index," );
		return -1;
	}
	int index = it->second;
	if(0<=index && index<=DEPACKETIZATION_BUFFER_SIZE)
		return index;
	else
	{
		CLogPrinter_WriteSpecific(CLogPrinter::DEBUGS, "To learn how to use maps: search Isearch.SafeFinder.net. SafeFinder:: Index out of range. Index : "+Tools::IntegertoStringConvert(index));
		return -1;
	}
#else
	return m_FrameTracker.find(Data)->second;
#endif
}

void CEncodedFrameDepacketizer::MoveForward(int frame)
{
//	int indexInside = m_FrameTracker.find(frame)->second;
	int indexInside = SafeFinder(frame);
	if(indexInside==-1)
		return;

	ClearFrame(indexInside, frame);

	m_FrontFrame++;
	m_BackFrame++;

	CreateNewIndex(m_BackFrame);
}

int CEncodedFrameDepacketizer::CreateNewIndex(int frame)
{
	int newIndex = *m_AvailableIndexes.begin(); 

	if (m_AvailableIndexes.begin() != m_AvailableIndexes.end())
		m_AvailableIndexes.erase(m_AvailableIndexes.begin());
	else
	{
		m_FrontFrame = 0;

		for (int i = 0; i <= m_BufferSize; i++)
		{
			m_AvailableIndexes.insert(i);
		}

		m_BackFrame = m_FrontFrame + m_BufferSize;

		for (int iFrame = m_FrontFrame; iFrame <= m_BackFrame; iFrame++)
		{
			CreateNewIndex(iFrame);
		}

		newIndex = 0;
	}

	m_FrameTracker.insert(std::pair<int, int>(frame, newIndex));

	m_CVideoPacketBuffer[newIndex].Reset();

	return newIndex;
}

void CEncodedFrameDepacketizer::ClearAndDeliverFrame(int frame)
{
//	int indexInside = m_FrameTracker.find(frame)->second;
	int indexInside = SafeFinder(frame);
	if(0 > indexInside || indexInside>DEPACKETIZATION_BUFFER_SIZE) {
		CLogPrinter_WriteSpecific(CLogPrinter::DEBUGS, "ClearAndDeliverFrame:: Invalid Index  ########################################### "+Tools::IntegertoStringConvert(indexInside) );
		return;
	}


	if (m_CVideoPacketBuffer[indexInside].IsComplete() && frame+m_VideoCallSession->opponentFPS>m_FrontFrame)
	{
		//timeStamp = m_mFrameTimeStamp[m_FrontFrame];
		//m_mFrameTimeStamp.erase (m_FrontFrame);
		CLogPrinter_WriteSpecific(CLogPrinter::DEBUGS, "PushPacketForDecoding:: timeStamp: " + m_Tools.IntegertoStringConvert(timeStamp));

		timeStamp = m_mFrameTimeStamp[frame];
		m_VideoCallSession->PushFrameForDecoding(m_CVideoPacketBuffer[indexInside].m_pFrameData,
													 m_CVideoPacketBuffer[indexInside].m_FrameSize,
													 frame, m_mFrameTimeStamp[frame]);
		g_FPSController.NotifyFrameComplete(frame);
	}
	else {
		g_FPSController.NotifyFrameDropped(frame);
	}

	ClearFrame(indexInside, frame);
}

void CEncodedFrameDepacketizer::ClearFrame(int index, int frame)
{
	if(m_mFrameTimeStamp.find(frame)!=m_mFrameTimeStamp.end())
		m_mFrameTimeStamp.erase(frame);

	if(-1<index && index<=DEPACKETIZATION_BUFFER_SIZE)
	{
		m_CVideoPacketBuffer[index].Reset();
		m_AvailableIndexes.insert(index);
		m_FrameTracker.erase(frame);
	}
	else{
		CLogPrinter_WriteSpecific(CLogPrinter::DEBUGS, "CEncodedFrameDepacketizer::ClearFrame, InvalidIndex = " + m_Tools.IntegertoStringConvert(index) + ", frame = " + m_Tools.IntegertoStringConvert(frame) + "###############################");
	}
}


