
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

	CVideoDepacketizationThread(LongLong friendID, CVideoPacketQueue *VideoPacketQueue, CVideoPacketQueue *RetransVideoPacketQueue, CVideoPacketQueue *MiniPacketQueue, BitRateController *BitRateController, CEncodedFrameDepacketizer *EncodedFrameDepacketizer, CCommonElementsBucket* CommonElementsBucket, int *miniPacketBandCounter);
	~CVideoDepacketizationThread();

	void StartDepacketizationThread();
	void StopDepacketizationThread();
	void DepacketizationThreadProcedure();
	static void *CreateVideoDepacketizationThread(void* param);

	void CreateAndSendMiniPacket(int resendFrameNumber, int resendPacketNumber);
	void UpdateExpectedFramePacketPair(pair<int, int> currentFramePacketPair, int iNumberOfPackets);

private:

	bool bDepacketizationThreadRunning;
	bool bDepacketizationThreadClosed;

	CVideoPacketQueue *m_pVideoPacketQueue;						// bring
	CVideoPacketQueue *m_pRetransVideoPacketQueue;				// bring
	CVideoPacketQueue *m_pMiniPacketQueue;						// bring
	CPacketHeader m_RcvdPacketHeader;							
	BitRateController *m_BitRateController;						// bring
	CEncodedFrameDepacketizer *m_pEncodedFrameDepacketizer;		// bring
	CCommonElementsBucket* m_pCommonElementsBucket;				// bring

	pair<int, int> ExpectedFramePacketPair;						
	int iNumberOfPacketsInCurrentFrame;							

	int *m_miniPacketBandCounter;								// bring
	LongLong m_FriendID;										// bring

	unsigned char m_PacketToBeMerged[MAX_VIDEO_DECODER_FRAME_SIZE];
	unsigned char m_miniPacket[PACKET_HEADER_LENGTH_NO_VERSION + 1];

	Tools m_Tools;

	SmartPointer<std::thread> pDepacketizationThread;
};

#endif 