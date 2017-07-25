
#ifndef IPV_LIVE_RECEIVER_H
#define IPV_LIVE_RECEIVER_H

#include "SmartPointer.h"
#include "CommonTypes.h"
#include <vector>

namespace MediaSDK
{

	class CCommonElementsBucket;
	class LiveVideoDecodingQueue;

	class LiveReceiver
	{

	public:

		LiveReceiver(CCommonElementsBucket* sharedObject);
		~LiveReceiver();

		void SetVideoDecodingQueue(LiveVideoDecodingQueue *pQueue);
		void PushVideoDataVector(int offset, unsigned char* uchVideoData, int iLen, int numberOfFrames, int *frameSizes, std::vector< std::pair<int, int> > vMissingFrames);

	private:

		int missedFrameCounter;

		SmartPointer<CLockHandler> m_pLiveReceiverMutex;

		LiveVideoDecodingQueue *m_pLiveVideoDecodingQueue;
		CCommonElementsBucket* m_pCommonElementsBucket;
	};

} //namespace MediaSDK

#endif 
