#include "EncodedFrameDepacketizer.h"
#include "CommonElementsBucket.h"
#include "LogPrinter.h"
#include "Tools.h"
#include "ResendingBuffer.h"
#include "Globals.h"

namespace MediaSDK
{

	//extern bool g_bIsVersionDetectableOpponent;
	//extern unsigned char g_uchSendPacketVersion;
	//extern int g_uchOpponentVersion;

	//extern PairMap g_timeInt;

#define DEFAULT_FIRST_FRAME_RCVD  65000

	map<long long, long long>g_ArribalTime;
	long long g_llChangeSum;
	int g_iChangeCounter;

	CEncodedFrameDepacketizer::CEncodedFrameDepacketizer(CCommonElementsBucket* sharedObject, CVideoCallSession *pVideoCallSession) :
		m_FrontFrame(0),
		m_Counter(0),
		m_BufferSize(DEPACKETIZATION_BUFFER_SIZE),
		m_pCommonElementsBucket(sharedObject),
		m_iMaxFrameNumRecvd(-1)
	{
		m_pEncodedFrameDepacketizerMutex.reset(new CLockHandler);

		g_ArribalTime.clear();
		g_llChangeSum = g_iChangeCounter = 0;
		m_iFirstFrameReceived = DEFAULT_FIRST_FRAME_RCVD;
		m_bIsDpkgBufferFilledUp = false;

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
		if (NULL != m_pPacketToResend)
		{
			delete m_pPacketToResend;
		}
		SHARED_PTR_DELETE(m_pEncodedFrameDepacketizerMutex);
	}

	void CEncodedFrameDepacketizer::ResetEncodedFrameDepacketizer()
	{
		m_FrontFrame = 0;
		m_Counter = 0;
		m_BufferSize = DEPACKETIZATION_BUFFER_SIZE;
		m_iMaxFrameNumRecvd = -1;

		g_ArribalTime.clear();
		g_llChangeSum = g_iChangeCounter = 0;
		m_iFirstFrameReceived = DEFAULT_FIRST_FRAME_RCVD;
		m_bIsDpkgBufferFilledUp = false;

		for (int i = 0; i <= m_BufferSize; i++)
		{
			m_AvailableIndexes.insert(i);
		}

		m_BackFrame = m_FrontFrame + m_BufferSize;

		for (int frame = m_FrontFrame; frame <= m_BackFrame; frame++)
		{
			CreateNewIndex(frame);
		}
	}

	int CEncodedFrameDepacketizer::Depacketize(unsigned char *in_data, unsigned int in_size, int PacketType, CVideoHeader &packetHeader)
	{
		bool bIsMiniPacket = (PacketType == MINI_PACKET_TYPE);
		int firstByte = 0;

#ifdef FPS_CHANGE_SIGNALING
		if (NORMAL_PACKET_TYPE == PacketType)
		{
			firstByte = in_data[SIGNAL_BYTE_INDEX_WITHOUT_MEDIA];
			m_VideoCallSession->GetFPSController()->SetFPSSignalByte(in_data[SIGNAL_BYTE_INDEX_WITHOUT_MEDIA]);
		}
#endif

		long long frameNumber = packetHeader.getFrameNumber();
		int numberOfPackets = packetHeader.getNumberOfPacket();
		int packetNumber = packetHeader.getPacketNumber();
		int packetLength = packetHeader.getPacketLength();
		long long timeStampDiff = packetHeader.getTimeStamp();
		int orientation = packetHeader.GetDeviceOrientation();

		//	if(g_ArribalTime.find(frameNumber) == g_ArribalTime.end()) {

		if (0 == packetNumber) {
			long long llCurTime = Tools::CurrentTimestamp();

			CLogPrinter_WriteLog(CLogPrinter::DEBUGS, DEPACKETIZATION_LOG, "#DP~ FN: " + m_Tools.getText(frameNumber) + " CurTime: " + m_Tools.LongLongToString(llCurTime) + " timedif: " + m_Tools.IntegertoStringConvert(timeStampDiff) + " Sh: " + m_Tools.LongLongToString(m_VideoCallSession->GetShiftedTime())
				+ "BUF SZ: " + Tools::IntegertoStringConvert(m_BackFrame - m_FrontFrame + 1));
			g_ArribalTime[frameNumber] = llCurTime;

			long long curDiff = TIME_DELAY_FOR_RETRANSMISSION_IN_MS + llCurTime - timeStampDiff;

			if (-1 == m_VideoCallSession->GetShiftedTime()) {
				m_VideoCallSession->SetShiftedTime(curDiff);
				g_iChangeCounter++;
				CLogPrinter_WriteLog(CLogPrinter::DEBUGS, DEPACKETIZATION_LOG, "#@ SH# *******************FirstShift " + m_Tools.LongLongToString(m_VideoCallSession->GetShiftedTime()));
			}
			/*else if(m_VideoCallSession->GetShiftedTime() > curDiff) {
				long long llCurChange = m_VideoCallSession->GetShiftedTime() - curDiff;
				g_llChangeSum += llCurChange;
				g_iChangeCounter++;
				m_VideoCallSession->SetShiftedTime(curDiff);
				CLogPrinter_WriteLog(CLogPrinter::DEBUGS, DEPACKETIZATION_LOG ,"#@ SH# CurShift " + m_Tools.LongLongToString(llCurChange) + " ChangeSum: "+ m_Tools.LongLongToString(g_llChangeSum)
				+ " CNT: "+ m_Tools.IntegertoStringConvert(g_iChangeCounter)
				+ " Shift: "+ m_Tools.LongLongToString(m_VideoCallSession->GetShiftedTime()));
				}*/
		}

		if (-1 == m_VideoCallSession->GetFirstVideoPacketTime())
		{
			m_VideoCallSession->SetFirstFrameEncodingTime(timeStampDiff);
			m_VideoCallSession->SetFirstVideoPacketTime(Tools::CurrentTimestamp());
		}

		//	CLogPrinter_WriteInstentTestLog(CLogPrinter::DEBUGS, "FN: " + m_Tools.getText(frameNumber) + " NOP: "+ m_Tools.IntegertoStringConvert(numberOfPackets)+ " PN: "+ m_Tools.IntegertoStringConvert(packetNumber) + " timedif: "+ m_Tools.IntegertoStringConvert(timeStampDiff));

		CLogPrinter_WriteLog(CLogPrinter::INFO, PACKET_LOSS_INFO_LOG, " &*&*Receiving frameNumber: " + m_Tools.getText(frameNumber) + " :: PacketNo: " + m_Tools.IntegertoStringConvert(packetNumber));

		int index;

		if (in_size > packetHeader.GetHeaderLength() && frameNumber > m_iMaxFrameNumRecvd)
		{
			m_iMaxFrameNumRecvd = frameNumber;

			if (m_iFirstFrameReceived > frameNumber) {
				m_iFirstFrameReceived = frameNumber;
				m_FirstFrameEncodingTime = timeStampDiff;
			}
		}

		DepacketizerLocker lock(*m_pEncodedFrameDepacketizerMutex);

		m_mFrameTimeStamp[frameNumber] = timeStampDiff;
		m_mFrameOrientation[frameNumber] = orientation;

		if (frameNumber < m_FrontFrame)		//Very old frame
		{
			return -1;
		}
		else if (m_FrontFrame > m_BackFrame)	//IF BUFFER IS EMPTY
		{
			//		CLogPrinter_WriteSpecific(CLogPrinter::DEBUGS, "GetReceivedFrame:: EMPTY BUFFER  = "+ m_Tools.IntegertoStringConvert(frameNumber)+
			//				" ["+ m_Tools.IntegertoStringConvert(m_FrontFrame)+" @ "+ m_Tools.IntegertoStringConvert(m_FrontFrame));
			m_FrontFrame = max(m_FrontFrame, frameNumber - 3);
			m_BackFrame = frameNumber;

			for (int frame = m_FrontFrame; frame < m_BackFrame; frame++)
				CreateNewIndex(frame);

			index = CreateNewIndex(m_BackFrame);
			//		CLogPrinter_WriteSpecific(CLogPrinter::DEBUGS, "GetReceivedFrame:: NEW BUFFER  = "
			//														+ m_Tools.IntegertoStringConvert(m_FrontFrame)+" @ "+ m_Tools.IntegertoStringConvert(m_FrontFrame));
		}
		else if (frameNumber > m_BackFrame)
		{
			int previousBackFrame = m_BackFrame;

			m_BackFrame = frameNumber;

			if (m_FrontFrame + m_BufferSize < m_BackFrame)
			{
				CLogPrinter_WriteInstentTestLog(CLogPrinter::DEBUGS, "####--------------------------------->BufferOverflow# FN: " + m_Tools.IntegertoStringConvert(m_FrontFrame));
				CLogPrinter_WriteLog(CLogPrinter::DEBUGS, QUEUE_OVERFLOW_LOG, "Video Buffer OverFlow ( VideoPacketBuffer in EncodedFrameDepacketizer ) --> OverFlow FrontFrame " + m_Tools.IntegertoStringConvert(m_FrontFrame) + " BackFrame " + m_Tools.IntegertoStringConvert(m_BackFrame));
				int previousFrontFrame = m_FrontFrame;

				m_FrontFrame = max(m_FrontFrame, m_BackFrame - m_BufferSize);

				int frame;

				for (frame = previousFrontFrame; frame < m_FrontFrame; frame++)		//Remove all old frames fromm Merging buffer
				{
					std::map<int, int>::iterator it = m_FrameTracker.find(frame);

					if (it == m_FrameTracker.end())
						break;
					else
					{
						index = SafeFinder(frame);
						if (index == -1)
							break;

						ClearFrame(index, frame);
					}
				}

				if (previousBackFrame >= m_FrontFrame)		///NewFrameNumber <= BUFFER_SIZE + PreviousBackFrame
					frame = previousBackFrame + 1;
				else
				{
					m_FrontFrame = max(m_FrontFrame, m_BackFrame - 3);				///NewFrameNumber > BUFFER_SIZE + PreviousBackFrame
					frame = m_FrontFrame;
				}

				for (; frame < m_BackFrame; frame++)
				{
					CreateNewIndex(frame);
				}
			}
			else {
				for (int frame = 1 + previousBackFrame; frame < m_BackFrame; ++frame)
				{
					CreateNewIndex(frame);
				}
			}

			index = CreateNewIndex(frameNumber);
		}
		else
		{
			index = SafeFinder(frameNumber);

			if (index == -1) {
				CLogPrinter_WriteSpecific(CLogPrinter::DEBUGS,
					"CEncodedFrameDepacketizer::PushPacketForDecoding Invalid Index : " +
					m_Tools.IntegertoStringConvert(index));
				return -1;
			}
		}

		//	CLogPrinter_WriteSpecific(CLogPrinter::DEBUGS,
		//							   " GetReceivedFrame : PushPacketForDecoding  InsertTime " +
		//							   m_Tools.IntegertoStringConvert(frameNumber)+" == "+m_Tools.IntegertoStringConvert(timeStampDiff));

		m_CVideoPacketBuffer[index].SetNumberOfPackets(numberOfPackets);

		int iHeaderLength = packetHeader.GetHeaderLength();
		int nPacketStartingIndex = packetHeader.GetPacketStartingIndex();

		int isCompleteFrame = m_CVideoPacketBuffer[index].PushVideoPacket(in_data, packetLength, packetNumber, iHeaderLength, nPacketStartingIndex);

		return 1;
	}


	int CEncodedFrameDepacketizer::GetReceivedFrame(unsigned char* data, int &nFramNumber, int &nEcodingTime, int nExpectedTime, int nRight, int &nOrientation)
	{
		DepacketizerLocker lock(*m_pEncodedFrameDepacketizerMutex);

		if (m_FrontFrame > m_iMaxFrameNumRecvd)	//BUFFER IS EMPTY
			return -1;

		nEcodingTime = GetEncodingTime(m_FrontFrame);
		nOrientation = GetOrientation(m_FrontFrame);

		//	CLogPrinter_WriteSpecific(CLogPrinter::DEBUGS,
		//							   " GetReceivedFrame : Fron: " +
		//							   m_Tools.IntegertoStringConvert(m_FrontFrame)+" Back: "+m_Tools.IntegertoStringConvert(m_BackFrame) +"  Rng: [ "+m_Tools.IntegertoStringConvert(m_iFirstFrameReceived)+" @ "
		//							   +m_Tools.IntegertoStringConvert(m_iMaxFrameNumRecvd)+"] Time# Ex: "+m_Tools.IntegertoStringConvert(nExpectedTime)
		//							   + " En: "+m_Tools.IntegertoStringConvert(nEcodingTime));

		/*CLogPrinter_WriteSpecific5(CLogPrinter::DEBUGS,
								   " GetReceivedFrame: Fron: " +
								   m_Tools.IntegertoStringConvert(m_FrontFrame)+" Back: "+m_Tools.IntegertoStringConvert(m_BackFrame) +" Time#  "+m_Tools.IntegertoStringConvert(nExpectedTime)
								   + " > "+m_Tools.IntegertoStringConvert(nEcodingTime));

								   */
		//	CLogPrinter_WriteSpecific(CLogPrinter::DEBUGS,
		//							   " GetReceivedFrame : Encoding time: "+m_Tools.IntegertoStringConvert(nEcodingTime));
		int nFrameLength = -1;
		int frameNumber = -1;

		int index = SafeFinder(m_FrontFrame);

#ifdef CRASH_CHECK
		if (index == -1)
			return -1;
#endif

		int isCompleteFrame = m_CVideoPacketBuffer[index].IsComplete();

		if (m_bIsDpkgBufferFilledUp)		// 300 Milliseconds delay has preserved.
		{
			// No packet of this frame is found and it is not the only frame of buffer.
			if (nEcodingTime == -1 && m_FrontFrame < m_iMaxFrameNumRecvd)	{
				CLogPrinter_WriteLog(CLogPrinter::DEBUGS, DEPACKETIZATION_LOG, "No Pkt Found Dropped-----> " + m_Tools.IntegertoStringConvert(m_FrontFrame) + "  ExpTime: " + m_Tools.IntegertoStringConvert(nExpectedTime));
				//			g_FPSController.NotifyFrameDropped(m_FrontFrame);
				MoveForward(m_FrontFrame);	//Drop the frame
				//asdf
				m_VideoCallSession->CreateAndSend_IDR_Frame_Info_Packet(m_FrontFrame);

				return -1;
			}
			//At least one packet is found and it is not the right time to send this frame to decoder.
			if (-1 != nExpectedTime && nEcodingTime > nExpectedTime)
			{
				return -1;
			}
		}

		// Before 300 completing milliseconds.
		if (!m_bIsDpkgBufferFilledUp) {
			long long llCurrentTimeStamp = Tools::CurrentTimestamp();
			if (m_iFirstFrameReceived != DEFAULT_FIRST_FRAME_RCVD && TIME_DELAY_FOR_RETRANSMISSION_IN_MS <= llCurrentTimeStamp - m_VideoCallSession->GetFirstVideoPacketTime()) {
				for (int frame = m_FrontFrame; frame < m_iFirstFrameReceived; ++frame)
				{
					MoveForward(frame);
					CLogPrinter_WriteFileLog(CLogPrinter::INFO, WRITE_TO_LOG_FILE, "CEncodedFrameDepacketizer::GetReceivedFrame 1");
				}
				m_bIsDpkgBufferFilledUp = true;
				CLogPrinter_WriteSpecific(CLogPrinter::DEBUGS,
					" GetReceivedFrame# : ####################################################### FirstFrame " + m_Tools.IntegertoStringConvert(m_iFirstFrameReceived)
					+ " LastFram: " + m_Tools.IntegertoStringConvert(m_iMaxFrameNumRecvd));
			}
			return -1;
		}

		//Complete front frame and it is the right time to send the front frame to decoder.
		if (isCompleteFrame)
		{
			nFrameLength = ProcessFrame(data, index, m_FrontFrame, nFramNumber);
			//		CLogPrinter_WriteSpecific(CLogPrinter::DEBUGS, " GetReceivedFrame# : Complete Frame EncodingTime "+m_Tools.IntegertoStringConvert(nEcodingTime)+"  Frame: "+m_Tools.IntegertoStringConvert(nFramNumber));
			return nFrameLength;
		}
		else if (m_FrontFrame < m_iMaxFrameNumRecvd)		// Front frame is not complete and it is not the only frame of buffer.
		{
			CLogPrinter_WriteLog(CLogPrinter::DEBUGS, DEPACKETIZATION_LOG, "Incomplete Frame Dropped-----> " + m_Tools.IntegertoStringConvert(m_FrontFrame) + "  ExpTime: " + m_Tools.IntegertoStringConvert(nExpectedTime));
			//		g_FPSController.NotifyFrameDropped(m_FrontFrame);
			MoveForward(m_FrontFrame);
			//asdf
			m_VideoCallSession->CreateAndSend_IDR_Frame_Info_Packet(m_FrontFrame);

			CLogPrinter_WriteFileLog(CLogPrinter::INFO, WRITE_TO_LOG_FILE, "CEncodedFrameDepacketizer::GetReceivedFrame 2");
			return -1;
		}
		return -1;
	}

	int CEncodedFrameDepacketizer::ProcessFrame(unsigned char *data, int index, int frameNumber, int &nFramNumber){
		memcpy(data, m_CVideoPacketBuffer[index].m_ucaFrameData, m_CVideoPacketBuffer[index].m_nFrameSize);		//Send I-Frame
		int nFrameLength = m_CVideoPacketBuffer[index].m_nFrameSize;
		nFramNumber = frameNumber;
		//	g_FPSController.NotifyFrameComplete(frameNumber);
		MoveForward(frameNumber);

		return nFrameLength;
	}


	int CEncodedFrameDepacketizer::SafeFinder(int Data)
	{
#ifdef CRASH_CHECK
		std::map<int, int>::iterator it = m_FrameTracker.find(Data);
		if (it == m_FrameTracker.end())
		{
			CLogPrinter_WriteSpecific(CLogPrinter::DEBUGS, "To learn how to use maps: search Isearch.SafeFinder.net. SafeFinder:: Invalid Index,");
			return -1;
		}
		int index = it->second;
		if (0 <= index && index <= DEPACKETIZATION_BUFFER_SIZE)
			return index;
		else
		{
			CLogPrinter_WriteSpecific(CLogPrinter::DEBUGS, "To learn how to use maps: search Isearch.SafeFinder.net. SafeFinder:: Index out of range. Index : " + Tools::IntegertoStringConvert(index));
			return -1;
		}
#else
		return m_FrameTracker.find(Data)->second;
#endif
	}

	void CEncodedFrameDepacketizer::MoveForward(int frame)
	{
		int indexInside = SafeFinder(frame);
		if (indexInside == -1)
			return;

		ClearFrame(indexInside, frame);

		m_FrontFrame++;

		//	m_BackFrame++;
		//	CreateNewIndex(m_BackFrame);
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

	void CEncodedFrameDepacketizer::ClearFrame(int index, int frame)
	{
		if (-1 < index && index <= DEPACKETIZATION_BUFFER_SIZE)
		{
			if (m_mFrameTimeStamp.find(frame) != m_mFrameTimeStamp.end())
				m_mFrameTimeStamp.erase(frame);

			if (m_mFrameOrientation.find(frame) != m_mFrameOrientation.end())
				m_mFrameOrientation.erase(frame);

			m_CVideoPacketBuffer[index].Reset();
			m_AvailableIndexes.insert(index);
			m_FrameTracker.erase(frame);
		}
		else{
			CLogPrinter_WriteSpecific(CLogPrinter::DEBUGS, "CEncodedFrameDepacketizer::ClearFrame, InvalidIndex = " + m_Tools.IntegertoStringConvert(index) + ", frame = " + m_Tools.IntegertoStringConvert(frame) + "###############################");
		}
	}

	int CEncodedFrameDepacketizer::GetEncodingTime(int nFrameNumber)
	{
		if (m_mFrameTimeStamp.find(nFrameNumber) != m_mFrameTimeStamp.end())
			return m_mFrameTimeStamp[nFrameNumber];
		return -1;
	}

	int CEncodedFrameDepacketizer::GetOrientation(int nFrameNumber)
	{
		if (m_mFrameOrientation.find(nFrameNumber) != m_mFrameOrientation.end())
			return m_mFrameOrientation[nFrameNumber];
		return -1;
	}

} //namespace MediaSDK
