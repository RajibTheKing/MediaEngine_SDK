
#ifndef _RETRANSMISSION_VIDEO_PACKET_QUEUE_H_
#define _RETRANSMISSION_VIDEO_PACKET_QUEUE_H_

#include "SmartPointer.h"
#include "LockHandler.h"
#include "Size.h"

using namespace std;

class CRetransmitVideoPacketQueue
{

public:

	CRetransmitVideoPacketQueue();
	~CRetransmitVideoPacketQueue();

	int Queue(unsigned char *frame, int length);
	int DeQueue(unsigned char *decodeBuffer);
	void IncreamentIndex(int &index);
	int GetQueueSize();

private:

	int m_iPushIndex;
	int m_iPopIndex;
	int m_iQueueCapacity;
	int m_iQueueSize;

	unsigned char m_Buffer[MAX_RETRANS_VIDEO_PACKET_QUEUE_SIZE][MAX_VIDEO_PACKET_SIZE];
	int m_BufferDataLength[MAX_RETRANS_VIDEO_PACKET_QUEUE_SIZE];

	SmartPointer<CLockHandler> m_pChannelMutex;
};

#endif
