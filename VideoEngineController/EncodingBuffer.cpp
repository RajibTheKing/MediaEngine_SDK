
#include "EncodingBuffer.h"

#include <string.h>
#include "LogPrinter.h"

CEncodingBuffer::CEncodingBuffer() :
m_iPushIndex(0),
m_iPopIndex(0),
m_iQueueSize(0),
m_iQueueCapacity(45)
{
	m_pChannelMutex.reset(new CLockHandler);
}

CEncodingBuffer::~CEncodingBuffer()
{
	/*if (m_pChannelMutex.get())
		m_pChannelMutex.reset();*/
}

int CEncodingBuffer::Queue(unsigned char *frame, int length)
{
    Locker lock(*m_pChannelMutex);
    
    memcpy(m_Buffer[m_iPushIndex], frame, length);
    m_BufferDataLength[m_iPushIndex] = length;
    
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

int CEncodingBuffer::DeQueue(unsigned char *decodeBuffer)
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

void CEncodingBuffer::IncreamentIndex(int &index)
{
	index++;

	if (index >= m_iQueueCapacity)
		index = 0;
}

int CEncodingBuffer::GetQueueSize()
{
	Locker lock(*m_pChannelMutex);

	return m_iQueueSize;
}