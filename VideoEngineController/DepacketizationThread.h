
#ifndef _VIDEO_DEPACKETIZATION_THREAD_H_
#define _VIDEO_DEPACKETIZATION_THREAD_H_

#include "Tools.h"
#include "SmartPointer.h"
#include "LogPrinter.h"
#include "VideoPacketQueue.h"
#include "PacketHeader.h"
#include "BitRateController.h"
#include "EncodedFrameDepacketizer.h"
#include "DecodingBuffer.h"

#include <thread>

class CCommonElementsBucket;

class CVideoDepacketizationThread
{

public:

	CVideoDepacketizationThread(LongLong friendID, CVideoPacketQueue *VideoPacketQueue, CVideoPacketQueue *RetransVideoPacketQueue, CVideoPacketQueue *MiniPacketQueue, BitRateController *BitRateController, CEncodedFrameDepacketizer *EncodedFrameDepacketizer, CCommonElementsBucket* CommonElementsBucket, unsigned int *miniPacketBandCounter);
	~CVideoDepacketizationThread();

	void StartDepacketizationThread();
	void StopDepacketizationThread();
	void DepacketizationThreadProcedure();
	static void *CreateVideoDepacketizationThread(void* param);

private:

	void UpdateExpectedFramePacketPair(pair<int, int> currentFramePacketPair, int iNumberOfPackets);
	void ExpectedPacket();

	bool bDepacketizationThreadRunning;
	bool bDepacketizationThreadClosed;

	CVideoPacketQueue *m_pVideoPacketQueue;						
	CVideoPacketQueue *m_pRetransVideoPacketQueue;				
	CVideoPacketQueue *m_pMiniPacketQueue;						
	CPacketHeader m_RcvdPacketHeader;							
	BitRateController *m_BitRateController;						
	CEncodedFrameDepacketizer *m_pEncodedFrameDepacketizer;		
	CCommonElementsBucket* m_pCommonElementsBucket;				

	pair<int, int> ExpectedFramePacketPair;						
	int iNumberOfPacketsInCurrentFrame;

	unsigned int *m_miniPacketBandCounter;								
	LongLong m_FriendID;										

	unsigned char m_PacketToBeMerged[MAX_VIDEO_DECODER_FRAME_SIZE];

	Tools m_Tools;

	SmartPointer<std::thread> pDepacketizationThread;
};

#endif 