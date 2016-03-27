#ifndef __ENCODED_FRAME_PACKETIZER_H_
#define __ENCODED_FRAME_PACKETIZER_H_

#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <string>

#include "SmartPointer.h"
#include "LockHandler.h"
#include "ThreadTools.h"
#include "AudioVideoEngineDefinitions.h"
#include "EncodingBuffer.h"
#include "SendingBuffer.h"
#include "Size.h"
#include "Tools.h"
#include "PacketHeader.h"
#include "SendingThread.h"

namespace IPV
{
	class thread;
}

class CCommonElementsBucket;

class CEncodedFramePacketizer
{

public:

	CEncodedFramePacketizer(CCommonElementsBucket* sharedObject, CSendingBuffer* pSendingBuffer);
	~CEncodedFramePacketizer();

	int Packetize(LongLong lFriendID, unsigned char *in_data, unsigned int in_size, int frameNumber, unsigned int iTimeStampDiff);

private:
	Tools m_Tools;
	int m_PacketSize;

	CPacketHeader m_PacketHeader;

	CSendingBuffer *m_SendingBuffer;

	unsigned char m_EncodedFrame[MAX_VIDEO_ENCODER_FRAME_SIZE];

	CCommonElementsBucket* m_pCommonElementsBucket;

	unsigned char m_Packet[MAX_VIDEO_PACKET_SIZE];

protected:

	SmartPointer<CLockHandler> m_pEncodedFrameParsingMutex;

};

#endif