#include "AudioLinearBuffer.h"
#include "LogPrinter.h"
#include <cstring>


namespace MediaSDK
{
	//TODO: lock this , wait notify, release mechanism
	//TODO: add for live channel ...
	AudioLinearBuffer::AudioLinearBuffer(int size) :
		m_bufferMaxSize(size),
		m_beginPos(0),
		m_endPos(-1),
		m_availableDataSize(0)
	{
		m_buffer = new short[m_bufferMaxSize];
	}

	AudioLinearBuffer::~AudioLinearBuffer()
	{
		if (m_buffer)
		{
			delete[] m_buffer;
			m_buffer = nullptr;
		}
	}

	int AudioLinearBuffer::PopData(short* data)
	{
		std::lock_guard<std::mutex> guard(m_mutex);

		m_availableDataSize = m_endPos - m_beginPos + 1;
		if (m_availableDataSize >= CHUNK_SIZE)
		{
			memcpy(data, &m_buffer[m_beginPos], CHUNK_SIZE * sizeof(short));
			m_beginPos += CHUNK_SIZE;
			LOGT("##TTPOP data size 800 availablesize %d", m_availableDataSize);
			return CHUNK_SIZE;
		}
		return 0;
	}

	void AudioLinearBuffer::PushData(short* data, int dataLen)
	{
		std::lock_guard<std::mutex> guard(m_mutex);
		//TODO: handle bigger data than max buffer size

		int availableSpace = m_bufferMaxSize - m_endPos - 1;
		LOGT("##TTPUSH availableSpace %d begin %d endpos %d", availableSpace, m_beginPos, m_endPos);


		if (dataLen > availableSpace)
		{
			int toFreeLen = dataLen - availableSpace;

			int newIndex = 0;
			for (int i = toFreeLen; i <= m_endPos; i++, newIndex++)
			{
				m_buffer[newIndex] = m_buffer[i];
			}

			m_endPos = newIndex - 1;
			m_beginPos -= toFreeLen;
			if (m_beginPos < 0)
			{
				m_beginPos = 0;
			}
			LOGT("##TTPUSH after shifting. beginpos %d endpos %d freed %d", m_beginPos, m_endPos, toFreeLen);
		}

		memcpy(&m_buffer[m_endPos + 1], data, dataLen * sizeof(short));
		m_endPos += dataLen;
	}
}