
#ifndef _AUDIO_BUFFER_H_
#define _AUDIO_BUFFER_H_

#include "SmartPointer.h"
#include "LockHandler.h"
#include "Tools.h"

#define MAX_AUDIO_ENCODER_BUFFER_SIZE 30
#define MAX_AUDIO_ENCODER_FRAME_SIZE 4096

class CAudioEncoderBuffer
{

public:

	CAudioEncoderBuffer();
	~CAudioEncoderBuffer();

	int Queue(short *frame, int length);
	int DeQueue(short *decodeBuffer);
	void IncreamentIndex(int &index);
	int GetQueueSize();

private:

	Tools m_Tools;

	int m_iPushIndex;
	int m_iPopIndex;
	int m_iQueueCapacity;
	int m_iQueueSize;

    long long m_lPrevOverFlowTime;
    long long m_lSumOverFlowTime;
    double m_dAvgOverFlowTime;
    int m_iOverFlowCount;

	short m_Buffer[MAX_AUDIO_ENCODER_BUFFER_SIZE][MAX_AUDIO_ENCODER_FRAME_SIZE];
	int m_BufferDataLength[MAX_AUDIO_ENCODER_BUFFER_SIZE];

	SmartPointer<CLockHandler> m_pChannelMutex;
};

#endif 
