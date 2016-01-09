
#include "ResendingBuffer.h"

#include <string.h>
#include "LogPrinter.h"

CResendingBuffer::CResendingBuffer() :

m_iPushIndex(0),
m_iPopIndex(0),
m_iQueueSize(0),
m_iQueueCapacity(300)

{
	m_pChannelMutex.reset(new CLockHandler);
}

CResendingBuffer::~CResendingBuffer()
{
/*	if (m_pChannelMutex.get())
		m_pChannelMutex.reset();*/
}

void CResendingBuffer::Queue(unsigned char *frame, int length, int frameNumber, int packetNumber)
{
	CLogPrinter::WriteSpecific(CLogPrinter::INFO, "CResendingBuffer::Queue b4 lock");
	Locker lock(*m_pChannelMutex);
	CLogPrinter::WriteSpecific(CLogPrinter::INFO, "CResendingBuffer::Queue 8r lock");

    memcpy(m_Buffer[m_iPushIndex], frame, length);

    m_BufferDataLength[m_iPushIndex] = length;
    
    reverseResendingMapIterator = reverseResendingMap.find(m_iPushIndex);
    resendingMapIterator = resendingMap.find(reverseResendingMapIterator->second);
    
    if(resendingMapIterator != resendingMap.end())
        resendingMap.erase(resendingMapIterator);
    
    resendingMap[std::make_pair(frameNumber, packetNumber)] = m_iPushIndex;
    reverseResendingMap[m_iPushIndex] = std::make_pair(frameNumber, packetNumber);

    IncreamentIndex(m_iPushIndex);

    if (m_iQueueSize != m_iQueueCapacity)
        m_iQueueSize++;
}

int CResendingBuffer::DeQueue(unsigned char *decodeBuffer, int frameNumber, int packetNumber)
{
	CLogPrinter::WriteSpecific(CLogPrinter::INFO, "CResendingBuffer::DeQueue b4 lock");
	Locker lock(*m_pChannelMutex);
	CLogPrinter::WriteSpecific(CLogPrinter::INFO, "CResendingBuffer::DeQueue 8r lock");

	if (m_iQueueSize <= 0)
	{
		return -1;
	}
	else
	{
        resendingMapIterator = resendingMap.find(std::make_pair(frameNumber, packetNumber));
        
        if(resendingMapIterator == resendingMap.end())
            return -1;
        
        m_iPopIndex = resendingMapIterator->second;
        
		int length = m_BufferDataLength[m_iPopIndex];

		memcpy(decodeBuffer, m_Buffer[m_iPopIndex], length);

		m_iQueueSize--;
        
        resendingMap.erase(resendingMapIterator);
        
        reverseResendingMapIterator = reverseResendingMap.find(m_iPopIndex);
        
        if(reverseResendingMapIterator != reverseResendingMap.end())
            reverseResendingMap.erase(reverseResendingMapIterator);

		return length;
	}
}

void CResendingBuffer::IncreamentIndex(int &index)
{
	index++;

	if (index >= m_iQueueCapacity)
		index = 0;
}

int CResendingBuffer::GetQueueSize()
{
	Locker lock(*m_pChannelMutex);

	return m_iQueueSize;
}