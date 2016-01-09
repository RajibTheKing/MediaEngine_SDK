
#include "DecodingBuffer.h"

#include <string.h>

CDecodingBuffer::CDecodingBuffer() :

m_iPushIndex(0),
m_iPopIndex(0),
m_iQueueSize(0),
m_iQueueCapacity(30)

{
	m_pChannelMutex.reset(new CLockHandler);
}

CDecodingBuffer::~CDecodingBuffer()
{
/*	if (m_pChannelMutex.get())
		m_pChannelMutex.reset();*/
}

int CDecodingBuffer::Queue(int frameNumber, unsigned char *frame, int length, unsigned int timeStampDiff)
{
    
    Locker lock(*m_pChannelMutex);
    
    memcpy(m_Buffer[m_iPushIndex], frame, length);
    m_BufferDataLength[m_iPushIndex] = length;
    m_BufferFrameNumber[m_iPushIndex] = frameNumber;

	m_BufferTimeStamp[m_iPushIndex] = timeStampDiff;
    
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

int CDecodingBuffer::DeQueue(int &frameNumber, unsigned int &timeStampDiff, unsigned char *decodeBuffer)
{
	Locker lock(*m_pChannelMutex);

	if (m_iQueueSize <= 0)
	{
		return -1;
	}
	else
	{
		int length = m_BufferDataLength[m_iPopIndex];

		frameNumber = m_BufferFrameNumber[m_iPopIndex];

		timeStampDiff = m_BufferTimeStamp[m_iPopIndex];

		memcpy(decodeBuffer, m_Buffer[m_iPopIndex], length);

		IncreamentIndex(m_iPopIndex);

		m_iQueueSize--;

		return length;
	}
}

void CDecodingBuffer::IncreamentIndex(int &index)
{
	index++;

	if (index >= m_iQueueCapacity)
		index = 0;
}

int CDecodingBuffer::GetQueueSize()
{
	Locker lock(*m_pChannelMutex);

	return m_iQueueSize;
}