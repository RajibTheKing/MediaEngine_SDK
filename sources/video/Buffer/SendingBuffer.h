
#ifndef IPV_SENDING_BUFFER_H
#define IPV_SENDING_BUFFER_H

#include "SmartPointer.h"
#include "CommonTypes.h"
#include "Tools.h"
#include "Size.h"
#include "LogPrinter.h"

namespace MediaSDK
{

	class CSendingBuffer
	{

	public:

		CSendingBuffer();
		~CSendingBuffer();

		int Queue(long long llFriendID, unsigned char *ucaSendingVideoPacketData, int nLength, int iFrameNumber, int iPacketNumber);
		int DeQueue(long long &rllFriendID, unsigned char *ucaSendingVideoPacketData, int &rnFrameNumber, int &rnPacketNumber, int &rnTimeDifferenceInQueue);
		void IncreamentIndex(int &riIndex);
		int GetQueueSize();
		void ResetBuffer();

	private:

		int m_iPushIndex;
		int m_iPopIndex;
		int m_nQueueCapacity;
		int m_nQueueSize;

		unsigned char m_uc2aSendingVideoPacketBuffer[MAX_VIDEO_PACKET_SENDING_BUFFER_SIZE][MAX_VIDEO_PACKET_SENDING_PACKET_SIZE];

		int m_naBufferDataLengths[MAX_VIDEO_PACKET_SENDING_BUFFER_SIZE];
		int m_naBufferFrameNumbers[MAX_VIDEO_PACKET_SENDING_BUFFER_SIZE];
		int m_naBufferPacketNumbers[MAX_VIDEO_PACKET_SENDING_BUFFER_SIZE];
		long long m_llaBufferFriendIDs[MAX_VIDEO_PACKET_SENDING_BUFFER_SIZE];
		long long m_llaBufferInsertionTimes[MAX_VIDEO_PACKET_SENDING_BUFFER_SIZE];

		Tools m_Tools;

		SharedPointer<CLockHandler> m_pSendingBufferMutex;
	};

} //namespace MediaSDK

#endif 
