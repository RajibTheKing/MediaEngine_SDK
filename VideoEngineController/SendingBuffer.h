
#ifndef _SENDING_BUFFER_H_
#define _SENDING_BUFFER_H_

#include "SmartPointer.h"
#include "LockHandler.h"
#include "Tools.h"
#include "Size.h"

class CSendingBuffer
{

public:

	CSendingBuffer();
	~CSendingBuffer();

	int Queue(LongLong llFriendID, unsigned char *ucaSendingVideoPacketData, int nLength, int iFrameNumber, int iPacketNumber);
	int DeQueue(LongLong &llrFriendID, unsigned char *ucaSendingVideoPacketData, int &nrFrameNumber, int &nrPacketNumber, int &nrTimeDifferenceInQueue);
	void IncreamentIndex(int &irIndex);
	int GetQueueSize();

private:

	int m_iPushIndex;
	int m_iPopIndex;
	int m_nQueueCapacity;
	int m_nQueueSize;

	Tools m_Tools;

	unsigned char m_uc2aSendingVideoPacketBuffer[MAX_VIDEO_PACKET_SENDING_BUFFER_SIZE][MAX_VIDEO_PACKET_SENDING_PACKET_SIZE];

	int m_naBufferDataLengths[MAX_VIDEO_PACKET_SENDING_BUFFER_SIZE];
	LongLong m_llaBufferFriendIDs[MAX_VIDEO_PACKET_SENDING_BUFFER_SIZE];
	int m_naBufferFrameNumbers[MAX_VIDEO_PACKET_SENDING_BUFFER_SIZE];
	int m_naBufferPacketNumbers[MAX_VIDEO_PACKET_SENDING_BUFFER_SIZE];

	long long m_llBufferInsertionTimes[MAX_VIDEO_PACKET_SENDING_BUFFER_SIZE];

	SmartPointer<CLockHandler> m_pSendingBufferMutex;
};

#endif 
