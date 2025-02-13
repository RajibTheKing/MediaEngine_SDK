
#include "VideoPacketQueue.h"
#include "ThreadTools.h"
#include "LogPrinter.h"

namespace MediaSDK
{

	CVideoPacketQueue::CVideoPacketQueue() :

		m_iPushIndex(0),
		m_iPopIndex(0),
		m_nQueueSize(0),
		m_nMaxQueueSizeTillNow(0),
		m_nQueueCapacity(MAX_VIDEO_PACKET_QUEUE_SIZE)

	{
		m_pVideoPacketQueueMutex.reset(new CLockHandler);
	}

	CVideoPacketQueue::~CVideoPacketQueue()
	{
		SHARED_PTR_DELETE(m_pVideoPacketQueueMutex);
	}

	void CVideoPacketQueue::ResetBuffer()
	{
		PacketQueueLocker lock(*m_pVideoPacketQueueMutex);

		m_iPushIndex = 0;
		m_iPopIndex = 0;
		m_nQueueSize = 0;
	}

	int CVideoPacketQueue::Queue(unsigned char *ucaVideoPacketData, int nLength)
	{
		PacketQueueLocker lock(*m_pVideoPacketQueueMutex);

		if (m_nQueueSize > m_nMaxQueueSizeTillNow)
			m_nMaxQueueSizeTillNow = m_nQueueSize;

		CLogPrinter_LOG(BUFFER_SIZE_LOG, "CVideoPacketQueue::Queue VIDEO PACKET or MINI PACKET Buffer size %d m_nMaxQueueSizeTillNow %d m_nQueueCapacity %d", m_nQueueSize, m_nMaxQueueSizeTillNow, m_nQueueCapacity);

		if (m_nQueueSize >= m_nQueueCapacity)
		{
			CLogPrinter_LOG(QUEUE_OVERFLOW_LOG, "CVideoPacketQueue::Queue VIDEO PACKET or MINI PACKET Buffer OVERFLOW m_nQueueSize %d m_nQueueCapacity %d", m_nQueueSize, m_nQueueCapacity);
			return -1;
		}
		else
		{
			//	if (nLength >= MAX_VIDEO_PACKET_SIZE)
			//		return -1;

			memcpy(m_uc2aVideoPacketBuffer[m_iPushIndex], ucaVideoPacketData, nLength);

			m_naBufferDataLengths[m_iPushIndex] = nLength;

			IncreamentIndex(m_iPushIndex);
			m_nQueueSize++;

			return 1;
		}
	}

	int CVideoPacketQueue::DeQueue(unsigned char *ucaVideoPacketData)
	{
		PacketQueueLocker lock(*m_pVideoPacketQueueMutex);

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

	void CVideoPacketQueue::IncreamentIndex(int &riIndex)
	{
		riIndex++;

		if (riIndex >= m_nQueueCapacity)
			riIndex = 0;
	}

	int CVideoPacketQueue::GetQueueSize()
	{
		PacketQueueLocker lock(*m_pVideoPacketQueueMutex);

		return m_nQueueSize;
	}

} //namespace MediaSDK
