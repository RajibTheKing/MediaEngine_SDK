
#ifndef _SENDING_BUFFER_H_
#define _SENDING_BUFFER_H_

#include "SmartPointer.h"
#include "LockHandler.h"
#include "Size.h"


class CSendingBuffer
{

public:

	CSendingBuffer();
	~CSendingBuffer();

	int Queue(LongLong lFriendID, unsigned char *frame, int length);
	int DeQueue(LongLong &lFriendID, unsigned char *decodeBuffer);
	void IncreamentIndex(int &index);
	int GetQueueSize();

private:

	int m_iPushIndex;
	int m_iPopIndex;
	int m_iDecodingIndex;
	int m_iQueueCapacity;
	int m_iQueueSize;

	unsigned char m_Buffer[MAX_VIDEO_PACKET_SENDING_BUFFER_SIZE][MAX_VIDEO_PACKET_SENDING_PACKET_SIZE];
	int m_BufferDataLength[MAX_VIDEO_PACKET_SENDING_BUFFER_SIZE];
	LongLong m_BufferFrameNumber[MAX_VIDEO_PACKET_SENDING_BUFFER_SIZE];
	int m_BufferIndexState[MAX_VIDEO_PACKET_SENDING_BUFFER_SIZE];

	SmartPointer<CLockHandler> m_pChannelMutex;
};

#endif 
