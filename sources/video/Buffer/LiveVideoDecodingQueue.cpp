
#include "LiveVideoDecodingQueue.h"
#include "ThreadTools.h"
#include "LogPrinter.h"

namespace MediaSDK
{

	LiveVideoDecodingQueue::LiveVideoDecodingQueue() :

		m_iPushIndex(0),
		m_iPopIndex(0),
		m_nQueueSize(0),
		m_nMaxQueueSizeTillNow(0),
		m_nQueueCapacity(LIVE_VIDEO_DECODING_QUEUE_SIZE)

	{
		m_pLiveVideoDecodingQueueMutex.reset(new CLockHandler);
	}

	LiveVideoDecodingQueue::~LiveVideoDecodingQueue()
	{
		SHARED_PTR_DELETE(m_pLiveVideoDecodingQueueMutex);
	}

	void LiveVideoDecodingQueue::ResetBuffer()
	{
		LiveDecodingQueueLocker lock(*m_pLiveVideoDecodingQueueMutex);

		m_iPushIndex = 0;
		m_iPopIndex = 0;
		m_nQueueSize = 0;
	}

	int LiveVideoDecodingQueue::Queue(unsigned char *saReceivedVideoFrameData, int nLength, long long llCurrentChunkRelativeTime)
	{
		LiveDecodingQueueLocker lock(*m_pLiveVideoDecodingQueueMutex);

		if (m_nQueueSize > m_nMaxQueueSizeTillNow)
			m_nMaxQueueSizeTillNow = m_nQueueSize;

		CLogPrinter_LOG(BUFFER_SIZE_LOG, "LiveVideoDecodingQueue::Queue LIVE DECODING Buffer size %d m_nMaxQueueSizeTillNow %d m_nQueueCapacity %d", m_nQueueSize, m_nMaxQueueSizeTillNow, m_nQueueCapacity);

		if (nLength < 0 || nLength >= MAX_VIDEO_ENCODED_FRAME_SIZE)
		{
			CLogPrinter_WriteInstentTestLog(CLogPrinter::INFO, INSTENT_TEST_LOG_FF, "LiveVideoDecodingQueue::Queue   length : " + Tools::IntegertoStringConvert(nLength));

			return 0;
		}

		memcpy(m_uchBuffer[m_iPushIndex], saReceivedVideoFrameData, nLength);

		m_naBufferDataLength[m_iPushIndex] = nLength;
		m_naBufferDataTimeStamp[m_iPushIndex] = llCurrentChunkRelativeTime;

		if (m_nQueueSize == m_nQueueCapacity)
		{
			//CLogPrinter_WriteLog(CLogPrinter::INFO, INSTENT_TEST_LOG_2, "checked time");
			LOG_AAC("#aac#b4q# LiveVideoDecodingQueueOverflow, Couldn't push!!!");

			CLogPrinter_LOG(QUEUE_OVERFLOW_LOG, "LiveVideoDecodingQueue::Queue LIVE DECODING Buffer OVERFLOW m_nQueueSize %d m_nQueueCapacity %d", m_nQueueSize, m_nQueueCapacity);

			IncreamentIndex(m_iPopIndex);
		}
		else
		{
			m_nQueueSize++;
		}

		IncreamentIndex(m_iPushIndex);

		return 1;
	}

	int LiveVideoDecodingQueue::DeQueue(unsigned char *saReceivedVideoFrameData, long long &llCurrentChunkRelativeTime)
	{
		LiveDecodingQueueLocker lock(*m_pLiveVideoDecodingQueueMutex);

		if (m_nQueueSize == 0)
		{
			return -1;
		}
		else
		{
			int length = m_naBufferDataLength[m_iPopIndex];

			llCurrentChunkRelativeTime = m_naBufferDataTimeStamp[m_iPopIndex];

			memcpy(saReceivedVideoFrameData, m_uchBuffer[m_iPopIndex], length);

			IncreamentIndex(m_iPopIndex);

			m_nQueueSize--;

			return length;
		}
	}

	void LiveVideoDecodingQueue::IncreamentIndex(int &irIndex)
	{
		irIndex++;

		if (irIndex >= m_nQueueCapacity)
			irIndex = 0;
	}

	int LiveVideoDecodingQueue::GetQueueSize()
	{
		LiveDecodingQueueLocker lock(*m_pLiveVideoDecodingQueueMutex);

		return m_nQueueSize;
	}

} //namespace MediaSDK
