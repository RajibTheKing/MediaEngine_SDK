
#include "VideoPacketBuffer.h"
#include "HashGenerator.h"
#include "VideoHeader.h"

CVideoPacketBuffer::CVideoPacketBuffer():
m_nNumberOfGotPackets(0),
m_nNumberOfPackets(MAX_NUMBER_OF_PACKETS),
m_nFrameSize(0),
m_bIsClear(true)

{
	memset(m_baPacketTracker, 0, MAX_NUMBER_OF_PACKETS);
}

CVideoPacketBuffer::~CVideoPacketBuffer()
{

}

void CVideoPacketBuffer::Reset()
{
	if (!m_bIsClear)
	{
		memset(m_baPacketTracker, 0, MAX_NUMBER_OF_PACKETS);
		m_nNumberOfPackets = MAX_NUMBER_OF_PACKETS;
		m_nNumberOfGotPackets = 0;
		m_bIsClear = true;
		m_nFrameSize= 0;
	}
}

bool CVideoPacketBuffer::PushVideoPacket(unsigned char *pucVideoPacketData, unsigned int unLength, int nPacketNumber, int iHeaderLength, int nPacketStartingIndex)
{
	
	if (false == m_baPacketTracker[nPacketNumber])
	{
		int nPacketDataLength = unLength;

		m_baPacketTracker[nPacketNumber] = true;
		m_nNumberOfGotPackets++;

		memcpy(m_ucaFrameData + nPacketStartingIndex, pucVideoPacketData + iHeaderLength, nPacketDataLength);

        
		m_nFrameSize += nPacketDataLength;

		return (m_nNumberOfGotPackets == m_nNumberOfPackets);
	}
	else
		return false;
}

int CVideoPacketBuffer::IsComplete()
{
	if (m_nNumberOfGotPackets == m_nNumberOfPackets)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

void CVideoPacketBuffer::SetNumberOfPackets(int nNumberOfPackets)
{
	m_bIsClear = false;
	m_nNumberOfPackets = nNumberOfPackets;
}


void CVideoPacketBuffer::SetFrameNumber(int nFrameNumber)
{
    m_nFrameNumber = nFrameNumber;
}






