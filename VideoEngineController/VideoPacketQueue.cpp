
#include "VideoPacketQueue.h"
#include "ThreadTools.h"

#include <string.h>

CVideoPacketQueue::CVideoPacketQueue() :

m_iPushIndex(0),
m_iPopIndex(0),
m_nQueueSize(0),
m_nQueueCapacity(MAX_VIDEO_PACKET_QUEUE_SIZE)

{
	m_pVideoPacketQueueMutex.reset(new CLockHandler);
}

CVideoPacketQueue::~CVideoPacketQueue()
{
	SHARED_PTR_DELETE(m_pVideoPacketQueueMutex);
}

int CVideoPacketQueue::Queue(unsigned char *ucaVideoPacketData, int nLength)
{
	Locker lock(*m_pVideoPacketQueueMutex);

	if (m_nQueueSize >= m_nQueueCapacity)
	{
		return -1;
	}
	else
	{
		memcpy(m_uc2aVideoPacketBuffer[m_iPushIndex], ucaVideoPacketData, nLength);

		m_naBufferDataLengths[m_iPushIndex] = nLength;

		IncreamentIndex(m_iPushIndex);
		m_nQueueSize++;

		return 1;
	}
}

int CVideoPacketQueue::DeQueue(unsigned char *ucaVideoPacketData)
{
	Locker lock(*m_pVideoPacketQueueMutex);

	if (m_nQueueSize <= 0)
	{
		return -1;
	}
	else
	{
		int nLength;
			
		nLength = m_naBufferDataLengths[m_iPopIndex];

		memcpy(ucaVideoPacketData, m_uc2aVideoPacketBuffer[m_iPopIndex], nLength);

		IncreamentIndex(m_iPopIndex);
		m_nQueueSize--;

		return nLength;
	}
}

void CVideoPacketQueue::IncreamentIndex(int &irIndex)
{
	irIndex++;

	if (irIndex >= m_nQueueCapacity)
		irIndex = 0;
}

int CVideoPacketQueue::GetQueueSize()
{
	Locker lock(*m_pVideoPacketQueueMutex);

	return m_nQueueSize;
}