
#include "VideoPacketBuffer.h"
#include "HashGenerator.h"
#include "VideoHeader.h"

#define USE_HASH_GENERATOR_TO_DEPACKETIZE

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

bool CVideoPacketBuffer::PushVideoPacket(unsigned char *pucVideoPacketData, unsigned int unLength, int nPacketNumber, int iHeaderLength)
{
	
	if (false == m_baPacketTracker[nPacketNumber])
	{
		int nPacketDataLength = unLength;

		m_baPacketTracker[nPacketNumber] = true;
		m_nNumberOfGotPackets++;
        
        
#ifdef USE_HASH_GENERATOR_TO_DEPACKETIZE
        CVideoHeader packetHeader;
        packetHeader.setPacketHeader(pucVideoPacketData);
        //packetHeader.ShowDetails("ReceivingSide: ");
        
		memcpy(m_ucaFrameData + packetHeader.GetPacketStartingIndex(), pucVideoPacketData + iHeaderLength, nPacketDataLength);
#else
        
        memcpy(m_ucaFrameData + nPacketNumber * MAX_PACKET_SIZE_WITHOUT_HEADER, pucVideoPacketData + iHeaderLength, nPacketDataLength);
#endif
        
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






