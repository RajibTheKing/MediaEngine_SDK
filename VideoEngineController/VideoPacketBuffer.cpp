#include "VideoPacketBuffer.h"

CVideoPacketBuffer::CVideoPacketBuffer():
m_NumberOfGotPackets(0),
m_NumberOfPackets(MAX_NUMBER_OF_PACKETS),
m_FrameSize(0),
m_isClear(true)
{
	memset(m_pPacketTracker, 0, MAX_NUMBER_OF_PACKETS);
}

CVideoPacketBuffer::~CVideoPacketBuffer()
{

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
			nPacketDataLength = (in_size-1) - PACKET_HEADER_LENGTH;
			memcpy(m_pFrameData + packetNumber * ( MAX_VIDEO_PACKET_SIZE - PACKET_HEADER_LENGTH - 1),
				   in_data + PACKET_HEADER_LENGTH, nPacketDataLength);
		}
		else {
//			nPacketDataLength = in_size - PACKET_HEADER_LENGTH_NO_VERSION;
			nPacketDataLength = in_size;
			memcpy(m_pFrameData + packetNumber * MAX_PACKET_SIZE_WITHOUT_HEADER_NO_VERSION,
				   in_data + PACKET_HEADER_LENGTH_NO_VERSION, nPacketDataLength);
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







