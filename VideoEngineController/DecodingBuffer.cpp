
#include "DecodingBuffer.h"

#include <string.h>

CDecodingBuffer::CDecodingBuffer() :

m_iPushIndex(0),
m_iPopIndex(0),
m_nQueueSize(0),
m_nQueueCapacity(MAX_VIDEO_DECODER_BUFFER_SIZE)

{
	m_pDecodingBufferMutex.reset(new CLockHandler);
}

CDecodingBuffer::~CDecodingBuffer()
{

}

int CDecodingBuffer::Queue(int iFrameNumber, unsigned char *ucaEncodedVideoFrameData, int nLength, unsigned int unTimeStampDifference)
{
    Locker lock(*m_pDecodingBufferMutex);
    
	memcpy(m_uc2aEncodedVideoDataBuffer[m_iPushIndex], ucaEncodedVideoFrameData, nLength);

	m_naBufferDataLengths[m_iPushIndex] = nLength;
	m_naBufferFrameNumbers[m_iPushIndex] = iFrameNumber;

	m_unaBufferTimeStampDifferences[m_iPushIndex] = unTimeStampDifference;
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

int CDecodingBuffer::DeQueue(int &irFrameNumber, unsigned int &unrTimeStampDifference, unsigned char *ucaEncodedVideoFrameData, int &unrTimeDifferenceInQueue)
{
	Locker lock(*m_pDecodingBufferMutex);

	if (m_nQueueSize <= 0)
	{
		return -1;
	}
	else
	{
		int nlength;

		nlength = m_naBufferDataLengths[m_iPopIndex];
		irFrameNumber = m_naBufferFrameNumbers[m_iPopIndex];

		memcpy(ucaEncodedVideoFrameData, m_uc2aEncodedVideoDataBuffer[m_iPopIndex], nlength);

		unrTimeStampDifference = m_unaBufferTimeStampDifferences[m_iPopIndex];
		unrTimeDifferenceInQueue = m_Tools.CurrentTimestamp() - m_llBufferInsertionTimes[m_iPopIndex];

		IncreamentIndex(m_iPopIndex);
		m_nQueueSize--;

		return nlength;
	}
}

void CDecodingBuffer::IncreamentIndex(int &irIndex)
{
	irIndex++;

	if (irIndex >= m_nQueueCapacity)
		irIndex = 0;
}

int CDecodingBuffer::GetQueueSize()
{
	Locker lock(*m_pDecodingBufferMutex);

	return m_nQueueSize;
}