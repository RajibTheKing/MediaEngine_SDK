
#ifndef _VIDEO_DEPACKETIZATION_THREAD_H_
#define _VIDEO_DEPACKETIZATION_THREAD_H_

#include "Tools.h"
#include "SmartPointer.h"
#include "LogPrinter.h"
#include "VideoPacketQueue.h"
//#include "PacketHeader.h"
#include "VideoHeader.h"
#include "BitRateController.h"
#include "IDRFrameIntervalController.h"
#include "EncodedFrameDepacketizer.h"
#include "VersionController.h"
#include "IDRFrameIntervalController.h"



#include <thread>

class CCommonElementsBucket;

class CVideoDepacketizationThread
{

public:

	CVideoDepacketizationThread(LongLong friendID, CVideoPacketQueue *VideoPacketQueue, CVideoPacketQueue *RetransVideoPacketQueue, CVideoPacketQueue *MiniPacketQueue, BitRateController *BitRateController, IDRFrameIntervalController *pIdrFrameController, CEncodedFrameDepacketizer *EncodedFrameDepacketizer, CCommonElementsBucket* CommonElementsBucket, unsigned int *miniPacketBandCounter, CVersionController *pVersionController, CVideoCallSession* pVideoCallSession);
	~CVideoDepacketizationThread();

	void StartDepacketizationThread();
	void StopDepacketizationThread();
	void DepacketizationThreadProcedure();
	static void *CreateVideoDepacketizationThread(void* param);

	void ResetForPublisherCallerCallEnd();

private:

	void UpdateExpectedFramePacketPair(pair<int, int> currentFramePacketPair, int iNumberOfPackets);
	void ExpectedPacket();

	bool bDepacketizationThreadRunning;
	bool bDepacketizationThreadClosed;

	CVideoCallSession *m_pVideoCallSession;
	CVideoPacketQueue *m_pVideoPacketQueue;						
	CVideoPacketQueue *m_pRetransVideoPacketQueue;				
	CVideoPacketQueue *m_pMiniPacketQueue;						
	CVideoHeader m_RcvdPacketHeader;							
	BitRateController *m_BitRateController;
    IDRFrameIntervalController *m_pIdrFrameIntervalController;
	CEncodedFrameDepacketizer *m_pEncodedFrameDepacketizer;		
	CCommonElementsBucket* m_pCommonElementsBucket;
    
    

	bool m_bResetForPublisherCallerCallEnd;

	pair<int, int> ExpectedFramePacketPair;						
	int iNumberOfPacketsInCurrentFrame;

	unsigned int *m_miniPacketBandCounter;								
	LongLong m_FriendID;										

	unsigned char m_PacketToBeMerged[MAX_VIDEO_DECODER_FRAME_SIZE];

	Tools m_Tools;
    CVersionController *m_pVersionController;
    
    SmartPointer<std::thread> pDepacketizationThread;
};

#endif 
