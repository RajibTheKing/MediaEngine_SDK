
#include "RenderingBuffer.h"
#include "LogPrinter.h"
#include "Tools.h"
#include "LogPrinter.h"

namespace MediaSDK
{

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
		RenderingBufferLocker lock(*m_pRenderingBufferMutex);

		m_iPushIndex = 0;
		m_iPopIndex = 0;
		m_nQueueSize = 0;
	}

	int CRenderingBuffer::Queue(int nFrameNumber, unsigned char *ucaDecodedVideoFrameData, int nLength, long long llCaptureTimeDifference, int nVideoHeight, int nVideoWidth, int nOrientation, int nInsetHeight, int nInsetWidth)
	{
		if (m_nQueueSize >= MAX_VIDEO_RENDERER_BUFFER_SIZE)
			printf("Rendering, QUEUE SIZE = %d\n", m_nQueueSize);

		RenderingBufferLocker lock(*m_pRenderingBufferMutex);

		memcpy(m_uc2aDecodedVideoDataBuffer[m_iPushIndex], ucaDecodedVideoFrameData, nLength);

		m_naBufferDataLengths[m_iPushIndex] = nLength;
		m_naBufferFrameNumbers[m_iPushIndex] = nFrameNumber;
		m_naBufferVideoHeights[m_iPushIndex] = nVideoHeight;
		m_naBufferVideoWidths[m_iPushIndex] = nVideoWidth;
		m_naBufferVideoOrientations[m_iPushIndex] = nOrientation;
		m_naBufferInsetHeights[m_iPushIndex] = nInsetHeight;
		m_naBufferInsetWidths[m_iPushIndex] = nInsetWidth;

		m_llaBufferCaptureTimeDifferences[m_iPushIndex] = llCaptureTimeDifference;
		m_llaBufferInsertionTimes[m_iPushIndex] = Tools::CurrentTimestamp();

		if (m_nQueueSize == m_nQueueCapacity)
		{
			IncreamentIndex(m_iPopIndex);

			CLogPrinter_WriteLog(CLogPrinter::DEBUGS, QUEUE_OVERFLOW_LOG, "Video Buffer OverFlow ( RenderingBuffer ) --> OverFlow");
		}
		else
		{
			m_nQueueSize++;
		}

		IncreamentIndex(m_iPushIndex);

		return 1;
	}

	int CRenderingBuffer::DeQueue(int &rnFrameNumber, long long &rllCaptureTimeDifference, unsigned char *ucaDecodedVideoFrameData, int &rnVideoHeight, int &rnVideoWidth, int &rnTimeDifferenceInQueue, int &rnOrientation, int &rnInsetHeight, int &rnInsetWidth)
	{
		RenderingBufferLocker lock(*m_pRenderingBufferMutex);

		if (m_nQueueSize <= 0)
		{
			return -1;
		}
		else
		{
			int nLength;

			nLength = m_naBufferDataLengths[m_iPopIndex];
			rnFrameNumber = m_naBufferFrameNumbers[m_iPopIndex];
			rnVideoHeight = m_naBufferVideoHeights[m_iPopIndex];
			rnVideoWidth = m_naBufferVideoWidths[m_iPopIndex];
			rnOrientation = m_naBufferVideoOrientations[m_iPopIndex];
			rnInsetHeight = m_naBufferInsetHeights[m_iPopIndex];
			rnInsetWidth = m_naBufferInsetWidths[m_iPopIndex];

			memcpy(ucaDecodedVideoFrameData, m_uc2aDecodedVideoDataBuffer[m_iPopIndex], nLength);

			rllCaptureTimeDifference = m_llaBufferCaptureTimeDifferences[m_iPopIndex];
			rnTimeDifferenceInQueue = (int)(Tools::CurrentTimestamp() - m_llaBufferInsertionTimes[m_iPopIndex]);

			IncreamentIndex(m_iPopIndex);
			m_nQueueSize--;

			return nLength;
		}
	}

	void CRenderingBuffer::IncreamentIndex(int &riIndex)
	{
		riIndex++;

		if (riIndex >= m_nQueueCapacity)
			riIndex = 0;
	}

	int CRenderingBuffer::GetQueueSize()
	{
		RenderingBufferLocker lock(*m_pRenderingBufferMutex);

		return m_nQueueSize;
	}

} //namespace MediaSDK
