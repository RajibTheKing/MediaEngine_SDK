
#include "DeviceCapabilityCheckBuffer.h"
#include "LogPrinter.h"

namespace MediaSDK
{

	CDeviceCapabilityCheckBuffer::CDeviceCapabilityCheckBuffer() :

		m_iPushIndex(0),
		m_iPopIndex(0),
		m_nQueueSize(0),
		m_nMaxQueueSizeTillNow(0),
		m_nQueueCapacity(MAX_VIDEO_PACKET_SENDING_BUFFER_SIZE)

	{
		m_pDeviceCapabilityCheckBufferMutex.reset(new CLockHandler);
	}

	CDeviceCapabilityCheckBuffer::~CDeviceCapabilityCheckBuffer()
	{

	}

	void CDeviceCapabilityCheckBuffer::ResetBuffer()
	{
		CapabilityLocker lock(*m_pDeviceCapabilityCheckBufferMutex);

		m_iPushIndex = 0;
		m_iPopIndex = 0;
		m_nQueueSize = 0;
	}

	int CDeviceCapabilityCheckBuffer::Queue(long long llFriendID, int nOperation, int nNotification, int nVideoHeight, int nVideoWidth)
	{
		//printf("DeviceCapabilityCheckBuffer QUEUE Locked\n");
		CapabilityLocker lock(*m_pDeviceCapabilityCheckBufferMutex);
		//printf("DeviceCapabilityCheckBuffer QUEUE UN--Locked\n");

		if (m_nQueueSize > m_nMaxQueueSizeTillNow)
			m_nMaxQueueSizeTillNow = m_nQueueSize;

		CLogPrinter_LOG(BUFFER_SIZE_LOG, "CDeviceCapabilityCheckBuffer::Queue CHECK CAPABILITY Buffer size %d m_nMaxQueueSizeTillNow %d m_nQueueCapacity %d", m_nQueueSize, m_nMaxQueueSizeTillNow, m_nQueueCapacity);

		m_naBufferOperations[m_iPushIndex] = nOperation;
		m_llaBufferFriendIDs[m_iPushIndex] = llFriendID;
		m_naBufferNotifications[m_iPushIndex] = nNotification;
		m_naBufferVideoHeights[m_iPushIndex] = nVideoHeight;
		m_naBufferVideoWidths[m_iPushIndex] = nVideoWidth;

		if (m_nQueueSize == m_nQueueCapacity)
		{
			IncreamentIndex(m_iPopIndex);

			CLogPrinter_LOG(QUEUE_OVERFLOW_LOG, "CDeviceCapabilityCheckBuffer::Queue CHECK CAPABILITY Buffer OVERFLOW m_nQueueSize %d m_nQueueCapacity %d", m_nQueueSize, m_nQueueCapacity);
		}
		else
		{
			m_nQueueSize++;
		}

		IncreamentIndex(m_iPushIndex);

		return 1;
	}

	int CDeviceCapabilityCheckBuffer::DeQueue(long long &llrFriendID, int &nrNotification, int &nrVideoHeight, int &nrVideoWidth)
	{
		//printf("DeviceCapabilityCheckBuffer DeQUEUE Locked\n");
		CapabilityLocker lock(*m_pDeviceCapabilityCheckBufferMutex);
		//printf("DeviceCapabilityCheckBuffer DeQUEUE Un--Locked\n");

		if (m_nQueueSize <= 0)
		{
			return -1;
		}
		else
		{
			int nOperation;

			nOperation = m_naBufferOperations[m_iPopIndex];
			llrFriendID = m_llaBufferFriendIDs[m_iPopIndex];
			nrNotification = m_naBufferNotifications[m_iPopIndex];
			nrVideoHeight = m_naBufferVideoHeights[m_iPopIndex];
			nrVideoWidth = m_naBufferVideoWidths[m_iPopIndex];

			IncreamentIndex(m_iPopIndex);
			m_nQueueSize--;

			return nOperation;
		}
	}

	void CDeviceCapabilityCheckBuffer::IncreamentIndex(int &irIndex)
	{
		irIndex++;

		if (irIndex >= m_nQueueCapacity)
			irIndex = 0;
	}

	int CDeviceCapabilityCheckBuffer::GetQueueSize()
	{
		CapabilityLocker lock(*m_pDeviceCapabilityCheckBufferMutex);

		return m_nQueueSize;
	}

} //namespace MediaSDK
