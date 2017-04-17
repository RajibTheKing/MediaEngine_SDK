#ifndef _AUDIO_BUFFER_H_
#define _AUDIO_BUFFER_H_

#include "SmartPointer.h"
#include "LockHandler.h"
#include "Tools.h"
#include "AudioMacros.h"


class CAudioShortBuffer
{

public:

	CAudioShortBuffer();
	CAudioShortBuffer(int iQueueSize);
	~CAudioShortBuffer();

	int EnQueue(short *saCapturedAudioFrameData, int nlength, long long llTimeStump);
	int DeQueue(short *saCapturedAudioFrameData, long long &receivedTime);
	int DeQueueForCallee(short *saCapturedAudioFrameData, long long &receivedTime, int iCalleeFrameNoSentByPublisher);
	void IncreamentIndex(int &irIndex);
	int GetQueueSize();
	void ResetBuffer();

private:

	Tools m_Tools;

	int m_iPushIndex;
	int m_iPopIndex;
	int m_nQueueCapacity;
	int m_nQueueSize;

    long long mt_llPrevOverFlowTime;
    long long mt_llSumOverFlowTime;
    int mt_nOverFlowCounter;
	double m_dAvgOverFlowTime;

	short m_s2aAudioEncodingBuffer[MAX_AUDIO_ENCODING_BUFFER_SIZE][MAX_AUDIO_ENCODING_FRAME_SIZE];
	int m_naBufferDataLength[MAX_AUDIO_ENCODING_BUFFER_SIZE];
	long long m_laReceivedTimeList[MAX_AUDIO_ENCODING_BUFFER_SIZE];

	SmartPointer<CLockHandler> m_pAudioEnocdingBufferMutex;
};

#endif 
