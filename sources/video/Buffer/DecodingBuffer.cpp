
#include "DecodingBuffer.h"
#include "LogPrinter.h"

namespace MediaSDK
{

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

	int CDecodingBuffer::Queue(int iFrameNumber, unsigned char *ucaEncodedVideoFrameData, int nLength, unsigned int unCaptureTimeDifference)
	{
		CDecodingBufferLocker lock(*m_pDecodingBufferMutex);

		if (m_nQueueSize > m_nMaxQueueSizeTillNow)
			m_nMaxQueueSizeTillNow = m_nQueueSize;

		CLogPrinter_LOG(BUFFER_SIZE_LOG, "CDecodingBuffer::Queue DECODING Buffer size %d m_nMaxQueueSizeTillNow %d m_nQueueCapacity %d", m_nQueueSize, m_nMaxQueueSizeTillNow, m_nQueueCapacity);

		memcpy(m_uc2aEncodedVideoDataBuffer[m_iPushIndex], ucaEncodedVideoFrameData, nLength);

		m_naBufferDataLengths[m_iPushIndex] = nLength;
		m_naBufferFrameNumbers[m_iPushIndex] = iFrameNumber;

		m_unaBufferCaptureTimeDifferences[m_iPushIndex] = unCaptureTimeDifference;
		m_llaBufferInsertionTimes[m_iPushIndex] = m_Tools.CurrentTimestamp();

		if (m_nQueueSize == m_nQueueCapacity)
		{
			IncreamentIndex(m_iPopIndex);

			CLogPrinter_LOG(QUEUE_OVERFLOW_LOG, "CDecodingBuffer::Queue DECODING Buffer OVERFLOW m_nQueueSize %d m_nQueueCapacity %d", m_nQueueSize, m_nQueueCapacity);
		}
		else
		{
			m_nQueueSize++;
		}

		IncreamentIndex(m_iPushIndex);

		return 1;
	}

	int CDecodingBuffer::DeQueue(int &irFrameNumber, unsigned int &unrCaptureTimeDifference, unsigned char *ucaEncodedVideoFrameData, int &nrTimeDifferenceInQueue)
	{
		CDecodingBufferLocker lock(*m_pDecodingBufferMutex);
		//printf("TheKing--> DecodingBuffer m_nQueueSize = %d\n", m_nQueueSize);
		if (m_nQueueSize <= 0)
		{
			return -1;
		}
		else
		{
			int nLength;

			nLength = m_naBufferDataLengths[m_iPopIndex];
			irFrameNumber = m_naBufferFrameNumbers[m_iPopIndex];

			memcpy(ucaEncodedVideoFrameData, m_uc2aEncodedVideoDataBuffer[m_iPopIndex], nLength);

			unrCaptureTimeDifference = m_unaBufferCaptureTimeDifferences[m_iPopIndex];
			nrTimeDifferenceInQueue = m_Tools.CurrentTimestamp() - m_llaBufferInsertionTimes[m_iPopIndex];

			IncreamentIndex(m_iPopIndex);
			m_nQueueSize--;

			return nLength;
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
		CDecodingBufferLocker lock(*m_pDecodingBufferMutex);

		return m_nQueueSize;
	}

} //namespace MediaSDK
