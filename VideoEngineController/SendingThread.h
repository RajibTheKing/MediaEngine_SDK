
#ifndef _SENDING_THREAD_H_
#define _SENDING_THREAD_H_

#include "Tools.h"
#include "SmartPointer.h"
#include "LogPrinter.h"
#include "SendingBuffer.h"
#include "EncodingBuffer.h"
#include "BandwidthController.h"
#include <thread>

class CCommonElementsBucket;
class CFPSController;

class CSendingThread
{

public:

	CSendingThread(CCommonElementsBucket* commonElementsBucket, CSendingBuffer *sendingBuffer, CFPSController *FPSController);
	~CSendingThread();

	void StartSendingThread();
	void StopSendingThread();
	void SendingThreadProcedure();
	static void *CreateVideoSendingThread(void* param);

	int GetSleepTime();

private:
    
#ifdef  BANDWIDTH_CONTROLLING_TEST
    std::vector<int>m_TimePeriodInterval;
    std::vector<int>m_BandWidthList;
    BandwidthController m_BandWidthController;
#endif
    
	bool bSendingThreadRunning;
	bool bSendingThreadClosed;

	CCommonElementsBucket* m_pCommonElementsBucket;		
	CSendingBuffer *m_SendingBuffer;						

	CFPSController *g_FPSController;

	unsigned char m_EncodedFrame[MAX_VIDEO_ENCODER_FRAME_SIZE];

	Tools m_Tools;

	SmartPointer<std::thread> pSendingThread;
};

#endif 