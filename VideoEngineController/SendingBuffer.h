
#ifndef _SENDING_BUFFER_H_
#define _SENDING_BUFFER_H_

#include "SmartPointer.h"
#include "LockHandler.h"
#include "Size.h"
#include "Tools.h"


class CSendingBuffer
{

public:

	CSendingBuffer();
	~CSendingBuffer();

	int Queue(LongLong lFriendID, unsigned char *frame, int length, int frameNumber, int packetNumber);
	int DeQueue(LongLong &lFriendID, unsigned char *decodeBuffer, int &frameNumber, int &packetNumber, int &timeDiff);
	void IncreamentIndex(int &index);
	int GetQueueSize();

private:

	int m_iPushIndex;
	int m_iPopIndex;
	int m_iDecodingIndex;
	int m_iQueueCapacity;
	int m_iQueueSize;

	Tools m_Tools;

	unsigned char m_Buffer[MAX_VIDEO_PACKET_SENDING_BUFFER_SIZE][MAX_VIDEO_PACKET_SENDING_PACKET_SIZE];
	int m_BufferDataLength[MAX_VIDEO_PACKET_SENDING_BUFFER_SIZE];
	LongLong m_BufferFriendIDs[MAX_VIDEO_PACKET_SENDING_BUFFER_SIZE];
	int m_BufferFrameNumber[MAX_VIDEO_PACKET_SENDING_BUFFER_SIZE];
	int m_BufferPacketNumber[MAX_VIDEO_PACKET_SENDING_BUFFER_SIZE];
	int m_BufferIndexState[MAX_VIDEO_PACKET_SENDING_BUFFER_SIZE];
	long long m_BufferInsertionTime[MAX_VIDEO_PACKET_SENDING_BUFFER_SIZE];

	SmartPointer<CLockHandler> m_pChannelMutex;
};

#endif 
