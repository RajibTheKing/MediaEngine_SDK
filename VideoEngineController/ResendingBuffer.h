
#ifndef _RESENDING_BUFFER_H_
#define _RESENDING_BUFFER_H_

#include "SmartPointer.h"
#include "LockHandler.h"

#include <map>

#define MAX_VIDEO_PACKET_RESENDING_BUFFER_SIZE 1000
#define MAX_VIDEO_PACKET_RESENDING_PACKET_SIZE 508

class CResendingBuffer
{
public:
	CResendingBuffer();
	~CResendingBuffer();

	void Queue(unsigned char *frame, int length, int frameNumber, int packetNumber);
	int DeQueue(unsigned char *decodeBuffer, int frameNumber, int packetNumber);
	void IncreamentIndex(int &index);

	//	int GetQueueSize();

private:

	int m_iPushIndex;
	int m_iPopIndex;
	int m_iQueueCapacity;

	std::map< std::pair<int, int>, int> resendingMap;
	std::map< std::pair<int, int>, int>::iterator resendingMapIterator;


	unsigned char m_Buffer[MAX_VIDEO_PACKET_RESENDING_BUFFER_SIZE][MAX_VIDEO_PACKET_RESENDING_PACKET_SIZE];
	int m_BufferDataLength[MAX_VIDEO_PACKET_RESENDING_BUFFER_SIZE];
	LongLong m_BufferFrameNumber[MAX_VIDEO_PACKET_RESENDING_BUFFER_SIZE];
	int m_BufferPacketNumber[MAX_VIDEO_PACKET_RESENDING_BUFFER_SIZE];

	SmartPointer<CLockHandler> m_pChannelMutex;
};

#endif 
