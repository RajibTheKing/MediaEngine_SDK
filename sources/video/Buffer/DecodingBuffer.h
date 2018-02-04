
#ifndef IPV_DECODING_BUFFER_H
#define IPV_DECODING_BUFFER_H

#include "SmartPointer.h"
#include "CommonTypes.h"
#include "Tools.h"
#include "Size.h"
#include "LogPrinter.h"

namespace MediaSDK
{

	class CDecodingBuffer
	{

	public:

		CDecodingBuffer();
		~CDecodingBuffer();

		int Queue(int iFrameNumber, unsigned char *ucaEncodedVideoFrameData, int nLength, unsigned int unCaptureTimeDifference);
		int DeQueue(int &irFrameNumber, unsigned int &unrCaptureTimeDifference, unsigned char *ucaEncodedVideoFrameData, int &nrTimeDifferenceInQueue);
		void IncreamentIndex(int &irIndex);
		int GetQueueSize();

	private:

		int m_iPushIndex;
		int m_iPopIndex;
		int m_nQueueCapacity;
		int m_nQueueSize;

		int m_nMaxQueueSizeTillNow;

		Tools m_Tools;

		unsigned char m_uc2aEncodedVideoDataBuffer[MAX_VIDEO_DECODER_BUFFER_SIZE][MAX_VIDEO_DECODER_FRAME_SIZE];

		int m_naBufferDataLengths[MAX_VIDEO_DECODER_BUFFER_SIZE];
		int m_naBufferFrameNumbers[MAX_VIDEO_DECODER_BUFFER_SIZE];

		unsigned int m_unaBufferCaptureTimeDifferences[MAX_VIDEO_DECODER_BUFFER_SIZE];
		long long m_llaBufferInsertionTimes[MAX_VIDEO_DECODER_BUFFER_SIZE];

		SharedPointer<CLockHandler> m_pDecodingBufferMutex;
	};

} //namespace MediaSDK

#endif 
