
#include "RenderingBuffer.h"

#include <string.h>

CRenderingBuffer::CRenderingBuffer() :

m_iPushIndex(0),
m_iPopIndex(0),
m_iQueueSize(0),
m_iQueueCapacity(30)

{
	m_pChannelMutex.reset(new CLockHandler);
}

CRenderingBuffer::~CRenderingBuffer()
{
/*	if (m_pChannelMutex.get())
		m_pChannelMutex.reset();*/
}

int CRenderingBuffer::Queue(int frameNumber, unsigned char *frame, int length, unsigned int timeStampDiff, int videoHeight, int videoWidth)
{
    
    Locker lock(*m_pChannelMutex);
    
    memcpy(m_Buffer[m_iPushIndex], frame, length);
    m_BufferDataLength[m_iPushIndex] = length;
    m_BufferFrameNumber[m_iPushIndex] = frameNumber;
	m_VideoHeights[m_iPushIndex] = videoHeight;
	m_VideoWidths[m_iPushIndex] = videoWidth;
	m_BufferInsertionTime[m_iPushIndex] = m_Tools.CurrentTimestamp();
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

int CRenderingBuffer::DeQueue(int &frameNumber, unsigned int &timeStampDiff, unsigned char *decodeBuffer, int &videoHeight, int &videoWidth, int &timeDiff)
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

		videoHeight = m_VideoHeights[m_iPopIndex];

		videoWidth = m_VideoWidths[m_iPopIndex];

		timeStampDiff = m_BufferTimeStamp[m_iPopIndex];

		memcpy(decodeBuffer, m_Buffer[m_iPopIndex], length);

		timeDiff = m_Tools.CurrentTimestamp() - m_BufferInsertionTime[m_iPopIndex];

		IncreamentIndex(m_iPopIndex);

		m_iQueueSize--;

		return length;
	}
}

void CRenderingBuffer::IncreamentIndex(int &index)
{
	index++;

	if (index >= m_iQueueCapacity)
		index = 0;
}

int CRenderingBuffer::GetQueueSize()
{
	Locker lock(*m_pChannelMutex);

	return m_iQueueSize;
}