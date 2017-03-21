
#ifndef __ENCODED_FRAME_PACKETIZER_H_
#define __ENCODED_FRAME_PACKETIZER_H_

#define _CRT_SECURE_NO_WARNINGS

#include "AudioVideoEngineDefinitions.h"
#include "SendingBuffer.h"
#include "Size.h"
#include "Tools.h"
//#include "PacketHeader.h"
#include "VideoHeader.h"
#include "HashGenerator.h"

class CVideoCallSession;

class CCommonElementsBucket;

class CEncodedFramePacketizer
{

public:

	CEncodedFramePacketizer(CCommonElementsBucket* pcSharedObject, CSendingBuffer* pcSendingBuffer, CVideoCallSession *pVideoCallSession);
	~CEncodedFramePacketizer();

	int Packetize(LongLong llFriendID, unsigned char *ucaEncodedVideoFrameData, unsigned int unLength, int iFrameNumber, unsigned int unCaptureTimeDifference, int device_orientation, bool bIsDummy);

private:

	Tools m_Tools;

	int m_nOwnDeviceType;

	int m_nPacketSize;
    CVideoCallSession *m_pVideoCallSession;
    
	//CPacketHeader m_cPacketHeader;
    CVideoHeader m_cVideoHeader;
    
	CSendingBuffer *m_pcSendingBuffer;
	CCommonElementsBucket* m_pcCommonElementsBucket;
	unsigned char m_ucaPacket[MAX_VIDEO_PACKET_SENDING_PACKET_SIZE];
    long long llSendingquePrevTime;
    CHashGenerator *m_pHashGenerator;

};

#endif
