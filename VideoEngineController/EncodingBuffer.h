
#ifndef _ENCODING_BUFFER_H_
#define _ENCODING_BUFFER_H_

#include "SmartPointer.h"
#include "LockHandler.h"

#define MAX_VIDEO_ENCODER_BUFFER_SIZE 45
#define MAX_VIDEO_ENCODER_FRAME_SIZE (352 * 288 * 3)/2+1

class CEncodingBuffer
{

public:

	CEncodingBuffer();
	~CEncodingBuffer();

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

	unsigned char m_Buffer[MAX_VIDEO_ENCODER_BUFFER_SIZE][MAX_VIDEO_ENCODER_FRAME_SIZE];
	int m_BufferDataLength[MAX_VIDEO_ENCODER_BUFFER_SIZE];
	int m_BufferIndexState[MAX_VIDEO_ENCODER_BUFFER_SIZE];

	SmartPointer<CLockHandler> m_pChannelMutex;
};

#endif 
