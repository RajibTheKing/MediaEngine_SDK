
#ifndef _VIDEO_PACKET_QUEUE_H_
#define _VIDEO_PACKET_QUEUE_H_

#include "SmartPointer.h"
#include "EventNotifier.h"
#include "ThreadTools.h"
#include "LockHandler.h"
#include <queue>
#include <utility>
#include "Tools.h"
#include "Size.h"
#include <set>

using namespace std;



class CVideoPacketQueue
{

public:

	CVideoPacketQueue();
	~CVideoPacketQueue();

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

	unsigned char m_Buffer[MAX_VIDEO_PACKET_QUEUE_SIZE][MAX_VIDEO_PACKET_SIZE];
	int m_BufferDataLength[MAX_VIDEO_PACKET_QUEUE_SIZE];

	SmartPointer<CLockHandler> m_pChannelMutex;
};

#endif //_VIDEO_PACKET_QUEUE_H_
