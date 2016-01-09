#ifndef _VIDEO_PACKET_MERGER_H_
#define _VIDEO_PACKET_MERGER_H_

#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <string>
#include <map>
#include <set>
#include <vector>

#include "SmartPointer.h"
#include "LockHandler.h"
#include "ThreadTools.h"
#include "VideoPacketBuffer.h"
#include "AudioVideoEngineDefinitions.h"
#include "Size.h"
#include "Tools.h"


namespace IPV
{
	class thread;
}

class CCommonElementsBucket;
class CVideoCallSession;

class CVideoPacketMerger
{

public:

	CVideoPacketMerger(CCommonElementsBucket* sharedObject, CVideoCallSession *pVideoCallSession);
	~CVideoPacketMerger();

	int PushPacketForDecoding(unsigned char *in_data, unsigned int in_size);
	void ClearAndDeliverFrame(int frame);
	void MoveForward(int frame);
	int CreateNewIndex(int frame);
	void ClearFrame(int index, int frame);
	//int GetPacketLength(unsigned char *packetData, int start_index);
	void StartVideoPacketMergerThread();
	void StopVideoPacketMergerThread();
//	void FrameDroppedFPS(int frame);
//	void FrameCompleteFPS(int frame);

	static void *CreateVideoPacketMergerThread(void* param);

	map<int, int> m_mFrameTimeStamp;

	Tools m_Tools;

private:
	int SafeFinder(int Data);

	int m_iRetransPktDrpd;
	int m_iRetransPktUsed;
	int m_iIntervalDroppedFPS;
	int m_nIntervalLastFrameNumber;
	int m_iCountResendPktSent;
	int m_iCountReqResendPacket;
	int m_iMaxFrameNumRecvd;
	int m_iMaxFrameNumRecvdOld;
	long long m_LastDecoderSentTime;
	unsigned int timeStamp;

	LongLong lastTimeStamp;

	int fpsCompleteFrame;


	std::map<int, int> m_FrameTracker;
	int m_FrontFrame;
	int m_BackFrame;
	int m_Counter;
	int m_BufferSize;
	std::set<int> m_AvailableIndexes;

	CCommonElementsBucket* m_pCommonElementsBucket;
	CVideoCallSession* m_VideoCallSession;

	CVideoPacketBuffer m_CVideoPacketBuffer[DEPACKETIZATION_BUFFER_SIZE + 1];

	unsigned char * m_pPacketToResend;

protected:

	std::thread* m_pVideoPacketMergerThread;

	SmartPointer<CLockHandler> m_pVideoPacketMergerMutex;

};

#endif