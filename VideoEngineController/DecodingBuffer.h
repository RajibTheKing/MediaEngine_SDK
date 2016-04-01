
#ifndef _DECODING_BUFFER_H_
#define _DECODING_BUFFER_H_

#include "SmartPointer.h"
#include "LockHandler.h"
#include "Tools.h"
#include "Size.h"

class CDecodingBuffer
{

public:

	CDecodingBuffer();
	~CDecodingBuffer();

	int Queue(int iFrameNumber, unsigned char *ucaEncodedVideoFrameData, int nLength, unsigned int unCaptureTimeDifference);
	int DeQueue(int &irFrameNumber, unsigned int &unCaptureTimeDifference, unsigned char *ucaEncodedVideoFrameData, int &unrTimeDifferenceInQueue);
	void IncreamentIndex(int &irIndex);
	int GetQueueSize();

private:

	int m_iPushIndex;
	int m_iPopIndex;
	int m_nQueueCapacity;
	int m_nQueueSize;

	Tools m_Tools;

	unsigned char m_uc2aEncodedVideoDataBuffer[MAX_VIDEO_DECODER_BUFFER_SIZE][MAX_VIDEO_DECODER_FRAME_SIZE];
	int m_naBufferDataLengths[MAX_VIDEO_DECODER_BUFFER_SIZE];
	int m_naBufferFrameNumbers[MAX_VIDEO_DECODER_BUFFER_SIZE];
	unsigned int m_unaBufferCaptureTimeDifferences[MAX_VIDEO_DECODER_BUFFER_SIZE];
	long long m_llBufferInsertionTimes[MAX_VIDEO_DECODER_BUFFER_SIZE];

	SmartPointer<CLockHandler> m_pDecodingBufferMutex;
};

#endif 
