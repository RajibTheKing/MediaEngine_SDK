
#ifndef IPV_DEVICE_CAPABILITY_CHECK_BUFFER_H
#define IPV_DEVICE_CAPABILITY_CHECK_BUFFER_H

#include "SmartPointer.h"
#include "LockHandler.h"
#include "Tools.h"
#include "Size.h"

namespace MediaSDK
{

	class CDeviceCapabilityCheckBuffer
	{

	public:

		CDeviceCapabilityCheckBuffer();
		~CDeviceCapabilityCheckBuffer();

		int Queue(LongLong llFriendID, int nOperation, int nNotification, int nVideoHeight, int nVideoWidth);
		int DeQueue(LongLong &llrFriendID, int &nrNotification, int &nrVideoHeight, int &nrVideoWidth);
		void IncreamentIndex(int &irIndex);
		int GetQueueSize();
		void ResetBuffer();

	private:

		int m_iPushIndex;
		int m_iPopIndex;
		int m_nQueueCapacity;
		int m_nQueueSize;

		Tools m_Tools;

		LongLong m_llaBufferFriendIDs[MAX_VIDEO_PACKET_SENDING_BUFFER_SIZE];
		int m_naBufferOperations[MAX_VIDEO_PACKET_SENDING_BUFFER_SIZE];
		int m_naBufferNotifications[MAX_VIDEO_PACKET_SENDING_BUFFER_SIZE];
		int m_naBufferVideoHeights[MAX_VIDEO_PACKET_SENDING_BUFFER_SIZE];
		int m_naBufferVideoWidths[MAX_VIDEO_PACKET_SENDING_BUFFER_SIZE];

		SmartPointer<CLockHandler> m_pDeviceCapabilityCheckBufferMutex;
	};

} //namespace MediaSDK

#endif 
