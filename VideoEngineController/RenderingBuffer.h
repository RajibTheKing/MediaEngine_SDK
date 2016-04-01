
#ifndef _RENDERING_BUFFER_H_
#define _RENDERING_BUFFER_H_

#include "SmartPointer.h"
#include "LockHandler.h"
#include "Tools.h"

#define MAX_VIDEO_RENDERER_BUFFER_SIZE 30
#define MAX_VIDEO_RENDERER_FRAME_SIZE 352 * 288 * 3

class CRenderingBuffer
{

public:

	CRenderingBuffer();
	~CRenderingBuffer();

	int Queue(int frameNumber, unsigned char *frame, int length, long long timeStampDiff, int videoHeight, int videoWidth);
	int DeQueue(int &frameNumber,long long &timeStampDiff, unsigned char *decodeBuffer, int &videoHeight, int &videoWidth, int &timeDiff);
	void IncreamentIndex(int &index);
	int GetQueueSize();

private:

	int m_iPushIndex;
	int m_iPopIndex;
	int m_iQueueCapacity;
	int m_iQueueSize;

	Tools m_Tools;

	unsigned char m_Buffer[MAX_VIDEO_RENDERER_BUFFER_SIZE][MAX_VIDEO_RENDERER_FRAME_SIZE];
	int m_BufferDataLength[MAX_VIDEO_RENDERER_BUFFER_SIZE];
	int m_VideoHeights[MAX_VIDEO_RENDERER_BUFFER_SIZE];
	int m_VideoWidths[MAX_VIDEO_RENDERER_BUFFER_SIZE];
	int m_BufferFrameNumber[MAX_VIDEO_RENDERER_BUFFER_SIZE];
	long long m_BufferInsertionTime[MAX_VIDEO_RENDERER_BUFFER_SIZE];

	long long m_BufferTimeStamp[MAX_VIDEO_RENDERER_BUFFER_SIZE];

	SmartPointer<CLockHandler> m_pChannelMutex;
};

#endif 
