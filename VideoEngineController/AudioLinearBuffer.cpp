#include "AudioLinearBuffer.h"
#include "LogPrinter.h"
#include <cstring>


namespace MediaSDK
{
	//TODO: wait notify, release mechanism

	AudioLinearBuffer::AudioLinearBuffer(int size) :
		m_bufferMaxSize(size),
		m_beginPos(0),
		m_endPos(-1)
	{
		m_buffer = new short[m_bufferMaxSize];
	}

	AudioLinearBuffer::~AudioLinearBuffer()
	{
		std::lock_guard<std::mutex> guard(m_mutex);
		if (m_buffer)
		{
			delete[] m_buffer;
			m_buffer = nullptr;
		}
	}

	int AudioLinearBuffer::PopData(short* data)
	{
		std::lock_guard<std::mutex> guard(m_mutex);

		int availableDataSize = m_endPos - m_beginPos + 1;
		if (availableDataSize >= CHUNK_SIZE)
		{
			memcpy(data, &m_buffer[m_beginPos], CHUNK_SIZE * sizeof(short));
			m_beginPos += CHUNK_SIZE;
			LOGT("##TTPOP data size 800 availablesize %d", availableDataSize);

			return CHUNK_SIZE;
		}
		return 0;
	}

	void AudioLinearBuffer::PushData(short* data, int dataLen)
	{
		std::lock_guard<std::mutex> guard(m_mutex);
		//TODO: handle bigger data than max buffer size
		short* data_pointer = data;

		if (dataLen >= LINEAR_BUFFER_MAX_SIZE) //discarding data , pushing only LINEAR_BUFFER_MAX_SIZE size
		{
			m_beginPos = 0;
			m_endPos = -1;
			
			LOGT("##TTPUSH handleBiggerdata with size %d begin %d endpos %d", dataLen, m_beginPos, m_endPos);
			data_pointer += dataLen - LINEAR_BUFFER_MAX_SIZE;
			dataLen = LINEAR_BUFFER_MAX_SIZE;
		}
		else
		{
			int availableSpace = m_bufferMaxSize - m_endPos - 1;
			LOGT("##TTPUSH availableSpace %d begin %d endpos %d", availableSpace, m_beginPos, m_endPos);

			if (dataLen > availableSpace)
			{
				int toFreeLen = dataLen - availableSpace;

				int newIndex = 0, i = 0;
				if (toFreeLen < m_beginPos)
				{
					i = m_beginPos;
				}
				else
				{
					i = toFreeLen;
				}

				m_beginPos -= i;
				if (m_beginPos < 0)
				{
					m_beginPos = 0;
				}

				for (; i <= m_endPos; i++, newIndex++)
				{
					m_buffer[newIndex] = m_buffer[i];
				}

				m_endPos = newIndex - 1;

				LOGT("##TTPUSH after shifting. beginpos %d endpos %d freed %d", m_beginPos, m_endPos, toFreeLen);
			}
		}

		memcpy(&m_buffer[m_endPos + 1], data_pointer, dataLen * sizeof(short));
		m_endPos += dataLen;
	}

	void AudioLinearBuffer::Clear()
	{
		std::lock_guard<std::mutex> guard(m_mutex);

		m_beginPos = 0;
		m_endPos = -1;
	}
}