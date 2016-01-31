
#ifndef _DECODING_BUFFER_H_
#define _DECODING_BUFFER_H_

#include "SmartPointer.h"
#include "LockHandler.h"
#include "Tools.h"

#define MAX_VIDEO_DECODER_BUFFER_SIZE 30
#define MAX_VIDEO_DECODER_FRAME_SIZE 352 * 288 * 3

class CDecodingBuffer
{

public:

	CDecodingBuffer();
	~CDecodingBuffer();

	int Queue(int frameNumber, unsigned char *frame, int length, unsigned int timeStampDiff);
	int DeQueue(int &frameNumber,unsigned int &timeStampDiff, unsigned char *decodeBuffer, int &timeDiffForQueue);
	void IncreamentIndex(int &index);
	int GetQueueSize();

private:

	int m_iPushIndex;
	int m_iPopIndex;
	int m_iDecodingIndex;
	int m_iQueueCapacity;
	int m_iQueueSize;

	Tools m_Tools;

	unsigned char m_Buffer[MAX_VIDEO_DECODER_BUFFER_SIZE][MAX_VIDEO_DECODER_FRAME_SIZE];
	int m_BufferDataLength[MAX_VIDEO_DECODER_BUFFER_SIZE];
	int m_BufferFrameNumber[MAX_VIDEO_DECODER_BUFFER_SIZE];
	int m_BufferIndexState[MAX_VIDEO_DECODER_BUFFER_SIZE];

	unsigned int m_BufferTimeStamp[MAX_VIDEO_DECODER_BUFFER_SIZE];
	long long m_BufferInsertionTime[MAX_VIDEO_DECODER_BUFFER_SIZE];

	SmartPointer<CLockHandler> m_pChannelMutex;
};

#endif 
