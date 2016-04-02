
#include "SendingBuffer.h"

CSendingBuffer::CSendingBuffer() :

m_iPushIndex(0),
m_iPopIndex(0),
m_nQueueSize(0),
m_nQueueCapacity(MAX_VIDEO_PACKET_SENDING_BUFFER_SIZE)

{
	m_pSendingBufferMutex.reset(new CLockHandler);
}

CSendingBuffer::~CSendingBuffer()
{

}

int CSendingBuffer::Queue(LongLong llFriendID, unsigned char *ucaSendingVideoPacketData, int nLength, int iFrameNumber, int iPacketNumber)
{
	Locker lock(*m_pSendingBufferMutex);
    
	memcpy(m_uc2aSendingVideoPacketBuffer[m_iPushIndex], ucaSendingVideoPacketData, nLength);

	m_naBufferDataLengths[m_iPushIndex] = nLength;
	m_llaBufferFriendIDs[m_iPushIndex] = llFriendID;
	m_naBufferFrameNumbers[m_iPushIndex] = iFrameNumber;
	m_naBufferPacketNumbers[m_iPushIndex] = iPacketNumber;

	m_llBufferInsertionTimes[m_iPushIndex] = m_Tools.CurrentTimestamp();

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

int CSendingBuffer::DeQueue(LongLong &llrFriendID, unsigned char *ucaSendingVideoPacketData, int &nrFrameNumber, int &nrPacketNumber, int &nrTimeDifferenceInQueue)
{
	Locker lock(*m_pSendingBufferMutex);

	if (m_nQueueSize <= 0)
	{
		return -1;
	}
	else
	{
		int nLength;
		
		nLength = m_naBufferDataLengths[m_iPopIndex];
		llrFriendID = m_llaBufferFriendIDs[m_iPopIndex];
		nrFrameNumber = m_naBufferFrameNumbers[m_iPopIndex];
		nrPacketNumber = m_naBufferPacketNumbers[m_iPopIndex];

		memcpy(ucaSendingVideoPacketData, m_uc2aSendingVideoPacketBuffer[m_iPopIndex], nLength);

		nrTimeDifferenceInQueue = m_Tools.CurrentTimestamp() - m_llBufferInsertionTimes[m_iPopIndex];

		IncreamentIndex(m_iPopIndex);
		m_nQueueSize--;

		return nLength;
	}
}

void CSendingBuffer::IncreamentIndex(int &irIndex)
{
	irIndex++;

	if (irIndex >= m_nQueueCapacity)
		irIndex = 0;
}

int CSendingBuffer::GetQueueSize()
{
	Locker lock(*m_pSendingBufferMutex);

	return m_nQueueSize;
}