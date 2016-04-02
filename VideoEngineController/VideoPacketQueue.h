
#ifndef _VIDEO_PACKET_QUEUE_H_
#define _VIDEO_PACKET_QUEUE_H_

#include "SmartPointer.h"
#include "LockHandler.h"
#include "Size.h"

using namespace std;

class CVideoPacketQueue
{

public:

	CVideoPacketQueue();
	~CVideoPacketQueue();

	int Queue(unsigned char *ucaVideoPacketData, int nLength);
	int DeQueue(unsigned char *ucaVideoPacketData);
	void IncreamentIndex(int &irIndex);
	int GetQueueSize();

private:

	int m_iPushIndex;
	int m_iPopIndex;
	int m_nQueueCapacity;
	int m_nQueueSize;

	unsigned char m_uc2aVideoPacketBuffer[MAX_VIDEO_PACKET_QUEUE_SIZE][MAX_VIDEO_PACKET_SIZE];
	int m_naBufferDataLengths[MAX_VIDEO_PACKET_QUEUE_SIZE];

	SmartPointer<CLockHandler> m_pVideoPacketQueueMutex;
};

#endif
