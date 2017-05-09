
#ifndef _AUDIO_DECODER_BUFFER_H_
#define _AUDIO_DECODER_BUFFER_H_

#include "SmartPointer.h"
#include "LockHandler.h"
#include "Tools.h"
#include "AudioMacros.h"

namespace MediaSDK
{

	class CAudioByteBuffer
	{

	public:

		CAudioByteBuffer();
		~CAudioByteBuffer();

		int EnQueue(unsigned char *saReceivedAudioFrameData, int nLength);
		int DeQueue(unsigned char *saReceivedAudioFrameData);
		void IncreamentIndex(int &irIndex);
		int GetQueueSize();
		void ResetBuffer();

	private:


		int m_iPushIndex;
		int m_iPopIndex;
		int m_nQueueCapacity;
		int m_nQueueSize;

		Tools m_Tools;
		long long mt_lPrevOverFlowTime;
		long long mt_lSumOverFlowTime;
		double mt_dAvgOverFlowTime;
		int mt_nOverFlowCount;

		unsigned char m_s2aAudioDecodingBuffer[MAX_AUDIO_DECODER_BUFFER_SIZE][MAX_AUDIO_DECODER_FRAME_SIZE];
		int m_naBufferDataLength[MAX_AUDIO_DECODER_BUFFER_SIZE];
		long long m_BufferInsertionTime[MAX_AUDIO_DECODER_BUFFER_SIZE];

		SmartPointer<CLockHandler> m_pAudioDecodingBufferMutex;

	};

} //namespace MediaSDK

#endif 
