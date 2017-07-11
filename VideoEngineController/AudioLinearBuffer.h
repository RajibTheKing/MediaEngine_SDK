#ifndef AUDIO_LINEAR_BUFFER_H
#define AUDIO_LINEAR_BUFFER_H

#include <mutex>

namespace MediaSDK
{
#define CHUNK_SIZE 800
	class AudioLinearBuffer
	{
		short* m_buffer = nullptr;
		int m_bufferMaxSize = -1;
		int m_beginPos = -1, m_endPos = -1;

		std::mutex m_mutex;
	public:

		void PushData(short* data, int dataLen);

		void Clear();
		AudioLinearBuffer(int size);
		~AudioLinearBuffer();
		int PopData(short* data);
	};
}
#endif//AUDIO_LINEAR_BUFFER_H
