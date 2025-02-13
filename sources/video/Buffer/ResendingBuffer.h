
#ifndef IPV_RESENDING_BUFFER_H
#define IPV_RESENDING_BUFFER_H

#include "SmartPointer.h"
#include "CommonTypes.h"
#include "Size.h"
#include "Tools.h"
#include <map>

namespace MediaSDK
{

	class CResendingBuffer
	{

	public:

		CResendingBuffer();
		~CResendingBuffer();

		void Queue(unsigned char *frame, int length, int frameNumber, int packetNumber);
		int DeQueue(unsigned char *decodeBuffer, int frameNumber, int packetNumber, int &timeStampDiff);
		void IncreamentIndex(int &index);
		void Reset();

	private:

		int m_iPushIndex;
		int m_iPopIndex;
		int m_iQueueCapacity;

		Tools m_Tools;

		std::map< std::pair<int, int>, int> resendingMap;
		std::map< std::pair<int, int>, int>::iterator resendingMapIterator;

		unsigned char m_Buffer[RESENDING_BUFFER_SIZE][MAX_VIDEO_PACKET_SIZE];
		int m_BufferDataLength[RESENDING_BUFFER_SIZE];
		long long m_BufferFrameNumber[RESENDING_BUFFER_SIZE];
		int m_BufferPacketNumber[RESENDING_BUFFER_SIZE];
		long long m_BufferInsertionTime[RESENDING_BUFFER_SIZE];

		SharedPointer<CLockHandler> m_pChannelMutex;
	};

} //namespace MediaSDK

#endif 
