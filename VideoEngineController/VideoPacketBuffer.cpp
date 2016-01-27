#include "VideoPacketBuffer.h"
#include "CommonElementsBucket.h"
#include "LogPrinter.h"
#include "Tools.h"


CVideoPacketBuffer::CVideoPacketBuffer():
m_NumberOfGotPackets(0),
m_NumberOfPackets(MAX_NUMBER_OF_PACKETS),
m_FrameSize(0),
m_isClear(true)
{
	memset(m_pPacketTracker, 0, MAX_NUMBER_OF_PACKETS);

	CLogPrinter_Write(CLogPrinter::INFO, "CVideoPacketBuffer::CVideoPacketBuffer");
}

CVideoPacketBuffer::~CVideoPacketBuffer()
{
/*	if (m_pEncodedFrameDepacketizerThread)
	{
		delete m_pEncodedFrameDepacketizerThread;
		m_pEncodedFrameDepacketizerThread = NULL;
	}

	SHARED_PTR_DELETE(m_pEncodedFrameDepacketizerMutex);*/
}

void CVideoPacketBuffer::Reset()
{
	if (!m_isClear)
	{
		memset(m_pPacketTracker, 0, MAX_NUMBER_OF_PACKETS);
		m_NumberOfPackets = MAX_NUMBER_OF_PACKETS;
		m_NumberOfGotPackets = 0;
		m_isClear = true;
		m_FrameSize= 0;
	}
}

bool CVideoPacketBuffer::PushVideoPacket(unsigned char *in_data, unsigned int in_size, int packetNumber)
{
	if (false == m_pPacketTracker[packetNumber])
	{
		int nPacketDataLength;
		m_pPacketTracker[packetNumber] = true;
		m_NumberOfGotPackets++;

		if(in_data[VERSION_BYTE_INDEX]) {
			nPacketDataLength = in_size - PACKET_HEADER_LENGTH;
			memcpy(m_pFrameData + packetNumber * ( MAX_VIDEO_PACKET_SIZE - PACKET_HEADER_LENGTH - 1),
				   in_data + PACKET_HEADER_LENGTH, nPacketDataLength);
		}
		else {
//			nPacketDataLength = in_size - PACKET_HEADER_LENGTH_NO_VERSION;
			nPacketDataLength = in_size - PACKET_HEADER_LENGTH_NO_VERSION;
			memcpy(m_pFrameData + packetNumber * ( MAX_VIDEO_PACKET_SIZE - PACKET_HEADER_LENGTH_NO_VERSION - 1),
				   in_data + PACKET_HEADER_LENGTH_NO_VERSION,in_size);
		}

		m_FrameSize += nPacketDataLength;

		return (m_NumberOfGotPackets == m_NumberOfPackets);
	}
	else
		return false;
}

int CVideoPacketBuffer::IsComplete()
{
	if (m_NumberOfGotPackets == m_NumberOfPackets)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

void CVideoPacketBuffer::SetNumberOfPackets(int number)
{
	m_isClear = false;
	m_NumberOfPackets = number;
}

bool CVideoPacketBuffer::IsIFrame(){
	return (m_pFrameData[4] & 0x1F)==7;
}







