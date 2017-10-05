
#ifndef IPV_MultiResolutionThread_OF_LIVE_H
#define IPV_MultiResolutionThread_OF_LIVE_H

#include "Tools.h"
#include "SmartPointer.h"
#include "LogPrinter.h"
#include "VideoFrameBuffer.h"
#include "ColorConverter.h"
#include <thread>
#include "CommonElementsBucket.h"

namespace MediaSDK
{

	class MultiResolutionThread
	{

	public:

		MultiResolutionThread(VideoFrameBuffer *pcVideoFrameBuffer, CCommonElementsBucket *pCommonElementsBucket, int *targetHeight, int *targetWidth, int iLen);
		~MultiResolutionThread();

		void StartMultiResolutionThread();
		void StopMultiResolutionThread();
		void MultiResolutionThreadProcedure();
		static void *CreateMultiResolutionThread(void* pParam);


	private:

		int *m_TargetHeight;
		int *m_TargetWidth;
		int m_Len;

		bool m_bMultiResolutionThreadRunning;
		bool m_bMultiResolutionThreadClosed;

		unsigned char m_ucaVideoFrame[MAX_VIDEO_DECODER_FRAME_SIZE];

		unsigned char m_ucaMultVideoFrame[5][MAX_VIDEO_DECODER_FRAME_SIZE];

		Tools m_Tools;

		VideoFrameBuffer *m_pcVideoFrameBuffer;
		CCommonElementsBucket* m_pcCommonElementsBucket;
		CColorConverter *m_pColorConverter;

		SharedPointer<std::thread> pMultiResolutionThread;
	};

} //namespace MediaSDK

#endif 
