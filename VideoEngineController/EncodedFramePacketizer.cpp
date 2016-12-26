
#include "EncodedFramePacketizer.h"
#include "CommonElementsBucket.h"
#include "LogPrinter.h"
#include "Globals.h"
#include "VideoCallSession.h"


#define USE_HASH_GENERATOR_TO_PACKETIZE

CEncodedFramePacketizer::CEncodedFramePacketizer(CCommonElementsBucket* pcSharedObject, CSendingBuffer* pcSendingBuffer, CVideoCallSession *pVideoCallSession) :

m_nPacketSize(MAX_PACKET_SIZE_WITHOUT_HEADER),
m_pcCommonElementsBucket(pcSharedObject)

{
	CLogPrinter_Write(CLogPrinter::INFO, "CEncodedFramePacketizer::CEncodedFramePacketizer");

	m_pcSendingBuffer = pcSendingBuffer;

	CLogPrinter_Write(CLogPrinter::DEBUGS, "CEncodedFramePacketizer::CEncodedFramePacketizer Created");
    
    m_pVideoCallSession = pVideoCallSession;
    llSendingquePrevTime = 0;
    
    m_pHashGenerator = new CHashGenerator();
    m_pHashGenerator->SetSeedNumber(m_Tools.CurrentTimestamp() % SEED_MOD_VALUE);
    
}

CEncodedFramePacketizer::~CEncodedFramePacketizer()
{
    if(m_pHashGenerator != NULL)
    {
        delete m_pHashGenerator;
        m_pHashGenerator = NULL;
    }
}

int CEncodedFramePacketizer::Packetize(LongLong llFriendID, unsigned char *ucaEncodedVideoFrameData, unsigned int unLength, int iFrameNumber, unsigned int unCaptureTimeDifference, int device_orientation, bool bIsDummy)
{
	CLogPrinter_Write(CLogPrinter::DEBUGS, "CEncodedFramePacketizer::Packetize parsing started");

	int nOpponentVersion = m_pVideoCallSession->GetVersionController()->GetCurrentCallVersion();
    unsigned char uchSendVersion = 0;

	int nVersionWiseHeaderLength = VIDEO_HEADER_LENGTH;
    
    if(nOpponentVersion == -1 || nOpponentVersion == 0 || bIsDummy == true)
    {
        uchSendVersion = 0;
    }
    else
    {
        uchSendVersion = (unsigned char)m_pVideoCallSession->GetVersionController()->GetCurrentCallVersion();
    }

	int nPacketHeaderLenghtWithMediaType = nVersionWiseHeaderLength + 1;

	m_nPacketSize = MAX_VIDEO_PACKET_SIZE - nPacketHeaderLenghtWithMediaType;

	int nNumberOfPackets = (unLength + m_nPacketSize - 1) / m_nPacketSize;

	if (nNumberOfPackets > MAX_NUMBER_OF_PACKETS)
		return -1;

	CLogPrinter_Write(CLogPrinter::INFO, "CEncodedFramePacketizer::Packetize in_size " + m_Tools.IntegertoStringConvert(in_size) + " m_PacketSize " + m_Tools.IntegertoStringConvert(m_PacketSize));

    int nNetworkType = m_pVideoCallSession->GetBitRateController()->GetOwnNetworkType();
    unsigned char uchOwnVersion = m_pVideoCallSession->GetVersionController()->GetOwnVersion();
    int nOwnQualityLevel = m_pVideoCallSession->GetOwnVideoCallQualityLevel();
    int nCurrentCallQualityLevel = m_pVideoCallSession->GetCurrentVideoCallQualityLevel();

    if(bIsDummy) 
	{
		/*m_cPacketHeader.setPacketHeader(__NEGOTIATION_PACKET_TYPE,
                                        uchOwnVersion,
                                        0, 0, 0, unCaptureTimeDifference, 0, 0,
                                        nOwnQualityLevel, 0, nNetworkType);*/

		m_cVideoHeader.setPacketHeader(__NEGOTIATION_PACKET_TYPE,				//packetType
										uchOwnVersion,							//VersionCode
										VIDEO_HEADER_LENGTH,					//HeaderLength
										0,										//FPSByte
										0,										//FrameNumber
										nNetworkType,							//NetworkType
										0,										//Device Orientation
										nOwnQualityLevel,						//QualityLevel
										0,										//NumberofPacket
										0,										//PacketNumber
										unCaptureTimeDifference,				//TimeStamp
										0,										//PacketStartingIndex
										0										//PacketDataLength
										);

        m_ucaPacket[0] = VIDEO_PACKET_MEDIA_TYPE;


        //m_cPacketHeader.GetHeaderInByteArray(m_ucaPacket + 1);
		m_cVideoHeader.GetHeaderInByteArray(m_ucaPacket + 1);
		m_pcSendingBuffer->Queue(llFriendID, m_ucaPacket, nPacketHeaderLenghtWithMediaType, 0, 0);

        return 1;
    }

//    string __show = "#VP FrameNumber: "+Tools::IntegertoStringConvert(iFrameNumber)+"  NP: "+Tools::IntegertoStringConvert(nNumberOfPackets)+"  Size: "+Tools::IntegertoStringConvert(unLength);
//    LOGE("%s",__show.c_str());
	if ((m_pVideoCallSession->GetServiceType() == SERVICE_TYPE_LIVE_STREAM || m_pVideoCallSession->GetServiceType() == SERVICE_TYPE_SELF_STREAM) && m_pVideoCallSession->GetEntityType() != ENTITY_TYPE_VIEWER_CALLEE)
	{

        int nPacketNumber = 0;
        int nNumberOfPackets = 1;
        
        m_cVideoHeader.setPacketHeader(__VIDEO_PACKET_TYPE,             //packetType
                                       uchOwnVersion,                   //VersionCode
                                       VIDEO_HEADER_LENGTH,             //HeaderLength
                                       0,                               //FPSByte
                                       iFrameNumber,                    //FrameNumber
                                       nNetworkType,                    //NetworkType
                                       device_orientation,              //Device Orientation
                                       nCurrentCallQualityLevel,        //QualityLevel
                                       nNumberOfPackets,                //NumberofPacket
                                       nPacketNumber,                   //PacketNumber
                                       unCaptureTimeDifference,         //TimeStamp
                                       0,                               //PacketStartingIndex
                                       unLength                         //PacketDataLength
                                       );

        
        m_ucaPacket[0] = VIDEO_PACKET_MEDIA_TYPE;
        
        /* m_cPacketHeader.setPacketHeader(__VIDEO_PACKET_TYPE,
                                            uchSendVersion,
                                            iFrameNumber,
                                            nNumberOfPackets,
                                            nPacketNumber,
                                            unCaptureTimeDifference,
                                            0,
                                            unLength,
                                            nCurrentCallQualityLevel,
                                            device_orientation,
                                            nNetworkType);*/

        
        
		//m_cPacketHeader.GetHeaderInByteArray(m_ucaPacket + 1);
        //m_cPacketHeader.ShowDetails("JUST");
        
        m_cVideoHeader.GetHeaderInByteArray(m_ucaPacket + 1);
        m_cVideoHeader.ShowDetails("JUST");

		memcpy(m_ucaPacket + nPacketHeaderLenghtWithMediaType, ucaEncodedVideoFrameData , unLength);


        {
            m_pcSendingBuffer->Queue(llFriendID, m_ucaPacket, nPacketHeaderLenghtWithMediaType + unLength, iFrameNumber, nPacketNumber);
            
            //CLogPrinter_WriteLog(CLogPrinter::INFO, PACKET_LOSS_INFO_LOG ," &*&*Sending frameNumber: " + toolsObject.IntegertoStringConvert(frameNumber) + " :: PacketNo: " + toolsObject.IntegertoStringConvert(packetNumber));
        }
	}
    else
    {
        int iStartIndex;
#ifdef USE_HASH_GENERATOR_TO_PACKETIZE
        nNumberOfPackets = m_pHashGenerator->CalculateNumberOfPackets(iFrameNumber, unLength);
        iStartIndex = 0;
#endif
        
        
        for (int nPacketNumber = 0, nPacketizedDataLength = 0; nPacketizedDataLength < unLength; nPacketNumber++, nPacketizedDataLength += m_nPacketSize)
        {
#ifdef USE_HASH_GENERATOR_TO_PACKETIZE
            m_nPacketSize = m_pHashGenerator->GetHashedPacketSize(iFrameNumber, nPacketNumber);
#endif
            
            if (nPacketizedDataLength + m_nPacketSize > unLength)
                m_nPacketSize = unLength - nPacketizedDataLength;
            
            /*m_cPacketHeader.setPacketHeader(__VIDEO_PACKET_TYPE,
                                            uchSendVersion,
                                            iFrameNumber,
                                            nNumberOfPackets,
                                            nPacketNumber,
                                            unCaptureTimeDifference,
                                            0,
                                            m_nPacketSize,
                                            nCurrentCallQualityLevel,
                                            device_orientation,
                                            nNetworkType);
											*/

			m_cVideoHeader.setPacketHeader(__VIDEO_PACKET_TYPE,             //packetType
                                           uchSendVersion,                  //VersionCode
											VIDEO_HEADER_LENGTH,             //HeaderLength
											0,                               //FPSByte
											iFrameNumber,                    //FrameNumber
											nNetworkType,                    //NetworkType
											device_orientation,              //Device Orientation
											nCurrentCallQualityLevel,        //QualityLevel
											nNumberOfPackets,                //NumberofPacket
											nPacketNumber,                   //PacketNumber
											unCaptureTimeDifference,         //TimeStamp
											iStartIndex,                               //PacketStartingIndex
											m_nPacketSize                    //PacketDataLength
											);
            iStartIndex += m_nPacketSize;

            m_ucaPacket[0] = VIDEO_PACKET_MEDIA_TYPE;
            //m_cPacketHeader.GetHeaderInByteArray(m_ucaPacket + 1);
			m_cVideoHeader.GetHeaderInByteArray(m_ucaPacket + 1);
            //        m_cPacketHeader.ShowDetails("JUST");
            
            memcpy(m_ucaPacket + nPacketHeaderLenghtWithMediaType, ucaEncodedVideoFrameData + nPacketizedDataLength, m_nPacketSize);
            
            
            if(m_pVideoCallSession->GetResolationCheck() == false)
            {
                unsigned char *pEncodedFrame = m_ucaPacket;
                int PacketSize = nPacketHeaderLenghtWithMediaType + m_nPacketSize;
                //printf("Sending data for nFrameNumber--> %d\n", iFrameNumber);
                m_pVideoCallSession->PushPacketForMerging(++pEncodedFrame, --PacketSize, true);
                //            CLogPrinter_WriteLog(CLogPrinter::INFO, INSTENT_TEST_LOG, "Sending to self");
                m_pcSendingBuffer->Queue(llFriendID, m_ucaPacket, nPacketHeaderLenghtWithMediaType + m_nPacketSize, iFrameNumber, nPacketNumber);
            }
            else
            {
                //m_cPacketHeader.ShowDetails("SendingSide: ");
                m_pcSendingBuffer->Queue(llFriendID, m_ucaPacket, nPacketHeaderLenghtWithMediaType + m_nPacketSize, iFrameNumber, nPacketNumber);
                
                
                //CLogPrinter_WriteLog(CLogPrinter::INFO, PACKET_LOSS_INFO_LOG ," &*&*Sending frameNumber: " + toolsObject.IntegertoStringConvert(frameNumber) + " :: PacketNo: " + toolsObject.IntegertoStringConvert(packetNumber));
            }
        }
    }

	return 1;
}












