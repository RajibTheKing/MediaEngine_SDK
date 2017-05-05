#ifndef IPV_VIDEO_PACKET_DEPACKETIZER_H
#define IPV_VIDEO_PACKET_DEPACKETIZER_H

//#define _CRT_SECURE_NO_WARNINGS

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
//#include "PacketHeader.h"
#include "VideoHeader.h"


namespace IPV
{
	class thread;
}

class CCommonElementsBucket;
class CVideoCallSession;

class CEncodedFrameDepacketizer
{

public:

	CEncodedFrameDepacketizer(CCommonElementsBucket* sharedObject, CVideoCallSession *pVideoCallSession);
	~CEncodedFrameDepacketizer();

	void ResetEncodedFrameDepacketizer();

	//int Depacketize(unsigned char *in_data, unsigned int in_size, int PacketType, CPacketHeader &packetHeader);
	int Depacketize(unsigned char *in_data, unsigned int in_size, int PacketType, CVideoHeader &packetHeader);
	void MoveForward(int frame);
	int CreateNewIndex(int frame);
	void ClearFrame(int index, int frame);
	int GetReceivedFrame(unsigned char* data,int &nFramNumber,int &nEcodingTime,int nExpectedTime,int nRight, int &nOrientation);

	map<long long, long long> m_mFrameTimeStamp;
	map<long long, int> m_mFrameOrientation;

	Tools m_Tools;

private:
	int ProcessFrame(unsigned char *data,int index,int frameNumber,int &nFramNumber);
	int GetEncodingTime(int nFrameNumber);
	int GetOrientation(int nFrameNumber);
	bool m_bIsDpkgBufferFilledUp;
	int m_iFirstFrameReceived;

	int SafeFinder(int Data);

	long long m_iMaxFrameNumRecvd;
	long long m_FirstFrameEncodingTime;

	std::map<int, int> m_FrameTracker;
	long long m_FrontFrame;
	long long m_BackFrame;
	int m_Counter;
	int m_BufferSize;
	std::set<int> m_AvailableIndexes;

	CCommonElementsBucket* m_pCommonElementsBucket;
	CVideoCallSession* m_VideoCallSession;

	CVideoPacketBuffer m_CVideoPacketBuffer[DEPACKETIZATION_BUFFER_SIZE + 1];

	//CVideoHeader PacketHeader;	// use to hoy na

	unsigned char * m_pPacketToResend;

protected:

	SmartPointer<CLockHandler> m_pEncodedFrameDepacketizerMutex;

};

#endif