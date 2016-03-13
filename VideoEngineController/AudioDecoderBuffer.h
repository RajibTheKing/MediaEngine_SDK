
#ifndef _AUDIO_DECODER_BUFFER_H_
#define _AUDIO_DECODER_BUFFER_H_

#include "SmartPointer.h"
#include "LockHandler.h"
#include "Tools.h"

#define MAX_AUDIO_DECODER_BUFFER_SIZE 30
#define MAX_AUDIO_DECODER_FRAME_SIZE 4096

class CAudioDecoderBuffer
{

public:

	CAudioDecoderBuffer();
	~CAudioDecoderBuffer();

	int Queue(unsigned char *frame, int length);
	int DeQueue(unsigned char *decodeBuffer);
	void IncreamentIndex(int &index);
	int GetQueueSize();

private:


	int m_iPushIndex;
	int m_iPopIndex;
	int m_iDecodingIndex;
	int m_iQueueCapacity;
	int m_iQueueSize;

	Tools m_Tools;
    long long m_lPrevOverFlowTime;
    long long m_lSumOverFlowTime;
    double m_dAvgOverFlowTime;
    int m_iOverFlowCount;

	unsigned char m_Buffer[MAX_AUDIO_DECODER_BUFFER_SIZE][MAX_AUDIO_DECODER_FRAME_SIZE];
	int m_BufferDataLength[MAX_AUDIO_DECODER_BUFFER_SIZE];
	int m_BufferIndexState[MAX_AUDIO_DECODER_BUFFER_SIZE];
	long long m_BufferInsertionTime[MAX_AUDIO_DECODER_BUFFER_SIZE];

	SmartPointer<CLockHandler> m_pChannelMutex;
};

#endif 
