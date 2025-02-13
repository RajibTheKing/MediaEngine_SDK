
#ifndef IPV_LIVE_RECEIVER_H
#define IPV_LIVE_RECEIVER_H

#include "SmartPointer.h"
#include "CommonTypes.h"
#include "Size.h"
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
		bool isComplement(int firstFrame, int secondFrame, int offset, unsigned char* uchVideoData, int numberOfFrames, int *frameSizes, std::vector<std::pair<int, int>>& vMissingFrames, unsigned char* constructedFrame);
		bool isComplementEfficient(int firstFrame, int secondFrame, int offset, unsigned char* uchVideoData, int numberOfFrames, int *frameSizes, std::vector<std::pair<int, int>>& vMissingFrames, unsigned char* constructedFrame);
		void PushVideoDataVector(bool isCheckForDuplicate, int offset, unsigned char* uchVideoData, int iLen, int numberOfFrames, int *frameSizes, std::vector< std::pair<int, int> > vMissingFrames, int serviceType, long long llCurrentChunkRelativeTime);

	private:

		int m_nMissedFrameCounter;
		int m_nGotFrameCounter;
		int m_nIFrameGap;
		int m_nPreviousIFrameNumber;

		SharedPointer<CLockHandler> m_pLiveReceiverMutex;

		unsigned char m_pBackupData[MAX_VIDEO_ENCODER_FRAME_SIZE];

		LiveVideoDecodingQueue *m_pLiveVideoDecodingQueue;
		CCommonElementsBucket* m_pCommonElementsBucket;
	};

} //namespace MediaSDK

#endif 
