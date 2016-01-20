
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
	CLogPrinter_WriteSpecific(CLogPrinter::INFO, "CResendingBuffer::Queue b4 lock");
	Locker lock(*m_pChannelMutex);
	CLogPrinter_WriteSpecific(CLogPrinter::INFO, "CResendingBuffer::Queue 8r lock");

    memcpy(m_Buffer[m_iPushIndex], frame, length);
	CLogPrinter_WriteSpecific(CLogPrinter::INFO, "CResendingBuffer::Queue 8r lock 0.1");

    m_BufferDataLength[m_iPushIndex] = length;
	CLogPrinter_WriteSpecific(CLogPrinter::INFO, "CResendingBuffer::Queue 8r lock 0.2");
    
    reverseResendingMapIterator = reverseResendingMap.find(m_iPushIndex);
	CLogPrinter_WriteSpecific(CLogPrinter::INFO, "CResendingBuffer::Queue 8r lock 0.3");

	if (reverseResendingMapIterator != reverseResendingMap.end())
	{
		CLogPrinter_WriteSpecific(CLogPrinter::INFO, "CResendingBuffer::Queue 8r lock 0.4");
		resendingMapIterator = resendingMap.find(reverseResendingMapIterator->second);
		CLogPrinter_WriteSpecific(CLogPrinter::INFO, "CResendingBuffer::Queue 8r lock 0.5");
	}
	else
	{
		CLogPrinter_WriteSpecific(CLogPrinter::INFO, "CResendingBuffer::Queue 8r lock 1");
		return;
	}
    

	CLogPrinter_WriteSpecific(CLogPrinter::INFO, "CResendingBuffer::Queue 8r lock 2");
    
    if(resendingMapIterator != resendingMap.end())
	{
		CLogPrinter_WriteSpecific(CLogPrinter::INFO, "CResendingBuffer::Queue 8r lock 3");
		resendingMap.erase(resendingMapIterator);
	}
        
    

	CLogPrinter_WriteSpecific(CLogPrinter::INFO, "CResendingBuffer::Queue 8r lock 4");
    resendingMap[std::make_pair(frameNumber, packetNumber)] = m_iPushIndex;
    reverseResendingMap[m_iPushIndex] = std::make_pair(frameNumber, packetNumber);
	CLogPrinter_WriteSpecific(CLogPrinter::INFO, "CResendingBuffer::Queue 8r lock 5");

    IncreamentIndex(m_iPushIndex);
	CLogPrinter_WriteSpecific(CLogPrinter::INFO, "CResendingBuffer::Queue 8r lock 6");

    if (m_iQueueSize != m_iQueueCapacity)
        m_iQueueSize++;
}

int CResendingBuffer::DeQueue(unsigned char *decodeBuffer, int frameNumber, int packetNumber)
{
	CLogPrinter_WriteSpecific(CLogPrinter::INFO, "CResendingBuffer::DeQueue b4 lock");
	Locker lock(*m_pChannelMutex);
	CLogPrinter_WriteSpecific(CLogPrinter::INFO, "CResendingBuffer::DeQueue 8r lock");

	if (m_iQueueSize <= 0)
	{
		CLogPrinter_WriteSpecific(CLogPrinter::INFO, "CResendingBuffer::DeQueue 8r lock 1");
		return -1;
	}
	else
	{
        resendingMapIterator = resendingMap.find(std::make_pair(frameNumber, packetNumber));
		CLogPrinter_WriteSpecific(CLogPrinter::INFO, "CResendingBuffer::DeQueue 8r lock 2");
        
		if (resendingMapIterator == resendingMap.end())
		{
			CLogPrinter_WriteSpecific(CLogPrinter::INFO, "CResendingBuffer::DeQueue 8r lock 3");
			return -1;
		}
            
        
        m_iPopIndex = resendingMapIterator->second;
        
		int length = m_BufferDataLength[m_iPopIndex];

		memcpy(decodeBuffer, m_Buffer[m_iPopIndex], length);

		m_iQueueSize--;
		CLogPrinter_WriteSpecific(CLogPrinter::INFO, "CResendingBuffer::DeQueue 8r lock 4");
        resendingMap.erase(resendingMapIterator);
		CLogPrinter_WriteSpecific(CLogPrinter::INFO, "CResendingBuffer::DeQueue 8r lock 5");
        
        reverseResendingMapIterator = reverseResendingMap.find(m_iPopIndex);
        
		if (reverseResendingMapIterator != reverseResendingMap.end())
		{
			CLogPrinter_WriteSpecific(CLogPrinter::INFO, "CResendingBuffer::DeQueue 8r lock 6");
			reverseResendingMap.erase(reverseResendingMapIterator);
			CLogPrinter_WriteSpecific(CLogPrinter::INFO, "CResendingBuffer::DeQueue 8r lock 7");
		}
            
		CLogPrinter_WriteSpecific(CLogPrinter::INFO, "CResendingBuffer::DeQueue 8r lock 8");
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