//
// Created by ipvision on 1/23/2016.
//

#include "VideoHeader.h"
#include "LogPrinter.h"
#include "Size.h"
#include "Tools.h"

#define PACKET_TYPE_INDEX 0
#define VERSION_CODE_INDEX 1
#define HEADER_LENGTH_INDEX 2
#define FPS_INDEX 3
#define FRAME_NUMBER_INDEX 4
#define CALL_INFO_INDEX 8
#define NUMBER_OF_PACKET_INDEX 9
#define PACKET_NUMBER_INDEX 10
#define TIMESTAMP_INDEX 11
#define PACKET_STARTING_INDEX 16
#define PACKET_DATA_LENGTH_INDEX 19
#define SENDER_DEVICE_TYPE_INDEX 22

//Inset Information
#define NUMBER_OF_INSET_INDEX 23
#define INSET_HEIGHT_WIDTH_INDEX 24

//MoreInformation, While Version = 2
#define SIGMA_VALUE_INDEX 28
#define BRIGHTNESS_VALUE_INDEX 29
#define DEVICE_FPS_INDEX 30
#define ENCODE_FAIL_INDEX 30
#define LIBRARY_VERSION_INDEX 31

//LiveVideoQualityLevel
#define LIVE_VIDEO_QUALITY_LEVEL_INDEX 32

//LiveStreamBitrate
#define LIVE_STREAM_VIDEO_BITRATE_INDEX 33
#define LIVE_STREAM_VIDEO_MAX_BITRATE_INDEX 36

//VideoHeightWidth by 4
#define VIDEO_HEIGTH_FOURTH_INDEX 39
#define VIDEO_WIDTH_FOURTH_INDEX 40

//Inset Upper offset
#define INSET_UPPER_OFFSET_INDEX 41

#define QUALITY_BITS_N      3
#define ORIENTATION_BITS_N  2

#define QUALITY_LEVEL_BITSET    ((1<<QUALITY_BITS_N) - 1)
#define ORIENTATION_BITSET      ((1<<ORIENTATION_BITS_N) - 1)

namespace MediaSDK
{

	CVideoHeader::CVideoHeader()
	{
		m_iPacketType = 0; //1 byte
		m_iVersionCode = 0;    // 1 byte
		m_iHeaderLength = 0;    //1 byte

		//FPS Byte
		m_iFPS = 0;             // 1 byte
		m_iOpponentFPS = 0;
		m_iFPSForceBit = 0;
		m_iFPSChange = 0;

		m_llFrameNumber = 0;     // 4 byte

		//Call Info Byte
		m_iCallInfo = 0;        //1 byte
		m_iNetworkType = 0;
		m_iVideoQualityLevel = 0;
		m_iDeviceOrientation = 0;


		m_iNumberOfPacket = 0;  // 1 byte
		m_iPacketNumber = 0;    // 1 byte

		m_llTimeStamp = 0;       // 5 byte

		m_iPacketStartingIndex = 0; //3 byte
		m_iPacketDataLength = 0;    //3 byte

		m_nSenderDeviceType = 0; // 1 byte

		//insetInfo
		m_nNumberOfInset = 0; //1 byte
		m_nInsetHeight[0] = m_nInsetHeight[1] = m_nInsetHeight[2] = 0; // 2 byte per height
		m_nInsetWidth[0] = m_nInsetWidth[1] = m_nInsetWidth[2] = 0; // 2 byte per height
        
        
        //MoreInformation
        m_iSigmaValue = 0;  //1 byte
        m_iBrightnessValue = 0; //1 byte
        
        m_iDeviceFPS = 0; //5 bit
        m_iNumberOfEncodeFailPerFPS = 0; //5 bit
        m_iMediaEngineVersion = 0; //6 bit
        
        //LiveVideoQualityLevel
        m_iLiveVideoQualityLevel = 0; //3 bit
        
        //LiveStreamBitrates
        m_iLiveStreamBitrate = 0; //3 byte
        m_iLiveStreamMaxBitrate = 0; //3 byte
        
        
        //VideoHeightWidth by 4
        m_iVideoHeightFourth = 0; //1 byte
        m_iVideoWidthFourth = 0; //1 byte
        
        //Inset Upper Offset
        m_iInsetUppderOffset = 0; //2 byte
        

	}

	CVideoHeader::~CVideoHeader()
	{

	}

	void CVideoHeader::SetPacketHeader(unsigned char *headerData)
	{
        int nowIndex = 0;
        
        SetPacketType(headerData + PACKET_TYPE_INDEX);                              nowIndex += 1;
		SetVersionCode(headerData + VERSION_CODE_INDEX);                            nowIndex += 1;
        SetHeaderLength(headerData + HEADER_LENGTH_INDEX);                          nowIndex += 1;
		SetFPS(headerData + FPS_INDEX);                                             nowIndex += 1;
		SetFrameNumber(headerData + FRAME_NUMBER_INDEX);                            nowIndex += 4;

		//CallInfoByte
		SetDeviceOrientation(headerData + CALL_INFO_INDEX);
		SetVideoQualityLevel(headerData + CALL_INFO_INDEX);
		SetNetworkType(headerData + CALL_INFO_INDEX);                               nowIndex += 1;

		SetNumberOfPacket(headerData + NUMBER_OF_PACKET_INDEX);                     nowIndex += 1;
		SetPacketNumber(headerData + PACKET_NUMBER_INDEX);                          nowIndex += 1;
		SetTimeStamp(headerData + TIMESTAMP_INDEX);                                 nowIndex += 5;

		SetPacketStartingIndex(headerData + PACKET_STARTING_INDEX);                 nowIndex += 3;
		SetPacketDataLength(headerData + PACKET_DATA_LENGTH_INDEX);                 nowIndex += 3;

		SetSenderDeviceType(headerData + SENDER_DEVICE_TYPE_INDEX);                 nowIndex += 1;

		SetNumberOfInset(headerData + NUMBER_OF_INSET_INDEX);                       nowIndex += 1;
		SetInsetHeights(headerData + INSET_HEIGHT_WIDTH_INDEX, m_nNumberOfInset);   nowIndex += 2;
		SetInsetWidths(headerData + INSET_HEIGHT_WIDTH_INDEX, m_nNumberOfInset);    nowIndex += 2;
        
        
        if(m_iVersionCode == 2 || m_iVersionCode == 3 || m_iVersionCode == 4 || m_iVersionCode == 5 || m_iVersionCode == 6)
        {
            printf("here inside new Header parsing\n");
            SetSigmaValue(headerData + SIGMA_VALUE_INDEX);                          nowIndex += 1;
            SetBrightnessValue(headerData + BRIGHTNESS_VALUE_INDEX);                nowIndex += 1;
            SetDeviceFPS(headerData + DEVICE_FPS_INDEX);                            //nowIndex += 1;
            SetEncodeFailPerFPS(headerData + ENCODE_FAIL_INDEX);                    nowIndex += 1;
            SetLibraryVersion(headerData + LIBRARY_VERSION_INDEX);                  nowIndex += 1;
            
        }
        
        if(m_iVersionCode == 3 || m_iVersionCode == 4 || m_iVersionCode == 5 || m_iVersionCode == 6)
        {
            SetLiveVideoQualityLevel(headerData + LIVE_VIDEO_QUALITY_LEVEL_INDEX);  nowIndex += 1;
        }
        
        if(m_iVersionCode == 4 || m_iVersionCode == 5 || m_iVersionCode == 6)
        {
            setLiveStreamVideoBitrate(headerData + LIVE_STREAM_VIDEO_BITRATE_INDEX);         nowIndex += 3;
            setLiveStreamVideoMaxBitrate(headerData + LIVE_STREAM_VIDEO_MAX_BITRATE_INDEX);  nowIndex += 3;
        }
        
        if(m_iVersionCode == 5 || m_iVersionCode == 6)
        {
            setVideoHeightFourth(headerData + VIDEO_HEIGTH_FOURTH_INDEX);                   nowIndex += 1;
            setVideoWidthFourth(headerData + VIDEO_WIDTH_FOURTH_INDEX);                     nowIndex += 1;
        }
        
        if(m_iVersionCode == 6)
        {
            setInsetUpperOffset(headerData + INSET_UPPER_OFFSET_INDEX);                     nowIndex += 2;
        }
	}
    
	void CVideoHeader::SetPacketHeader(unsigned char packetType,
                                       unsigned char uchVersion,
                                       unsigned int iHeaderLength,
                                       unsigned int iFPSbyte,
                                       long long llFrameNumber,
                                       int iNetworkType,
                                       int iDeviceOrientation,
                                       int iQualityLevel,
                                       unsigned int NumberOfPacket,
                                       unsigned int PacketNumber,
                                       long long llTimeStamp,
                                       unsigned int iPacketStartingIndex,
                                       unsigned int PacketLength,
                                       int senderDeviceType,
                                       int nNumberOfInsets,
                                       int *pInsetHeights,
                                       int *pInsetWidths,
                                       //MoreInfo
                                       int iSigmaValue,
                                       int iBrightnessValue,
                                       int iDeviceFPS,
                                       int iNumberOfEncodeFailPerFps,
                                       int iMediaEngineVersion,
                                       int iLiveVideoQualityLevel,
                                       int iLiveStreamBitrate,
                                       int iLiveStreamMaxBitrate,
                                       int iVideoHeightFourth,
                                       int iVideoWidthFourth,
                                       int iInsetUpperOffset
                                       
		)
	{
		
        m_iPacketType = packetType;
        m_iVersionCode = uchVersion;
        m_iHeaderLength = iHeaderLength;
        m_iFPS = iFPSbyte;
		m_llFrameNumber = llFrameNumber;

		//CallInfoByte
		m_iNetworkType = iNetworkType;
		m_iDeviceOrientation = iDeviceOrientation;
        m_iVideoQualityLevel = iQualityLevel;

		m_iNumberOfPacket = NumberOfPacket;
        m_iPacketNumber = PacketNumber;

		m_llTimeStamp = llTimeStamp;
        
		m_iPacketStartingIndex = iPacketStartingIndex;
		m_iPacketDataLength = PacketLength;
		m_nSenderDeviceType = senderDeviceType;

		
        //InsetInfo
        m_nNumberOfInset = nNumberOfInsets;
        for (int i = 0; i < nNumberOfInsets; i++)
        {
            m_nInsetHeight[i] = pInsetHeights[i];
        }
        for (int i = 0; i < nNumberOfInsets; i++)
        {
            m_nInsetWidth[i] = pInsetWidths[i];
        }
        
        //MoreInfo
        m_iSigmaValue = iSigmaValue;
        m_iBrightnessValue = iBrightnessValue;
        m_iDeviceFPS = iDeviceFPS;
        m_iNumberOfEncodeFailPerFPS = iNumberOfEncodeFailPerFps;
        m_iMediaEngineVersion = iMediaEngineVersion;
        
        //LiveVideoQualityLevel
        m_iLiveVideoQualityLevel = iLiveVideoQualityLevel;
        
        //LiveVideo Bitrate
        m_iLiveStreamBitrate = iLiveStreamBitrate;
        
        //LiveVideo MaxBitrate
        m_iLiveStreamMaxBitrate = iLiveStreamMaxBitrate;
        
        //VideoHeightWidth by 4
        m_iVideoHeightFourth = iVideoHeightFourth;
        m_iVideoWidthFourth = iVideoWidthFourth;
        
        //InsetUpperOffset
        m_iInsetUppderOffset = iInsetUpperOffset;
    }

	void CVideoHeader::ShowDetails(string sTag)
	{
		string sLog = "#PKT#  -> " + sTag
			+ " PT: " + Tools::getText(m_iPacketType)
			+ " VC:" + Tools::getText(m_iVersionCode)
			+ " HL:" + Tools::getText(m_iHeaderLength)
			+ " Fps:" + Tools::getText(m_iFPS)
			+ " FN:" + Tools::getText(m_llFrameNumber)
			+ " NT:" + Tools::getText(m_iNetworkType)
			+ " DO:" + Tools::getText(m_iDeviceOrientation)
			+ " QL:" + Tools::getText(m_iVideoQualityLevel)
			+ " NP:" + Tools::getText(m_iNumberOfPacket)
			+ " PN:" + Tools::getText(m_iPacketNumber)
			+ " TM:" + Tools::getText(m_llTimeStamp)
			+ " SIndex:" + Tools::getText(m_iPacketStartingIndex)
			+ " Len:" + Tools::getText(m_iPacketDataLength)
			+ " SDT:" + Tools::getText(m_nSenderDeviceType)
			+ " EF:" + Tools::getText(m_iNumberOfEncodeFailPerFPS)
			+ " LV:" + Tools::getText(m_iMediaEngineVersion)
			+ " LVQL:" + Tools::getText(m_iLiveVideoQualityLevel)
			+ " dFPS:" + Tools::getText(m_iDeviceFPS)
			+ " NOI:" + Tools::getText(m_nNumberOfInset)
			+ " IH0:" + Tools::getText(m_nInsetHeight[0])
			+ " IW0:" + Tools::getText(m_nInsetWidth[0])
			+ " sigma:" + Tools::getText(m_iSigmaValue)
			+ " brightness:" + Tools::getText(m_iBrightnessValue)
            + " LVBR:" + Tools::getText(m_iLiveStreamBitrate)
            + " LVMBR:" + Tools::getText(m_iLiveStreamMaxBitrate)
            + " H:" + Tools::getText(m_iVideoHeightFourth)
            + " W:" + Tools::getText(m_iVideoWidthFourth)
            + " UpperOffset:" + Tools::getText(m_iInsetUppderOffset)
			;

		unsigned char pLocalData[100];

		GetHeaderInByteArray(pLocalData);
		string sss = "#PKT#  -> " + sTag;

		for (int i = 0; i < 22; i++)
		{
			int byteElement = (int)pLocalData[i];

			sss += Tools::getText(byteElement);
			sss += ", ";


		}

		//CLogPrinter_WriteLog(CLogPrinter::INFO, PACKET_DETAILS_LOG, sss);

		//CLogPrinter::Log("%s\n", sLog.c_str());
		CLogPrinter_LOG(PACKET_DETAILS_LOG, "CVideoHeader::ShowDetails %s", sLog.c_str());

	}
	int CVideoHeader::GetHeaderInByteArray(unsigned char* data)
	{
		int index = 0;
		//Packet Type
		data[index++] = (unsigned char)m_iPacketType;

		//Version Code
		data[index++] = (unsigned char)m_iVersionCode;

		//Header Length
		data[index++] = (unsigned char)m_iHeaderLength;

		//FPS Byte
		data[index++] = (unsigned char)m_iFPS;

		//FrameNumber
		data[index++] = (unsigned char)(m_llFrameNumber >> 24);
		data[index++] = (unsigned char)(m_llFrameNumber >> 16);
		data[index++] = (unsigned char)(m_llFrameNumber >> 8);
		data[index++] = (unsigned char)(m_llFrameNumber >> 0);

		//CallInfoByte
		data[index] = 0;
		data[index] |= (m_iNetworkType & 1);    //0th BIT
		data[index] |= (m_iDeviceOrientation & ORIENTATION_BITSET) << 1;  //1,2 BITs
		data[index] |= (m_iVideoQualityLevel & QUALITY_LEVEL_BITSET) << 3;  //3,4 BITs
		index++;

		//Number of Packets
		data[index++] = m_iNumberOfPacket;

		//Packet Number
		data[index++] = m_iPacketNumber;

		//TimeStamp
		data[index++] = (unsigned char)(m_llTimeStamp >> 32);
		data[index++] = (unsigned char)(m_llTimeStamp >> 24);
		data[index++] = (unsigned char)(m_llTimeStamp >> 16);
		data[index++] = (unsigned char)(m_llTimeStamp >> 8);
		data[index++] = (unsigned char)(m_llTimeStamp >> 0);

		//Packet Starting Index
		data[index++] = (m_iPacketStartingIndex >> 16);
		data[index++] = (m_iPacketStartingIndex >> 8);
		data[index++] = (m_iPacketStartingIndex >> 0);

		//Packet Data Length
		data[index++] = (m_iPacketDataLength >> 16);
		data[index++] = (m_iPacketDataLength >> 8);
		data[index++] = (m_iPacketDataLength >> 0);

		//Sender Device Type
		data[index++] = m_nSenderDeviceType;

		//InsetInfo
		data[index++] = m_nNumberOfInset;

		for (int i = 0; i < m_nNumberOfInset; i++)
		{
			data[index++] = m_nInsetHeight[i] >> 8;
			data[index++] = m_nInsetHeight[i] >> 0;
		}
		for (int i = 0; i < m_nNumberOfInset; i++)
		{
			data[index++] = m_nInsetWidth[i] >> 8;
			data[index++] = m_nInsetWidth[i] >> 0;
		}
        
        //More Info
        data[index++] = (unsigned char)m_iSigmaValue;
        data[index++] = (unsigned char)m_iBrightnessValue;
        
        unsigned char temp1, temp2;
        temp1 = (unsigned char)m_iDeviceFPS;
        temp2 = (unsigned char)m_iNumberOfEncodeFailPerFPS;
        
        temp1 = (temp1 << 3);
        temp2 = (temp2 >> 2) & 0x7;
        data[index++] = temp1 | temp2;
        
        
        
        temp1 = (unsigned char)m_iNumberOfEncodeFailPerFPS;
        temp2 = (unsigned char)m_iMediaEngineVersion;
        temp1 = temp1<<6;
        temp2 = temp2 & 0x3F;
        data[index++] = temp1 | temp2;
        
        //LiveVideoQuality
        data[index] = 0;
        data[index] |= (m_iLiveVideoQualityLevel & 0x07);
        index++;
        
        //LiveStreamBitrate
        data[index++] = (m_iLiveStreamBitrate >> 16);
        data[index++] = (m_iLiveStreamBitrate >> 8);
        data[index++] = (m_iLiveStreamBitrate >> 0);
        
        //LiveStreamMaxBitrate
        data[index++] = (m_iLiveStreamMaxBitrate >> 16);
        data[index++] = (m_iLiveStreamMaxBitrate >> 8);
        data[index++] = (m_iLiveStreamMaxBitrate >> 0);
        
        
        //VideoHeightWidthFourth
        data[index++] = (unsigned char)m_iVideoHeightFourth;
        data[index++] = (unsigned char)m_iVideoWidthFourth;
		return index;
	}

	

	void CVideoHeader::SetVersionCode(unsigned char *VersionCodeByte) {
		m_iVersionCode = VersionCodeByte[0];
	}


	void CVideoHeader::SetPacketType(unsigned char *pData)
	{
		m_iPacketType = pData[0];
	}

	void CVideoHeader::SetFrameNumber(unsigned char *pData)
	{
		m_llFrameNumber = GetLongLongFromChar(pData, 0, 4);
	}


	void CVideoHeader::SetNumberOfPacket(unsigned char *pData)
	{
		m_iNumberOfPacket = GetIntFromChar(pData, 0, 1);
	}

	void CVideoHeader::SetPacketNumber(unsigned char *pData)
	{
		m_iPacketNumber = GetIntFromChar(pData, 0, 1);
	}

	long long CVideoHeader::GetTimeStampDirectly(unsigned char *pData)
	{
		return GetIntFromChar(pData + TIMESTAMP_INDEX, 0, 5);
	}


	void CVideoHeader::SetTimeStamp(unsigned char *pData)
	{
		m_llTimeStamp = GetLongLongFromChar(pData, 0, 5);
	}


	void CVideoHeader::SetFPS(unsigned char *pData)
	{
		m_iFPS = GetIntFromChar(pData, 0, 1);
	}


	void CVideoHeader::SetPacketDataLength(unsigned char *pData)
	{
		m_iPacketDataLength = GetIntFromChar(pData, 0, 3);
	}

	void CVideoHeader::SetSenderDeviceType(unsigned char * pData)
	{
		m_nSenderDeviceType = (int)pData[0];
	}


	long long CVideoHeader::GetFrameNumberDirectly(unsigned char *pData)
	{
		return GetLongLongFromChar(pData + FRAME_NUMBER_INDEX, 0, 4);

	}

	int CVideoHeader::GetFrameHeaderLengthDirectly(unsigned char *pData)
	{
		return GetIntFromChar(pData + HEADER_LENGTH_INDEX, 0, 1);

	}

	int CVideoHeader::GetIntFromChar(unsigned char *packetData, int index, int nLenght)
	{
		int result = 0;
		int interval = 8;
		int startPoint = (nLenght - 1) << 3;
		for (int i = startPoint; i >= 0; i -= interval)
		{
			int temp = (int)(packetData[index++] & 0xFF) << i;
			result += temp;
		}

		return result;
	}


	long long CVideoHeader::GetLongLongFromChar(unsigned char *packetData, int index, int nLenght)
	{
		long long result = 0;
		int interval = 8;
		int startPoint = (nLenght - 1) << 3;
		for (int i = startPoint; i >= 0; i -= interval)
		{
			long long temp = (long long)(packetData[index++] & 0xFF) << i;
			result += temp;
		}

		return result;
	}

	int CVideoHeader::GetOpponentResolution(unsigned char *PacketHeader)
	{
		return PacketHeader[CALL_INFO_INDEX] & 0x06;
	}

	void CVideoHeader::SetNetworkType(unsigned char* data){
		m_iNetworkType = (data[0] & 1);
	}

	void CVideoHeader::SetDeviceOrientation(unsigned char *packetData)
	{
		m_iDeviceOrientation = (GetIntFromChar(packetData, 0, 1) >> 1) & ORIENTATION_BITSET;
	}

	void CVideoHeader::SetVideoQualityLevel(unsigned char* data){
		m_iVideoQualityLevel = (GetIntFromChar(data, 0, 1) >> 3) & QUALITY_LEVEL_BITSET;
	}



	void CVideoHeader::SetHeaderLength(unsigned char *pData)
	{
		m_iHeaderLength = pData[0];
	}




	void CVideoHeader::SetPacketStartingIndex(unsigned char *pData)
	{
		m_iPacketStartingIndex = GetIntFromChar(pData, 0, 3);
	}



	//Inset Information
	void CVideoHeader::SetNumberOfInset(unsigned char *pData)
	{
		m_nNumberOfInset = (int)pData[0];
	}
    
	void CVideoHeader::SetInsetHeights(unsigned char *pData, int nNumberOfInsets)
	{

		for (int i = 0; i < nNumberOfInsets; i++)
		{
			int index = i * 2;
			m_nInsetHeight[i] = GetIntFromChar(pData + index, 0, 2);
		}
	}
	void CVideoHeader::GetInsetHeights(int *pHeightValues, int nNumberOfInsets)
	{
		for (int i = 0; i < nNumberOfInsets; i++)
		{
			pHeightValues[i] = m_nInsetHeight[i];
		}
	}


	void CVideoHeader::SetInsetWidths(unsigned char *pData, int nNumberOfInsets)
	{
		for (int i = 0; i < nNumberOfInsets; i++)
		{
			int index = nNumberOfInsets * 2 + (i * 2);
			m_nInsetWidth[i] = GetIntFromChar(pData + index, 0, 2);
		}
	}
	void CVideoHeader::GetInsetWidths(int *pWidthValues, int nNumberOfInsets)
	{
		for (int i = 0; i < nNumberOfInsets; i++)
		{
			pWidthValues[i] = m_nInsetWidth[i];
		}
	}

    //More Information
    void CVideoHeader::SetSigmaValue(unsigned char *pData)
    {
        m_iSigmaValue = (int)pData[0];
    }

    
    void CVideoHeader::SetBrightnessValue(unsigned char *pData)
    {
        m_iBrightnessValue = (int)pData[0];
    }
    
    void CVideoHeader::SetDeviceFPS(unsigned char *pData)
    {
        m_iDeviceFPS = (pData[0] >> 3) & 0x1F;
    }
    
    void CVideoHeader::SetEncodeFailPerFPS(unsigned char *pData)
    {
        int val = GetIntFromChar(pData, 0, 2);
        m_iNumberOfEncodeFailPerFPS = (val >> 6) & 0x1F;
        
    }
    
    void CVideoHeader::SetLibraryVersion(unsigned char *pData)
    {
        m_iMediaEngineVersion = pData[0] & 0x3F;
    }
    
    void CVideoHeader::SetLiveVideoQualityLevel(unsigned char *pData)
    {
        int val = pData[0];
        m_iLiveVideoQualityLevel = val & 0x07;
        
    }
    
    void CVideoHeader::setLiveStreamVideoBitrate(unsigned char *pData)
    {
        m_iLiveStreamBitrate = GetIntFromChar(pData, 0, 3);
    }
    void CVideoHeader::setLiveStreamVideoMaxBitrate(unsigned char *pData)
    {
        m_iLiveStreamMaxBitrate = GetIntFromChar(pData, 0, 3);
    }
    
    void CVideoHeader::setVideoHeightFourth(unsigned char *pData)
    {
        m_iVideoHeightFourth = (int)pData[0];
    }

    
    void CVideoHeader::setVideoWidthFourth(unsigned char *pData)
    {
        m_iVideoWidthFourth = (int)pData[0];
    }
    
    void CVideoHeader::setInsetUpperOffset(unsigned char *pData)
    {
        m_iInsetUppderOffset = GetIntFromChar(pData, 0, 2);
    }

    
    
    
} //namespace MediaSDK


