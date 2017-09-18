#ifndef LIVESTREAMING_LIVEAUDIODECODINGQUEUE_H
#define LIVESTREAMING_LIVEAUDIODECODINGQUEUE_H

#include "SmartPointer.h"
#include "CommonTypes.h"
#include <vector>

#define LIVE_AUDIO_DECODING_QUEUE_SIZE 150
#define MAX_AUDIO_ENCODED_FRAME_LEN 2048


namespace MediaSDK
{
	using PI = std::pair < int, int > ;
	using VP = std::vector < PI > ;

	class LiveAudioDecodingQueue
	{
	public:

		LiveAudioDecodingQueue();
		~LiveAudioDecodingQueue();

		int EnQueue(unsigned char *saReceivedAudioFrameData, int nLength, VP vMissing);
		int DeQueue(unsigned char *saReceivedAudioFrameData, VP &vMissing);
		void IncreamentIndex(int &irIndex);
		int GetQueueSize();
		void ResetBuffer();


	private:

		int m_iPushIndex;
		int m_iPopIndex;
		int m_nQueueCapacity;
		int m_nQueueSize;

		unsigned char m_uchBuffer[LIVE_AUDIO_DECODING_QUEUE_SIZE][MAX_AUDIO_ENCODED_FRAME_LEN];
		int m_naBufferDataLength[LIVE_AUDIO_DECODING_QUEUE_SIZE];
		VP m_vMissingBuffer[LIVE_AUDIO_DECODING_QUEUE_SIZE];

		SharedPointer<CLockHandler> m_pLiveAudioDecodingQueueMutex;
	};

}  // namespace MediaSDK

#endif // !LIVESTREAMING_LIVEAUDIODECODINGQUEUE_H
