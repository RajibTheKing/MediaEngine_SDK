
#include "EncodingBuffer.h"
#include "LogPrinter.h"

CEncodingBuffer::CEncodingBuffer() :

m_iPushIndex(0),
m_iPopIndex(0),
m_nQueueSize(0),
m_nQueueCapacity(MAX_VIDEO_ENCODER_BUFFER_SIZE)

{
	m_pEncodingBufferMutex.reset(new CLockHandler);
}

CEncodingBuffer::~CEncodingBuffer()
{

}

void CEncodingBuffer::ResetBuffer()
{
	Locker lock(*m_pEncodingBufferMutex);

	m_iPushIndex = 0;
	m_iPopIndex = 0;
	m_nQueueSize = 0;
}


int CEncodingBuffer::Queue(unsigned char *ucaCapturedVideoFrameData, int nLength, int nCaptureTimeDifference, int device_orientation)
{
	Locker lock(*m_pEncodingBufferMutex);
    
	memcpy(m_uc2aCapturedVideoDataBuffer[m_iPushIndex], ucaCapturedVideoFrameData, nLength);

	m_naBufferDataLengths[m_iPushIndex] = nLength;

	m_naBufferCaptureTimeDifferences[m_iPushIndex] = nCaptureTimeDifference;
	m_llaBufferInsertionTimes[m_iPushIndex] = m_Tools.CurrentTimestamp();
	m_naDevice_orientation[m_iPushIndex] = device_orientation;

	if (m_nQueueSize == m_nQueueCapacity)
    {
        IncreamentIndex(m_iPopIndex);

		CLogPrinter_WriteLog(CLogPrinter::DEBUGS, QUEUE_OVERFLOW_LOG ,"Video Buffer OverFlow ( VideoEncodingBuffer ) --> OverFlow Difftime = ");
    }
    else
    {  
		m_nQueueSize++;      
    }
    
    IncreamentIndex(m_iPushIndex);
    
    return 1;   
}

int CEncodingBuffer::DeQueue(unsigned char *ucaCapturedVideoFrameData, int &nrTimeDifferenceInQueue, int &nrCaptureTimeDifference, int &device_orientation)
{
	Locker lock(*m_pEncodingBufferMutex);
    //printf("TheKing--> EncodingBuffer m_nQueueSize = %d\n", m_nQueueSize);
	if (m_nQueueSize <= 0)
	{
		return -1;
	}
	else
	{
		int nLength;
		
		nLength = m_naBufferDataLengths[m_iPopIndex];

		memcpy(ucaCapturedVideoFrameData, m_uc2aCapturedVideoDataBuffer[m_iPopIndex], nLength);

		nrCaptureTimeDifference = m_naBufferCaptureTimeDifferences[m_iPopIndex];
		nrTimeDifferenceInQueue = m_Tools.CurrentTimestamp() - m_llaBufferInsertionTimes[m_iPopIndex];

		device_orientation = m_naDevice_orientation[m_iPopIndex];

		IncreamentIndex(m_iPopIndex);
		m_nQueueSize--;

		return nLength;
	}
}

void CEncodingBuffer::IncreamentIndex(int &irIndex)
{
	irIndex++;

	if (irIndex >= m_nQueueCapacity)
		irIndex = 0;
}

int CEncodingBuffer::GetQueueSize()
{
	Locker lock(*m_pEncodingBufferMutex);

	return m_nQueueSize;
}