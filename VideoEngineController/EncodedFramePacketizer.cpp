
#include "EncodedFramePacketizer.h"
#include "CommonElementsBucket.h"
#include "LogPrinter.h"
#include "Globals.h"
#include "VideoCallSession.h"

extern unsigned char g_uchSendPacketVersion;

CEncodedFramePacketizer::CEncodedFramePacketizer(CCommonElementsBucket* pcSharedObject, CSendingBuffer* pcSendingBuffer, CVideoCallSession *pVideoCallSession) :

m_nPacketSize(MAX_PACKET_SIZE_WITHOUT_HEADER),
m_pcCommonElementsBucket(pcSharedObject)

{
	CLogPrinter_Write(CLogPrinter::INFO, "CEncodedFramePacketizer::CEncodedFramePacketizer");

	m_pcSendingBuffer = pcSendingBuffer;

	CLogPrinter_Write(CLogPrinter::DEBUGS, "CEncodedFramePacketizer::CEncodedFramePacketizer Created");
    
    m_pVideoCallSession = pVideoCallSession;
}

CEncodedFramePacketizer::~CEncodedFramePacketizer()
{

}

int CEncodedFramePacketizer::Packetize(LongLong llFriendID, unsigned char *ucaEncodedVideoFrameData, unsigned int unLength, int iFrameNumber, unsigned int unCaptureTimeDifference)
{
	CLogPrinter_Write(CLogPrinter::DEBUGS, "CEncodedFramePacketizer::Packetize parsing started");

	unsigned char uchOpponentVersion = g_uchSendPacketVersion;

	int nVersionWiseHeaderLength;

	if (uchOpponentVersion)
		nVersionWiseHeaderLength = PACKET_HEADER_LENGTH;
	else
		nVersionWiseHeaderLength = PACKET_HEADER_LENGTH_NO_VERSION;

	int nPacketHeaderLenghtWithMediaType = nVersionWiseHeaderLength + 1;

	m_nPacketSize = MAX_VIDEO_PACKET_SIZE - nPacketHeaderLenghtWithMediaType;

	int nNumberOfPackets = (unLength + m_nPacketSize - 1) / m_nPacketSize;

	if (nNumberOfPackets > MAX_NUMBER_OF_PACKETS)
		return -1;

	CLogPrinter_Write(CLogPrinter::INFO, "CEncodedFramePacketizer::Packetize in_size " + m_Tools.IntegertoStringConvert(in_size) + " m_PacketSize " + m_Tools.IntegertoStringConvert(m_PacketSize));

	for (int nPacketNumber = 0, nPacketizedDataLength = 0; nPacketizedDataLength < unLength; nPacketNumber++, nPacketizedDataLength += m_nPacketSize)
	{
		if (nPacketizedDataLength + m_nPacketSize > unLength)
			m_nPacketSize = unLength - nPacketizedDataLength;

		if (uchOpponentVersion)
			m_cPacketHeader.setPacketHeader(uchOpponentVersion, iFrameNumber, nNumberOfPackets, nPacketNumber, unCaptureTimeDifference, 0, 0, m_nPacketSize + nPacketHeaderLenghtWithMediaType);
		else
			m_cPacketHeader.setPacketHeader(uchOpponentVersion, iFrameNumber, nNumberOfPackets, nPacketNumber, unCaptureTimeDifference, 0, 0, m_nPacketSize);

		m_cPacketHeader.GetHeaderInByteArray(m_ucaPacket + 1);

		m_ucaPacket[0] = VIDEO_PACKET_MEDIA_TYPE;

		memcpy(m_ucaPacket + nPacketHeaderLenghtWithMediaType, ucaEncodedVideoFrameData + nPacketizedDataLength, m_nPacketSize);

//		m_pCommonElementsBucket->m_pEventNotifier->firePacketEvent(m_pCommonElementsBucket->m_pEventNotifier->ENCODED_PACKET, frameNumber, numberOfPackets, packetNumber, m_PacketSize, nPacketHeaderLenghtWithMedia + m_PacketSize, m_Packet);

//		CLogPrinter_WriteInstentTestLog(CLogPrinter::INFO, "CEncodedFramePacketizer::Packetize Queue lFriendID " + Tools::IntegertoStringConvert(lFriendID) + " packetSize " + Tools::IntegertoStringConvert(nPacketHeaderLenghtWithMedia + m_PacketSize));
		
        
        /*CPacketHeader packetHeader;
        
        int startPoint = RESEND_INFO_START_BYTE_WITH_MEDIA_TYPE;
        
        packetHeader.setPacketHeader(m_ucaPacket + 1);
        
        unsigned char signal = g_FPSController->GetFPSSignalByte();
        m_ucaPacket[1 + SIGNAL_BYTE_INDEX_WITHOUT_MEDIA] = signal;*/
        
        
        m_pcSendingBuffer->Queue(llFriendID, m_ucaPacket, nPacketHeaderLenghtWithMediaType + m_nPacketSize, iFrameNumber, nPacketNumber);
        
        /*
        if(m_pVideoCallSession->GetResolationCheck() == false)
        {
            unsigned char *pEncodedFrame = m_ucaPacket;
            int PacketSize = nPacketHeaderLenghtWithMediaType + m_nPacketSize;
            m_pVideoCallSession->PushPacketForMerging(++pEncodedFrame, --PacketSize);

            m_pcSendingBuffer->Queue(llFriendID, m_ucaPacket, PACKET_HEADER_LENGTH_WITH_MEDIA_TYPE, iFrameNumber, nPacketNumber);
        }
        else
        {
            if(m_pVideoCallSession->GetHighResolutionSupportStatus() == true)
            {
                m_cPacketHeader.SetResolutionBit(m_ucaPacket, 2);
            }
            else
            {
                m_cPacketHeader.SetResolutionBit(m_ucaPacket, 1);
            }
            
            m_pcSendingBuffer->Queue(llFriendID, m_ucaPacket, nPacketHeaderLenghtWithMediaType + m_nPacketSize, iFrameNumber, nPacketNumber);
            
            //CLogPrinter_WriteLog(CLogPrinter::INFO, PACKET_LOSS_INFO_LOG ," &*&*Sending frameNumber: " + toolsObject.IntegertoStringConvert(frameNumber) + " :: PacketNo: " + toolsObject.IntegertoStringConvert(packetNumber));
        }*/
        
        
        
		
	}

	return 1;
}












