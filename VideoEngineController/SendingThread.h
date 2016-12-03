
#ifndef _SENDING_THREAD_H_
#define _SENDING_THREAD_H_

#include "Tools.h"
#include "SmartPointer.h"
#include "LogPrinter.h"
#include "SendingBuffer.h"
#include "EncodingBuffer.h"
#include "BandwidthController.h"
#include <thread>

class CVideoCallSession;
class CCommonElementsBucket;
class CFPSController;

class CSendingThread
{
public:

	CSendingThread(CCommonElementsBucket* commonElementsBucket, CSendingBuffer *sendingBuffer, CVideoCallSession* pVideoCallSession, bool bIsCheckCall);
	~CSendingThread();

	void StartSendingThread();
	void StopSendingThread();
	void SendingThreadProcedure();
	static void *CreateVideoSendingThread(void* param);

private:
	int GetSleepTime();

	int m_nTimeStampOfChunck;
	int m_nTimeStampOfChunckSend;

	CVideoCallSession* m_pVideoCallSession;
#ifdef  BANDWIDTH_CONTROLLING_TEST
    std::vector<int>m_TimePeriodInterval;
    std::vector<int>m_BandWidthList;
    BandwidthController m_BandWidthController;
#endif
    
	bool bSendingThreadRunning;
	bool bSendingThreadClosed;

	CCommonElementsBucket* m_pCommonElementsBucket;		
	CSendingBuffer *m_SendingBuffer;

	bool m_bIsCheckCall;

	unsigned char m_EncodedFrame[MAX_VIDEO_PACKET_SENDING_PACKET_SIZE];
    

	unsigned char m_VideoDataToSend[MAX_VIDEO_DATA_TO_SEND_SIZE];
	unsigned char m_AudioDataToSend[MAX_AUDIO_DATA_TO_SEND_SIZE];
	unsigned char m_AudioVideoDataToSend[MAX_AUDIO_VIDEO_DATA_TO_SEND_SIZE];
	int m_iAudioDataToSendIndex;

	bool firstFrame;
	int m_iDataToSendIndex;
	long long int llPrevTime;
    long long m_llPrevTimeWhileSendingToLive;


	Tools m_Tools;

	SmartPointer<std::thread> pSendingThread;
};

#endif 
