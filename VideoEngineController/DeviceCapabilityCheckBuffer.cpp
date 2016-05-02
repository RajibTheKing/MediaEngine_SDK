
#include "DeviceCapabilityCheckBuffer.h"
#include "LogPrinter.h"

CDeviceCapabilityCheckBuffer::CDeviceCapabilityCheckBuffer() :

m_iPushIndex(0),
m_iPopIndex(0),
m_nQueueSize(0),
m_nQueueCapacity(MAX_VIDEO_PACKET_SENDING_BUFFER_SIZE)

{
	m_pDeviceCapabilityCheckBufferMutex.reset(new CLockHandler);
}

CDeviceCapabilityCheckBuffer::~CDeviceCapabilityCheckBuffer()
{

}

void CDeviceCapabilityCheckBuffer::ResetBuffer()
{
	Locker lock(*m_pDeviceCapabilityCheckBufferMutex);

	m_iPushIndex = 0;
	m_iPopIndex = 0;
	m_nQueueSize = 0;
}

int CDeviceCapabilityCheckBuffer::Queue(LongLong llFriendID, int nOperation, int nVideoHeight, int nVideoWidth)
{
	Locker lock(*m_pDeviceCapabilityCheckBufferMutex);

	m_naBufferOperations[m_iPushIndex] = nOperation;
	m_llaBufferFriendIDs[m_iPushIndex] = llFriendID;
	m_naBufferVideoHeights[m_iPushIndex] = nVideoHeight;
	m_naBufferVideoWidths[m_iPushIndex] = nVideoWidth;

	if (m_nQueueSize == m_nQueueCapacity)
	{
		IncreamentIndex(m_iPopIndex);
	}
	else
	{
		m_nQueueSize++;
	}

	IncreamentIndex(m_iPushIndex);

	return 1;
}

int CDeviceCapabilityCheckBuffer::DeQueue(LongLong &llrFriendID, int &nrVideoHeight, int &nrVideoWidth)
{
	Locker lock(*m_pDeviceCapabilityCheckBufferMutex);

	if (m_nQueueSize <= 0)
	{
		return -1;
	}
	else
	{
		int nOperation;

		nOperation = m_naBufferOperations[m_iPopIndex];
		llrFriendID = m_llaBufferFriendIDs[m_iPopIndex];
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
	Locker lock(*m_pDeviceCapabilityCheckBufferMutex);

	return m_nQueueSize;
}