#ifndef _VIDEO_PACKET_BUFFER_H_
#define _VIDEO_PACKET_BUFFER_H_

#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <string>

#include "SmartPointer.h"
#include "LockHandler.h"
#include "ThreadTools.h"

#include "Size.h"

#include <vector>

class CVideoPacketBuffer
{

public:

	CVideoPacketBuffer();
	~CVideoPacketBuffer();

	void Reset();
	bool PushVideoPacket(unsigned char *in_data, unsigned int in_size, int packetNumber);
	int IsComplete();
	void SetNumberOfPackets(int number);
	bool IsIFrame();

	bool m_pPacketTracker[MAX_NUMBER_OF_PACKETS];
	unsigned char m_pFrameData[MAX_NUMBER_OF_PACKETS*MAX_PACKET_SIZE_WITHOUT_HEADER];
	int m_NumberOfPackets;
	int m_NumberOfGotPackets;
	int m_FrameSize;
	bool m_isClear;

};

#endif