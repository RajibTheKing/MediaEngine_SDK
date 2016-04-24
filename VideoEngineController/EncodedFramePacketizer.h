
#ifndef __ENCODED_FRAME_PACKETIZER_H_
#define __ENCODED_FRAME_PACKETIZER_H_

#define _CRT_SECURE_NO_WARNINGS

#include "AudioVideoEngineDefinitions.h"
#include "SendingBuffer.h"
#include "Size.h"
#include "Tools.h"
#include "PacketHeader.h"

class CCommonElementsBucket;

class CEncodedFramePacketizer
{

public:

	CEncodedFramePacketizer(CCommonElementsBucket* pcSharedObject, CSendingBuffer* pcSendingBuffer);
	~CEncodedFramePacketizer();

	int Packetize(LongLong llFriendID, unsigned char *ucaEncodedVideoFrameData, unsigned int unLength, int iFrameNumber, unsigned int unCaptureTimeDifference, int device_orientation);

private:

	Tools m_Tools;

	int m_nPacketSize;

	CPacketHeader m_cPacketHeader;
	CSendingBuffer *m_pcSendingBuffer;
	CCommonElementsBucket* m_pcCommonElementsBucket;

	unsigned char m_ucaPacket[MAX_VIDEO_PACKET_SIZE];
};

#endif