
#include "RenderingBuffer.h"
#include "LogPrinter.h"
CRenderingBuffer::CRenderingBuffer() :

m_iPushIndex(0),
m_iPopIndex(0),
m_nQueueSize(0),
m_nQueueCapacity(MAX_VIDEO_RENDERER_BUFFER_SIZE)

{
	m_pRenderingBufferMutex.reset(new CLockHandler);
}

CRenderingBuffer::~CRenderingBuffer()
{

}

void CRenderingBuffer::ResetBuffer()
{
	Locker lock(*m_pRenderingBufferMutex);

	m_iPushIndex = 0;
	m_iPopIndex = 0;
	m_nQueueSize = 0;
}

int CRenderingBuffer::Queue(int iFrameNumber, unsigned char *ucaDecodedVideoFrameData, int nLength, long long llCaptureTimeDifference, int nVideoHeight, int nVideoWidth,int nOrientation)
{
    //printf("Rendering, QUEUE SIZE = %d\n", m_nQueueSize);
	Locker lock(*m_pRenderingBufferMutex);
    
	memcpy(m_uc2aDecodedVideoDataBuffer[m_iPushIndex], ucaDecodedVideoFrameData, nLength);

	m_naBufferDataLengths[m_iPushIndex] = nLength;
	m_naBufferFrameNumbers[m_iPushIndex] = iFrameNumber;
	m_naBufferVideoHeights[m_iPushIndex] = nVideoHeight;
	m_naBufferVideoWidths[m_iPushIndex] = nVideoWidth;
	m_naBufferVideoOrientations[m_iPushIndex] = nOrientation;

	m_llaBufferCaptureTimeDifferences[m_iPushIndex] = llCaptureTimeDifference;
	m_llaBufferInsertionTimes[m_iPushIndex] = m_Tools.CurrentTimestamp();
    
	if (m_nQueueSize == m_nQueueCapacity)
    {
        IncreamentIndex(m_iPopIndex);

		CLogPrinter_WriteLog(CLogPrinter::DEBUGS, QUEUE_OVERFLOW_LOG ,"Video Buffer OverFlow ( RenderingBuffer ) --> OverFlow" );
    }
    else
    { 
		m_nQueueSize++;      
    }
    
    IncreamentIndex(m_iPushIndex);
    
    return 1;
}

int CRenderingBuffer::DeQueue(int &irFrameNumber, long long &llrCaptureTimeDifference, unsigned char *ucaDecodedVideoFrameData, int &nrVideoHeight, int &nrVideoWidth,
							  int &nrTimeDifferenceInQueue, int &nOrientation)
{
	Locker lock(*m_pRenderingBufferMutex);
    //printf("TheKing--> RenderingBuffer m_nQueueSize = %d\n", m_nQueueSize);
	if (m_nQueueSize <= 0)
	{
		return -1;
	}
	else
	{
		int nLength;
		
		nLength = m_naBufferDataLengths[m_iPopIndex];
		irFrameNumber = m_naBufferFrameNumbers[m_iPopIndex];
		nrVideoHeight = m_naBufferVideoHeights[m_iPopIndex];
		nrVideoWidth = m_naBufferVideoWidths[m_iPopIndex];
		nOrientation = m_naBufferVideoOrientations[m_iPopIndex];

		memcpy(ucaDecodedVideoFrameData, m_uc2aDecodedVideoDataBuffer[m_iPopIndex], nLength);

		llrCaptureTimeDifference = m_llaBufferCaptureTimeDifferences[m_iPopIndex];
		nrTimeDifferenceInQueue = m_Tools.CurrentTimestamp() - m_llaBufferInsertionTimes[m_iPopIndex];

		IncreamentIndex(m_iPopIndex);
		m_nQueueSize--;

		return nLength;
	}
}

void CRenderingBuffer::IncreamentIndex(int &irIndex)
{
	irIndex++;

	if (irIndex >= m_nQueueCapacity)
		irIndex = 0;
}

int CRenderingBuffer::GetQueueSize()
{
	Locker lock(*m_pRenderingBufferMutex);

	return m_nQueueSize;
}