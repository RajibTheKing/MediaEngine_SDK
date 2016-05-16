
#include "EncodedFramePacketizer.h"
#include "CommonElementsBucket.h"
#include "LogPrinter.h"
#include "Globals.h"
#include "VideoCallSession.h"

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

int CEncodedFramePacketizer::Packetize(LongLong llFriendID, unsigned char *ucaEncodedVideoFrameData, unsigned int unLength, int iFrameNumber, unsigned int unCaptureTimeDifference, int device_orientation, bool bIsDummy)
{
	CLogPrinter_Write(CLogPrinter::DEBUGS, "CEncodedFramePacketizer::Packetize parsing started");

	int nOpponentVersion = m_pVideoCallSession->GetVersionController()->GetCurrentCallVersion();
    unsigned char uchSendVersion = 0;

	int nVersionWiseHeaderLength;
    
    if(nOpponentVersion == -1 || nOpponentVersion == 0 || bIsDummy == true)
    {
        nVersionWiseHeaderLength = PACKET_HEADER_LENGTH_NO_VERSION;
        uchSendVersion = 0;
    }
    else
    {
        uchSendVersion = (unsigned char)m_pVideoCallSession->GetVersionController()->GetCurrentCallVersion();
        nVersionWiseHeaderLength = PACKET_HEADER_LENGTH;
    }

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
        
        if(bIsDummy && m_nPacketSize == 1)
        {
            ++ m_nPacketSize;
        }
            
		if (uchSendVersion) {
            if(1 == uchSendVersion)
                m_cPacketHeader.setPacketHeader(uchSendVersion, iFrameNumber, nNumberOfPackets, nPacketNumber, unCaptureTimeDifference, 0, 0, m_nPacketSize + nPacketHeaderLenghtWithMediaType, device_orientation);
            else
                m_cPacketHeader.setPacketHeader(uchSendVersion, iFrameNumber, nNumberOfPackets, nPacketNumber, unCaptureTimeDifference, 0, 0, m_nPacketSize, device_orientation);

            m_ucaPacket[ RETRANSMISSION_SIG_BYTE_INDEX_WITHOUT_MEDIA] |= (m_pVideoCallSession->GetOwnVideoCallQualityLevel() << 1); //Resolution, FPS
            m_ucaPacket[RETRANSMISSION_SIG_BYTE_INDEX_WITHOUT_MEDIA] |= m_pVideoCallSession->GetBitRateController()->GetOwnNetworkType(); //2G
        }
		else
        {
            m_cPacketHeader.setPacketHeader(uchSendVersion, bIsDummy? 0 : iFrameNumber, bIsDummy? 0 : nNumberOfPackets, bIsDummy? 0 :nPacketNumber, unCaptureTimeDifference, 0, 0, m_nPacketSize);
        }
//Packet lenght issue should be fixed.
		m_cPacketHeader.GetHeaderInByteArray(m_ucaPacket + 1);

		//m_cPacketHeader.SetDeviceOrientation(m_ucaPacket+5);
		int deviceoritationTemp = m_cPacketHeader.GetDeviceOrientation();

		CLogPrinter_WriteLog(CLogPrinter::DEBUGS, INSTENT_TEST_LOG, "device orientaion : >>>>>>>>>>>>>>>>>>>>>>>>>  " + m_Tools.IntegertoStringConvert(deviceoritationTemp) + " ......>> " +m_Tools.IntegertoStringConvert(device_orientation));

		m_ucaPacket[0] = VIDEO_PACKET_MEDIA_TYPE;

		memcpy(m_ucaPacket + nPacketHeaderLenghtWithMediaType, ucaEncodedVideoFrameData + nPacketizedDataLength, m_nPacketSize);
        if(bIsDummy)
        {
            printf("TheVersion--> Sending dummy with own version = %d\n", m_pVideoCallSession->GetVersionController()->GetOwnVersion());
            m_ucaPacket[ nPacketHeaderLenghtWithMediaType ] = m_pVideoCallSession->GetVersionController()->GetOwnVersion();
			m_ucaPacket[nPacketHeaderLenghtWithMediaType + 1] = 0;
            m_ucaPacket[ nPacketHeaderLenghtWithMediaType + 1] |= (m_pVideoCallSession->GetOwnVideoCallQualityLevel() << 1); //Resolution, FPS
			m_ucaPacket[nPacketHeaderLenghtWithMediaType + 1] |= m_pVideoCallSession->GetBitRateController()->GetOwnNetworkType(); //2G
        }

//		m_pCommonElementsBucket->m_pEventNotifier->firePacketEvent(m_pCommonElementsBucket->m_pEventNotifier->ENCODED_PACKET, frameNumber, numberOfPackets, packetNumber, m_PacketSize, nPacketHeaderLenghtWithMedia + m_PacketSize, m_Packet);

//		CLogPrinter_WriteInstentTestLog(CLogPrinter::INFO, "CEncodedFramePacketizer::Packetize Queue lFriendID " + Tools::IntegertoStringConvert(lFriendID) + " packetSize " + Tools::IntegertoStringConvert(nPacketHeaderLenghtWithMedia + m_PacketSize));
		
        
        /*CPacketHeader packetHeader;
        
        int startPoint = RESEND_INFO_START_BYTE_WITH_MEDIA_TYPE;
        
        packetHeader.setPacketHeader(m_ucaPacket + 1);
        
        unsigned char signal = g_FPSController->GetFPSSignalByte();
        m_ucaPacket[1 + SIGNAL_BYTE_INDEX_WITHOUT_MEDIA] = signal;*/
        
        
        //m_pcSendingBuffer->Queue(llFriendID, m_ucaPacket, nPacketHeaderLenghtWithMediaType + m_nPacketSize, iFrameNumber, nPacketNumber);
        
        
        if(m_pVideoCallSession->GetResolationCheck() == false)
        {
            unsigned char *pEncodedFrame = m_ucaPacket;
            int PacketSize = nPacketHeaderLenghtWithMediaType + m_nPacketSize;
            printf("Sending data for nFrameNumber--> %d\n", iFrameNumber);
            m_pVideoCallSession->PushPacketForMerging(++pEncodedFrame, --PacketSize, true);
//            CLogPrinter_WriteLog(CLogPrinter::INFO, INSTENT_TEST_LOG, "Sending to self");
            m_pcSendingBuffer->Queue(llFriendID, m_ucaPacket, nPacketHeaderLenghtWithMediaType + m_nPacketSize, iFrameNumber, nPacketNumber);
        }
        else
        {
            /*if(bIsDummy == false)
            {
                if(m_pVideoCallSession->GetHighResolutionSupportStatus() == true)
                {
                    
                    m_cPacketHeader.SetResolutionBit(m_ucaPacket, 2);
                    
                    //CLogPrinter_WriteLog(CLogPrinter::INFO, INSTENT_TEST_LOG, "Set HighResolutionSupportStatus 2, get = " + m_Tools.IntegertoStringConvert(m_cPacketHeader.GetOpponentResolution(m_ucaPacket+1)));
                    
                }
                else
                {
                    
                    m_cPacketHeader.SetResolutionBit(m_ucaPacket, 1);
                    
                    //CLogPrinter_WriteLog(CLogPrinter::INFO, INSTENT_TEST_LOG, "set HighResolutionSupportStatus 1, get = " + m_Tools.IntegertoStringConvert(m_cPacketHeader.GetOpponentResolution(m_ucaPacket+1)));
                }
                
                //printf("TheVersion--> Sending RealData\n");
            }*/
            
            
            m_pcSendingBuffer->Queue(llFriendID, m_ucaPacket, nPacketHeaderLenghtWithMediaType + m_nPacketSize, iFrameNumber, nPacketNumber);
            
            //CLogPrinter_WriteLog(CLogPrinter::INFO, PACKET_LOSS_INFO_LOG ," &*&*Sending frameNumber: " + toolsObject.IntegertoStringConvert(frameNumber) + " :: PacketNo: " + toolsObject.IntegertoStringConvert(packetNumber));
        }
        
        
        
		
	}

	return 1;
}












