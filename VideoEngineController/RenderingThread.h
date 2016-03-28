
#ifndef _VIDEO_RENDERING_THREAD_H_
#define _VIDEO_RENDERING_THREAD_H_

#include "Tools.h"
#include "SmartPointer.h"
#include "LogPrinter.h"
#include "DecodingBuffer.h"
#include "RenderingBuffer.h"

#include <thread>

class CCommonElementsBucket;

class CVideoRenderingThread
{

public:

	CVideoRenderingThread(LongLong friendID, CRenderingBuffer *renderingBuffer, CCommonElementsBucket* commonElementsBucket);
	~CVideoRenderingThread();

	void StartRenderingThread();
	void StopRenderingThread();
	void RenderingThreadProcedure();
	static void *CreateVideoRenderingThread(void* param);

private:

	bool bRenderingThreadRunning;
	bool bRenderingThreadClosed;

	CRenderingBuffer *m_RenderingBuffer;					
	CCommonElementsBucket* m_pCommonElementsBucket;		

	LongLong m_FriendID;								

	unsigned char m_RenderingFrame[MAX_VIDEO_DECODER_FRAME_SIZE];

	Tools m_Tools;

	SmartPointer<std::thread> pRenderingThread;
};

#endif 