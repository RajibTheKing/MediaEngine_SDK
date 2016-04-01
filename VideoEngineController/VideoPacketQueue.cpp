
#include "VideoPacketQueue.h"
#include "LogPrinter.h"

#include <string.h>
#include <set>

CVideoPacketQueue::CVideoPacketQueue() :
m_iPushIndex(0),
m_iPopIndex(0),
m_iQueueSize(0),
m_iQueueCapacity(MAX_VIDEO_PACKET_QUEUE_SIZE)
{
	m_pChannelMutex.reset(new CLockHandler);
}

CVideoPacketQueue::~CVideoPacketQueue()
{
	SHARED_PTR_DELETE(m_pChannelMutex);
}

int CVideoPacketQueue::Queue(unsigned char *frame, int length)
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

int CVideoPacketQueue::DeQueue(unsigned char *decodeBuffer)
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

void CVideoPacketQueue::IncreamentIndex(int &index)
{
	index++;

	if (index >= m_iQueueCapacity)
		index = 0;
}

int CVideoPacketQueue::GetQueueSize()
{
	Locker lock(*m_pChannelMutex);

	return m_iQueueSize;
}