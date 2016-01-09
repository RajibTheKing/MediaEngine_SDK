
#include "RetransmitVideoPacketQueue.h"

#include <string.h>

CRetransmitVideoPacketQueue::CRetransmitVideoPacketQueue() :
m_iPushIndex(0),
m_iPopIndex(0),
m_iQueueSize(0),
m_iQueueCapacity(MAX_RETRANS_VIDEO_PACKET_QUEUE_SIZE)
{
	m_pChannelMutex.reset(new CLockHandler);
}

CRetransmitVideoPacketQueue::~CRetransmitVideoPacketQueue()
{
	/*if (m_pChannelMutex.get())
		m_pChannelMutex.reset();*/
}

int CRetransmitVideoPacketQueue::Queue(unsigned char *frame, int length)
{
	Locker lock(*m_pChannelMutex);

	if (m_iQueueSize >= m_iQueueCapacity)
	{
		return -1;
	}
	else
	{
		memcpy(m_Buffer[m_iPushIndex], frame, length);

		m_BufferDataLength[m_iPushIndex] = length;

		IncreamentIndex(m_iPushIndex);

		m_iQueueSize++;

		return 1;
	}
}

bool CRetransmitVideoPacketQueue::PacketExists(int iFrameNUmber, int iPacketNumber)
{
	for(int i = 0; i < m_iQueueSize; i ++)
	{
		pair<int, int> CVideoPacketQueue ;
		int iNumberOfPackets;

		CVideoPacketQueue = Tools::GetFramePacketFromHeader(m_Buffer[i], iNumberOfPackets);
		if(CVideoPacketQueue.first == iFrameNUmber && CVideoPacketQueue.second == iPacketNumber)
		{
			return true;
		}
	}

	return false;

}

int CRetransmitVideoPacketQueue::DeQueue(unsigned char *decodeBuffer)
{
	Locker lock(*m_pChannelMutex);

	if (m_iQueueSize <= 0)
	{
		return -1;
	}
	else
	{
		int length = m_BufferDataLength[m_iPopIndex];

		memcpy(decodeBuffer, m_Buffer[m_iPopIndex], length);

		IncreamentIndex(m_iPopIndex);

		m_iQueueSize--;

		return length;
	}
}

void CRetransmitVideoPacketQueue::IncreamentIndex(int &index)
{
	index++;

	if (index >= m_iQueueCapacity)
		index = 0;
}

int CRetransmitVideoPacketQueue::GetQueueSize()
{
	Locker lock(*m_pChannelMutex);

	return m_iQueueSize;
}