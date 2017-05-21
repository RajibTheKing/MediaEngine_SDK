
#ifndef IPV_VIDEO_PACKET_BUFFER_H
#define IPV_VIDEO_PACKET_BUFFER_H

//#define _CRT_SECURE_NO_WARNINGS

#include "SmartPointer.h"
#include "CommonTypes.h"
#include "Size.h"

class CVideoPacketBuffer
{

public:

	CVideoPacketBuffer();
	~CVideoPacketBuffer();

	void Reset();
	bool PushVideoPacket(unsigned char *pucVideoPacketData, unsigned int unLength, int nPacketNumber, int iHeaderLength, int nPacketStartingIndex);
	int IsComplete();
	void SetNumberOfPackets(int nNumberOfPackets);
    void SetFrameNumber(int nFrameNumber);

	bool m_baPacketTracker[MAX_NUMBER_OF_PACKETS];
	unsigned char m_ucaFrameData[MAX_NUMBER_OF_PACKETS * MAX_PACKET_SIZE_WITHOUT_HEADER];
	int m_nNumberOfPackets;
	int m_nNumberOfGotPackets;
	int m_nFrameSize;
	bool m_bIsClear;
    int m_nFrameNumber;

};

#endif
