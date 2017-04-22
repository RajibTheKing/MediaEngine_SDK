
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
    m_pcVideoCallSession = pVideoCallSession;

	m_nOwnDeviceType = pVideoCallSession->GetOwnDeviceType();
    
    m_pcHashGenerator = new CHashGenerator();
    m_pcHashGenerator->SetSeedNumber(m_Tools.CurrentTimestamp() % SEED_MOD_VALUE);

	CLogPrinter_Write(CLogPrinter::DEBUGS, "CEncodedFramePacketizer::CEncodedFramePacketizer Created");
}

CEncodedFramePacketizer::~CEncodedFramePacketizer()
{
	if (NULL != m_pcHashGenerator)
    {
        delete m_pcHashGenerator;
        m_pcHashGenerator = NULL;
    }
}

int CEncodedFramePacketizer::Packetize(LongLong llFriendID, unsigned char *ucaEncodedVideoFrameData, unsigned int unLength, int iFrameNumber, unsigned int unCaptureTimeDifference, int device_orientation, bool bIsDummy)
{
	CLogPrinter_Write(CLogPrinter::DEBUGS, "CEncodedFramePacketizer::Packetize parsing started");
    
    int nNumberOfInsets = 1;

	int height = m_pcVideoCallSession->GetColorConverter()->GetSmallFrameHeight();
	int width = m_pcVideoCallSession->GetColorConverter()->GetSmallFrameWidth();

	int pInsetHeights[] = { height, 0, 0 }, pInsetWidths[] = { width, 0, 0 }; //testing heights widths

	int nOpponentVersion = m_pcVideoCallSession->GetVersionController()->GetCurrentCallVersion();
    unsigned char uchSendVersion = 0;

	int nVersionWiseHeaderLength = VIDEO_HEADER_LENGTH;
    
    if(nOpponentVersion == -1 || nOpponentVersion == 0 || bIsDummy == true)
    {
        uchSendVersion = 0;
    }
    else
    { 
        uchSendVersion = (unsigned char)m_pcVideoCallSession->GetVersionController()->GetCurrentCallVersion();
    }

	int nPacketHeaderLenghtWithMediaType = nVersionWiseHeaderLength + 1;

	m_nPacketSize = MAX_VIDEO_PACKET_SIZE - nPacketHeaderLenghtWithMediaType;

	int nNumberOfPackets = (unLength + m_nPacketSize - 1) / m_nPacketSize;

	if (nNumberOfPackets > MAX_NUMBER_OF_PACKETS)
		return -1;

	CLogPrinter_Write(CLogPrinter::INFO, "CEncodedFramePacketizer::Packetize in_size " + m_Tools.IntegertoStringConvert(in_size) + " m_PacketSize " + m_Tools.IntegertoStringConvert(m_PacketSize));

    int nNetworkType = m_pcVideoCallSession->GetBitRateController()->GetOwnNetworkType();
    unsigned char uchOwnVersion = m_pcVideoCallSession->GetVersionController()->GetOwnVersion();
    int nOwnQualityLevel = m_pcVideoCallSession->GetOwnVideoCallQualityLevel();
    int nCurrentCallQualityLevel = m_pcVideoCallSession->GetCurrentVideoCallQualityLevel();

    if(bIsDummy) 
	{
		m_cVideoHeader.setPacketHeader(NEGOTIATION_PACKET_TYPE,					//packetType
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
										0,										//PacketDataLength
										m_nOwnDeviceType,						//SenderDeviceType
                                        nNumberOfInsets,                        //NumberOfInsets
                                        pInsetHeights,                          //InsetHeights
                                        pInsetWidths                            //InsetWidths
										);

        m_ucaPacket[0] = VIDEO_PACKET_MEDIA_TYPE;

        //m_cPacketHeader.GetHeaderInByteArray(m_ucaPacket + 1);
		m_cVideoHeader.GetHeaderInByteArray(m_ucaPacket + 1);
		m_pcSendingBuffer->Queue(llFriendID, m_ucaPacket, nPacketHeaderLenghtWithMediaType, 0, 0);

        return 1;
    }

//    string __show = "#VP FrameNumber: "+Tools::IntegertoStringConvert(iFrameNumber)+"  NP: "+Tools::IntegertoStringConvert(nNumberOfPackets)+"  Size: "+Tools::IntegertoStringConvert(unLength);
//    LOGE("%s",__show.c_str());
	if ((m_pcVideoCallSession->GetServiceType() == SERVICE_TYPE_LIVE_STREAM || m_pcVideoCallSession->GetServiceType() == SERVICE_TYPE_SELF_STREAM || m_pcVideoCallSession->GetServiceType() == SERVICE_TYPE_CHANNEL))
	{

        int nPacketNumber = 0;
        int nNumberOfPackets = 1;
        
        m_cVideoHeader.setPacketHeader(VIDEO_PACKET_TYPE,				//packetType
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
                                       unLength,                         //PacketDataLength
                                       m_nOwnDeviceType,				 //SenderDeviceType
                                       nNumberOfInsets,                  //NumberOfInsets
                                       pInsetHeights,                    //InsetHeights
                                       pInsetWidths                      //InsetWidths
                                       );

        
        m_ucaPacket[0] = VIDEO_PACKET_MEDIA_TYPE;
        
        /* m_cPacketHeader.setPacketHeader(VIDEO_PACKET_TYPE,
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



        CVideoHeader pH;
        pH.setPacketHeader(m_ucaPacket+1);
        pH.ShowDetails("JUST2");
        m_pcSendingBuffer->Queue(llFriendID, m_ucaPacket, nPacketHeaderLenghtWithMediaType + unLength, iFrameNumber, nPacketNumber);
            
        //CLogPrinter_WriteLog(CLogPrinter::INFO, PACKET_LOSS_INFO_LOG ," &*&*Sending frameNumber: " + toolsObject.IntegertoStringConvert(frameNumber) + " :: PacketNo: " + toolsObject.IntegertoStringConvert(packetNumber));
	}
    else
    {
        int iStartIndex;
        
        nNumberOfPackets = m_pcHashGenerator->CalculateNumberOfPackets(iFrameNumber, unLength);
        iStartIndex = 0;
        
        for (int nPacketNumber = 0, nPacketizedDataLength = 0; nPacketizedDataLength < unLength; nPacketNumber++, nPacketizedDataLength += m_nPacketSize)
        {
            m_nPacketSize = m_pcHashGenerator->GetHashedPacketSize(iFrameNumber, nPacketNumber);
            
            if (nPacketizedDataLength + m_nPacketSize > unLength)
                m_nPacketSize = unLength - nPacketizedDataLength;

			m_cVideoHeader.setPacketHeader(VIDEO_PACKET_TYPE,             //packetType
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
											iStartIndex,                     //PacketStartingIndex
											m_nPacketSize,                    //PacketDataLength
                                           m_nOwnDeviceType,						//SenderDeviceType
                                           nNumberOfInsets,                        //NumberOfInsets
                                           pInsetHeights,                          //InsetHeights
                                           pInsetWidths                            //InsetWidths
                                           );
            iStartIndex += m_nPacketSize;

			if ((m_pcVideoCallSession->GetServiceType() == SERVICE_TYPE_LIVE_STREAM || m_pcVideoCallSession->GetServiceType() == SERVICE_TYPE_SELF_STREAM || m_pcVideoCallSession->GetServiceType() == SERVICE_TYPE_CHANNEL))
			{
				m_ucaPacket[1] = VIDEO_PACKET_MEDIA_TYPE;
				//m_cPacketHeader.GetHeaderInByteArray(m_ucaPacket + 1);
				m_cVideoHeader.GetHeaderInByteArray(m_ucaPacket + 2);
				//        m_cPacketHeader.ShowDetails("JUST");

				memcpy(m_ucaPacket + nPacketHeaderLenghtWithMediaType + 1, ucaEncodedVideoFrameData + nPacketizedDataLength, m_nPacketSize);
			}
			else
			{
				m_ucaPacket[0] = VIDEO_PACKET_MEDIA_TYPE;
				//m_cPacketHeader.GetHeaderInByteArray(m_ucaPacket + 1);
				m_cVideoHeader.GetHeaderInByteArray(m_ucaPacket + 1);
				//        m_cPacketHeader.ShowDetails("JUST");

				memcpy(m_ucaPacket + nPacketHeaderLenghtWithMediaType, ucaEncodedVideoFrameData + nPacketizedDataLength, m_nPacketSize);
			}
 
            if(m_pcVideoCallSession->GetResolationCheck() == false)
            {
                unsigned char *pEncodedFrame = m_ucaPacket;
                int PacketSize = nPacketHeaderLenghtWithMediaType + m_nPacketSize;
                //printf("Sending data for nFrameNumber--> %d\n", iFrameNumber);
                m_pcVideoCallSession->PushPacketForMerging(++pEncodedFrame, --PacketSize, true);
                //            CLogPrinter_WriteLog(CLogPrinter::INFO, INSTENT_TEST_LOG, "Sending to self");
                m_pcSendingBuffer->Queue(llFriendID, m_ucaPacket, nPacketHeaderLenghtWithMediaType + m_nPacketSize, iFrameNumber, nPacketNumber);
            }
            else
            {
                //m_cPacketHeader.ShowDetails("SendingSide: ");
				if ((m_pcVideoCallSession->GetServiceType() == SERVICE_TYPE_LIVE_STREAM || m_pcVideoCallSession->GetServiceType() == SERVICE_TYPE_SELF_STREAM || m_pcVideoCallSession->GetServiceType() == SERVICE_TYPE_CHANNEL))
                {
                    m_pcSendingBuffer->Queue(llFriendID, m_ucaPacket, nPacketHeaderLenghtWithMediaType + m_nPacketSize + 1, iFrameNumber, nPacketNumber);
                }
                else
                {
                    m_pcSendingBuffer->Queue(llFriendID, m_ucaPacket, nPacketHeaderLenghtWithMediaType + m_nPacketSize, iFrameNumber, nPacketNumber);
                }
                
                //CLogPrinter_WriteLog(CLogPrinter::INFO, PACKET_LOSS_INFO_LOG ," &*&*Sending frameNumber: " + toolsObject.IntegertoStringConvert(frameNumber) + " :: PacketNo: " + toolsObject.IntegertoStringConvert(packetNumber));
            }
        }
    }

	return 1;
}












