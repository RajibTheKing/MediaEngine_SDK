
#ifndef IPV_RENDERING_THREAD_OF_LIVE_H
#define IPV_RENDERING_THREAD_OF_LIVE_H

#include "Tools.h"
#include "SmartPointer.h"
#include "LogPrinter.h"
#include "RenderingBuffer.h"
#include "AverageCalculator.h"

#include <thread>

namespace MediaSDK
{

	class CCommonElementsBucket;
	class CVideoCallSession;

	class CRenderingThreadOfLive
	{

	public:

		CRenderingThreadOfLive(long long llFriendID, CRenderingBuffer *pcRenderingBuffer, CCommonElementsBucket* pcCommonElementsBucket, CVideoCallSession *pcVideoCallSession, bool bIsCheckCall);
		~CRenderingThreadOfLive();

		void StartRenderingThread();
		void StopRenderingThread();
		void RenderingThreadProcedure();
		static void *CreateVideoRenderingThread(void* pParam);
		void CalculateFPS();

	private:

		bool m_bRenderingThreadRunning;
		bool m_bRenderingThreadClosed;
		bool m_bIsCheckCall;
		int m_nRenderFrameCount;
		int m_nInsetHeight;
		int m_nInsetWidth;
		long long m_llFriendID;
		long long m_lRenderCallTime;
		long long m_llRenderFrameCounter;

		unsigned char m_ucaRenderingFrame[MAX_VIDEO_DECODER_FRAME_SIZE];

		Tools m_Tools;
		CAverageCalculator m_cRenderTimeCalculator;

		CRenderingBuffer *m_pcRenderingBuffer;
		CCommonElementsBucket* m_pcCommonElementsBucket;
		CVideoCallSession *m_pcVideoCallSession;

		SharedPointer<std::thread> pRenderingThread;
	};

} //namespace MediaSDK

#endif 
