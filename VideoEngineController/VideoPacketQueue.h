
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

//#ifdef	RETRANSMISSION_ENABLED
//	bool PacketExists(int iFrameNUmber, int iPacketNumber);
//#endif

private:

	int m_iPushIndex;
	int m_iPopIndex;
	int m_iDecodingIndex;
	int m_iQueueCapacity;
	int m_iQueueSize;

	unsigned char m_Buffer[MAX_VIDEO_PACKET_QUEUE_SIZE][MAX_VIDEO_PACKET_SIZE];
	int m_BufferDataLength[MAX_VIDEO_PACKET_QUEUE_SIZE];
	int m_BufferIndexState[MAX_VIDEO_PACKET_QUEUE_SIZE];
//#ifdef	RETRANSMISSION_ENABLED
//	std::set<int>m_FrameInQueue;
//#endif
	SmartPointer<CLockHandler> m_pChannelMutex;


	/*pair<int, int> ExpectedFramePacketPair;
	int iNumberOfPacketsInCurrentFrame;
	pair<int, int> GetFramePacketFromHeader(unsigned char * packet, int &iNumberOfPackets);
	void UpdateExpectedFramePacketPair(pair<int, int> currentFramePacketPair, int iNumberOfPackets);

	int GetIntFromChar(unsigned char *packetData, int index);

	std::queue<pair<int,int>> ExpectedFramePacketDeQueue;*/

};

#endif //_VIDEO_PACKET_QUEUE_H_
