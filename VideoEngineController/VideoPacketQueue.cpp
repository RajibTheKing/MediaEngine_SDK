
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
	/*if (m_pChannelMutex.get())
		m_pChannelMutex.reset();*/
	SHARED_PTR_DELETE(m_pChannelMutex);

#ifdef RETRANSMISSION_ENABLED
	m_FrameInQueue.clear();
#endif
}

int CVideoPacketQueue::Queue(unsigned char *frame, int length)
{
	Locker lock(*m_pChannelMutex);
#ifdef	RETRANSMISSION_ENABLED
	Tools tools;
	int frameNumber = tools.GetIntFromChar(frame,1,3);
	int pktNumber = frame[5];

	m_FrameInQueue.insert(frameNumber*MAX_NUMBER_OF_PACKETS + pktNumber);
#endif

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

#ifdef	RETRANSMISSION_ENABLED
bool CVideoPacketQueue::PacketExists(int iFrameNUmber, int iPacketNumber)
{
	Locker lock(*m_pChannelMutex);	
	bool flag = (m_FrameInQueue.find(iFrameNUmber*MAX_NUMBER_OF_PACKETS+iPacketNumber)!=m_FrameInQueue.end());
	
	return flag;
}
#endif

int CVideoPacketQueue::DeQueue(unsigned char *decodeBuffer)
{
	Locker lock(*m_pChannelMutex);
#ifdef	RETRANSMISSION_ENABLED
	int frameNumber = Tools::GetIntFromChar(decodeBuffer,1,3);
	int pktNumber = decodeBuffer[5];

#ifdef CRASH_CHECK
	if(m_FrameInQueue.find(frameNumber*MAX_NUMBER_OF_PACKETS + pktNumber) != m_FrameInQueue.end())
#endif
		m_FrameInQueue.erase(frameNumber*MAX_NUMBER_OF_PACKETS + pktNumber);
#endif

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