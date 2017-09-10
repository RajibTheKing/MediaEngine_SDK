
#ifndef IPV_RETRANSMISSION_VIDEO_PACKET_QUEUE_H
#define IPV_RETRANSMISSION_VIDEO_PACKET_QUEUE_H

#include "SmartPointer.h"
#include "CommonTypes.h"
#include "Size.h"

namespace MediaSDK
{

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

		SharedPointer<CLockHandler> m_pChannelMutex;
	};

} //namespace MediaSDK

#endif
