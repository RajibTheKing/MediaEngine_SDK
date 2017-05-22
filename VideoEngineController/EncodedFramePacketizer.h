
#ifndef IPV_ENCODED_FRAME_PACKETIZER_H
#define IPV_ENCODED_FRAME_PACKETIZER_H

//#define _CRT_SECURE_NO_WARNINGS

#include "AudioVideoEngineDefinitions.h"
#include "SendingBuffer.h"
#include "Size.h"
#include "Tools.h"
//#include "PacketHeader.h"
#include "VideoHeader.h"
#include "HashGenerator.h"

namespace MediaSDK
{

	class CVideoCallSession;
	class CCommonElementsBucket;

	class CEncodedFramePacketizer
	{

	public:

		CEncodedFramePacketizer(CCommonElementsBucket* pcSharedObject, CSendingBuffer* pcSendingBuffer, CVideoCallSession *pVideoCallSession);
		~CEncodedFramePacketizer();

		int Packetize(long long llFriendID, unsigned char *ucaEncodedVideoFrameData, unsigned int unLength, int iFrameNumber, unsigned int unCaptureTimeDifference, int device_orientation, bool bIsDummy);

	private:

		int m_nOwnDeviceType;
		int m_nPacketSize;

		unsigned char m_ucaPacket[MAX_VIDEO_PACKET_SENDING_PACKET_SIZE];

		Tools m_Tools;
		//CPacketHeader m_cPacketHeader;

		CVideoCallSession *m_pcVideoCallSession;
		CVideoHeader m_cVideoHeader;
		CSendingBuffer *m_pcSendingBuffer;
		CCommonElementsBucket* m_pcCommonElementsBucket;
		CHashGenerator *m_pcHashGenerator;
	};

} //namespace MediaSDK

#endif
