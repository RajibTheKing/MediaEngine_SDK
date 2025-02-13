
#include "VideoFrameBuffer.h"
#include "LogPrinter.h"
#include "Tools.h"
#include "LogPrinter.h"

namespace MediaSDK
{

	VideoFrameBuffer::VideoFrameBuffer() :

		m_iPushIndex(0),
		m_iPopIndex(0),
		m_nQueueSize(0),
		m_nMaxQueueSizeTillNow(0),
		m_nQueueCapacity(MAX_VIDEO_FRAME_BUFFER_SIZE)

	{
		m_pVideoFrameBufferMutex.reset(new CLockHandler);
	}

	VideoFrameBuffer::~VideoFrameBuffer()
	{

	}

	void VideoFrameBuffer::ResetBuffer()
	{
		VideoFrameBufferLocker lock(*m_pVideoFrameBufferMutex);

		m_iPushIndex = 0;
		m_iPopIndex = 0;
		m_nQueueSize = 0;
	}

	int VideoFrameBuffer::Queue(unsigned char *ucaDecodedVideoFrameData, int nLength)
	{
		if (m_nQueueSize >= MAX_VIDEO_FRAME_BUFFER_SIZE)
			printf("Rendering, VideoFrameBuffer = %d\n", m_nQueueSize);

		VideoFrameBufferLocker lock(*m_pVideoFrameBufferMutex);

		if (m_nQueueSize > m_nMaxQueueSizeTillNow)
			m_nMaxQueueSizeTillNow = m_nQueueSize;

		CLogPrinter_LOG(BUFFER_SIZE_LOG, "VideoFrameBuffer::Queue VIDEO FRAME Buffer size %d m_nMaxQueueSizeTillNow %d m_nQueueCapacity %d", m_nQueueSize, m_nMaxQueueSizeTillNow, m_nQueueCapacity);

		memcpy(m_uc2aDecodedVideoDataBuffer[m_iPushIndex], ucaDecodedVideoFrameData, nLength);

		m_naBufferDataLengths[m_iPushIndex] = nLength;


		if (m_nQueueSize == m_nQueueCapacity)
		{
			IncreamentIndex(m_iPopIndex);

			CLogPrinter_LOG(QUEUE_OVERFLOW_LOG, "VideoFrameBuffer::Queue VIDEO FRAME Buffer OVERFLOW m_nQueueSize %d m_nQueueCapacity %d", m_nQueueSize, m_nQueueCapacity);
		}
		else
		{
			m_nQueueSize++;
		}

		IncreamentIndex(m_iPushIndex);

		return 1;
	}

	int VideoFrameBuffer::DeQueue(unsigned char *ucaDecodedVideoFrameData)
	{
		VideoFrameBufferLocker lock(*m_pVideoFrameBufferMutex);

		if (m_nQueueSize <= 0)
		{
			return -1;
		}
		else
		{
			int nLength;

			nLength = m_naBufferDataLengths[m_iPopIndex];

			memcpy(ucaDecodedVideoFrameData, m_uc2aDecodedVideoDataBuffer[m_iPopIndex], nLength);


			IncreamentIndex(m_iPopIndex);
			m_nQueueSize--;

			return nLength;
		}
	}

	void VideoFrameBuffer::IncreamentIndex(int &riIndex)
	{
		riIndex++;

		if (riIndex >= m_nQueueCapacity)
			riIndex = 0;
	}

	int VideoFrameBuffer::GetQueueSize()
	{
		VideoFrameBufferLocker lock(*m_pVideoFrameBufferMutex);

		return m_nQueueSize;
	}

} //namespace MediaSDK
