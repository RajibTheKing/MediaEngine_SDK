
#include "SendingBuffer.h"

#include <string.h>
#include "LogPrinter.h"

CSendingBuffer::CSendingBuffer() :

m_iPushIndex(0),
m_iPopIndex(0),
m_iQueueSize(0),
m_iQueueCapacity(MAX_VIDEO_PACKET_SENDING_BUFFER_SIZE)

{
	m_pChannelMutex.reset(new CLockHandler);
}

CSendingBuffer::~CSendingBuffer()
{
/*	if (m_pChannelMutex.get())
		m_pChannelMutex.reset();*/
}

int CSendingBuffer::Queue(LongLong lFriendID, unsigned char *frame, int length)
{
//	CLogPrinter_WriteSpecific(CLogPrinter::INFO, "CSendingBuffer::Queue b4 lock");
	Locker lock(*m_pChannelMutex);
    
    memcpy(m_Buffer[m_iPushIndex], frame, length);
    m_BufferDataLength[m_iPushIndex] = length;
    m_BufferFrameNumber[m_iPushIndex] = lFriendID;
    

	if (m_iQueueSize == m_iQueueCapacity)
	{
		IncreamentIndex(m_iPopIndex);
	}
	else
	{

		m_iQueueSize++;
		
	}
    
    IncreamentIndex(m_iPushIndex);
    
    return 1;
}

int CSendingBuffer::DeQueue(LongLong &lFriendID, unsigned char *decodeBuffer)
{
//	CLogPrinter_WriteSpecific(CLogPrinter::INFO, "CSendingBuffer::deQueue b4 lock");
	Locker lock(*m_pChannelMutex);
//	CLogPrinter_WriteSpecific(CLogPrinter::INFO, "CSendingBuffer::deQueue 84 lock");

	if (m_iQueueSize <= 0)
	{
		return -1;
	}
	else
	{
		int length = m_BufferDataLength[m_iPopIndex];

		lFriendID = m_BufferFrameNumber[m_iPopIndex];

		memcpy(decodeBuffer, m_Buffer[m_iPopIndex], length);

		IncreamentIndex(m_iPopIndex);

		m_iQueueSize--;

		return length;
	}
}

void CSendingBuffer::IncreamentIndex(int &index)
{
	index++;

	if (index >= m_iQueueCapacity)
		index = 0;
}

int CSendingBuffer::GetQueueSize()
{
	Locker lock(*m_pChannelMutex);

	return m_iQueueSize;
}