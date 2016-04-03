
#ifndef _ENCODING_BUFFER_H_
#define _ENCODING_BUFFER_H_

#include "SmartPointer.h"
#include "LockHandler.h"
#include "Tools.h"
#include "Size.h"

class CEncodingBuffer
{

public:

	CEncodingBuffer();
	~CEncodingBuffer();

	int Queue(unsigned char *ucaCapturedVideoFrameData, int nLength, int nCaptureTimeDifference);
	int DeQueue(unsigned char *ucaCapturedVideoFrameData, int &nrTimeDifferenceInQueue, int &nrCaptureTimeDifference);
	void IncreamentIndex(int &irIndex);
	int GetQueueSize();

private:

	int m_iPushIndex;
	int m_iPopIndex;
	int m_nQueueCapacity;
	int m_nQueueSize;

	Tools m_Tools;

	unsigned char m_uc2aCapturedVideoDataBuffer[MAX_VIDEO_ENCODER_BUFFER_SIZE][MAX_VIDEO_ENCODER_FRAME_SIZE];

	int m_naBufferDataLengths[MAX_VIDEO_ENCODER_BUFFER_SIZE];

	int m_naBufferCaptureTimeDifferences[MAX_VIDEO_ENCODER_BUFFER_SIZE];
	long long m_llaBufferInsertionTimes[MAX_VIDEO_ENCODER_BUFFER_SIZE];

	SmartPointer<CLockHandler> m_pEncodingBufferMutex;
};

#endif 
