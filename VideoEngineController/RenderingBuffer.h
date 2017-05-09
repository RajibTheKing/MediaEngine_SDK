
#ifndef IPV_RENDERING_BUFFER_H
#define IPV_RENDERING_BUFFER_H

#include "SmartPointer.h"
#include "Size.h"

namespace MediaSDK
{

	class CLockHandler;

	class CRenderingBuffer
	{

	public:

		CRenderingBuffer();
		~CRenderingBuffer();

		int Queue(int nFrameNumber, unsigned char *ucaDecodedVideoFrameData, int nLength, long long llCaptureTimeDifference, int nVideoHeight, int nVideoWidth, int nOrientation, int nInsetHeight, int nInsetWidth);
		int DeQueue(int &rnFrameNumber, long long &rllCaptureTimeDifference, unsigned char *ucaDecodedVideoFrameData, int &rnVideoHeight, int &rnVideoWidth, int &rnTimeDifferenceInQueue, int &nOrientation, int &nInsetHeight, int &nInsetWidth);
		void IncreamentIndex(int &riIndex);
		int GetQueueSize();
		void ResetBuffer();

	private:

		int m_iPushIndex;
		int m_iPopIndex;
		int m_nQueueCapacity;
		int m_nQueueSize;

		unsigned char m_uc2aDecodedVideoDataBuffer[MAX_VIDEO_RENDERER_BUFFER_SIZE][MAX_VIDEO_RENDERER_FRAME_SIZE];

		int m_naBufferDataLengths[MAX_VIDEO_RENDERER_BUFFER_SIZE];
		int m_naBufferFrameNumbers[MAX_VIDEO_RENDERER_BUFFER_SIZE];
		int m_naBufferVideoHeights[MAX_VIDEO_RENDERER_BUFFER_SIZE];
		int m_naBufferVideoWidths[MAX_VIDEO_RENDERER_BUFFER_SIZE];
		int m_naBufferVideoOrientations[MAX_VIDEO_RENDERER_BUFFER_SIZE];
		int m_naBufferInsetHeights[MAX_VIDEO_RENDERER_BUFFER_SIZE];
		int m_naBufferInsetWidths[MAX_VIDEO_RENDERER_BUFFER_SIZE];
		long long m_llaBufferInsertionTimes[MAX_VIDEO_RENDERER_BUFFER_SIZE];
		long long m_llaBufferCaptureTimeDifferences[MAX_VIDEO_RENDERER_BUFFER_SIZE];

		SmartPointer<CLockHandler> m_pRenderingBufferMutex;
	};

} //namespace MediaSDK

#endif 
