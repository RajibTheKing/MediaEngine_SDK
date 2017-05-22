
#ifndef IPV_ENCODING_BUFFER_H
#define IPV_ENCODING_BUFFER_H

#include "SmartPointer.h"
#include "CommonTypes.h"
#include "Tools.h"
#include "Size.h"
#include "LogPrinter.h"

namespace MediaSDK
{

	class CEncodingBuffer
	{

	public:

		CEncodingBuffer();
		~CEncodingBuffer();

		int Queue(unsigned char *ucaCapturedVideoFrameData, int nLength, int iHeight, int iWidth, int nCaptureTimeDifference, int device_orientation);
		int DeQueue(unsigned char *ucaCapturedVideoFrameData, int &iHeight, int &iWidth, int &nrTimeDifferenceInQueue, int &nrCaptureTimeDifference, int &device_orientation);
		void IncreamentIndex(int &irIndex);
		int GetQueueSize();
		void ResetBuffer();

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
		int m_naDevice_orientation[MAX_VIDEO_ENCODER_BUFFER_SIZE];
		int m_iHeight[MAX_VIDEO_ENCODER_BUFFER_SIZE];
		int m_iWidth[MAX_VIDEO_ENCODER_BUFFER_SIZE];

		SmartPointer<CLockHandler> m_pEncodingBufferMutex;
	};

} //namespace MediaSDK

#endif 
