
#ifndef IPV_SENDING_THREAD_OF_CALL_H
#define IPV_SENDING_THREAD_OF_CALL_H

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
//class CVideoHeader;

//#define CHANNEL_FROM_FILE

class CSendingThreadOfCall
{
public:

	CSendingThreadOfCall(CCommonElementsBucket* commonElementsBucket, CSendingBuffer *sendingBuffer, CVideoCallSession* pVideoCallSession, bool bIsCheckCall, long long llfriendID, bool bAudioOnlyLive);
	~CSendingThreadOfCall();

	void StartSendingThread();
	void StopSendingThread();
	void SendingThreadProcedure();
	static void *CreateVideoSendingThread(void* param);

private:
	int GetSleepTime();

	long long m_nTimeStampOfChunck;
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
    
	long long m_lfriendID;

//	CVideoHeader m_cVH;

	long long int llPrevTime;
    long long m_llPrevTimeWhileSendingToLive;


	Tools m_Tools;

	SmartPointer<std::thread> pSendingThread;
};

#endif 
