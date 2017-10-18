
#ifndef IPV_VIDEO_HEADER_H
#define IPV_VIDEO_HEADER_H

#define VLOG(a)     //CLogPrinter_WriteSpecific6(CLogPrinter::INFO,a);
#include <iostream>
#include <string>
#include <bitset>

#define VIDEO_HEADER_LENGTH 39
#define LIBRARY_VERSION 5

namespace MediaSDK
{
	using namespace std;

	class CVideoHeader {
	public:
		CVideoHeader();
		~CVideoHeader();

		void SetPacketHeader(unsigned char *headerData);

        void SetPacketHeader(unsigned char packetType,
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
                             int iLiveStreamMaxBitrate
			);
        
        int GetHeaderInByteArray(unsigned char* data);
        
        int GetIntFromChar(unsigned char *packetData, int index, int nLenght);
        long long GetLongLongFromChar(unsigned char *packetData, int index, int nLenght);
        
        long long GetTimeStampDirectly(unsigned char *data);
        long long GetFrameNumberDirectly(unsigned char *packetData);
        int GetFrameHeaderLengthDirectly(unsigned char *pData);
        int GetOpponentResolution(unsigned char *PacketHeader);

		void ShowDetails(string sTag);

        void SetPacketType(unsigned char *pData);
        unsigned char GetPacketType()
        {
            return (unsigned char)m_iPacketType;
        }

        void SetVersionCode(unsigned char * VersionCode);
		unsigned char GetVersionCode()
        {
            return (unsigned char)m_iVersionCode;
        }
        
        void SetHeaderLength(unsigned char *pData);
        int GetHeaderLength()
        {
            return m_iHeaderLength;
        }
        
        void SetFPS(unsigned char * FPS);
        unsigned int GetFPS()
        {
            return m_iFPS;
        }
        
        void SetFrameNumber(unsigned char * FrameNumber);
		long long GetFrameNumber()
        {
            return m_llFrameNumber;
        }
        
        void SetNetworkType(unsigned char* data);
        int GetNetworkType()
        {
            return m_iNetworkType;
        }
        
        void SetDeviceOrientation(unsigned char *packetData);
        int GetDeviceOrientation()
        {
            return m_iDeviceOrientation;
        }
        
        void SetVideoQualityLevel(unsigned char* data);
        int GetVideoQualityLevel()
        {
            return m_iVideoQualityLevel;
        }

        void SetNumberOfPacket(unsigned char * NumberOfPacket);
		unsigned int GetNumberOfPacket()
        {
            return m_iNumberOfPacket;
        }
        
        void SetPacketNumber(unsigned char * PacketNumber);
		unsigned int GetPacketNumber()
        {
            return m_iPacketNumber;
        }

        void SetTimeStamp(unsigned char * TimeStamp);
		long long GetTimeStamp()
        {
            return m_llTimeStamp;
        }

        void SetPacketStartingIndex(unsigned char *pData);
        int GetPacketStartingIndex()
        {
            return m_iPacketStartingIndex;
        }

        void SetPacketDataLength(unsigned char * PacketDataLength);
		int GetPacketLength()
        {
            return m_iPacketDataLength;
        }

		void SetSenderDeviceType(unsigned char * PacketDataLength);
		int GetSenderDeviceType()
        {
            return m_nSenderDeviceType;
        }


		//Inset Information
		void SetNumberOfInset(unsigned char *pData);
		int GetNumberOfInset()
        {
            return m_nNumberOfInset;
        }
        
		void SetInsetHeights(unsigned char *pData, int nNumberOfInsets);
		void GetInsetHeights(int *pHeightValues, int nNumberOfInsets);

		void SetInsetWidths(unsigned char *pData, int nNumberOfInsets);
		void GetInsetWidths(int *pHeightValues, int nNumberOfInsets);
        
        //More Information
        void SetSigmaValue(unsigned char *pData);
        int GetSigmaValue()
        {
            return m_iSigmaValue;
        }
        
        void SetBrightnessValue(unsigned char *pData);
        int GetBrightnessValue()
        {
            return m_iBrightnessValue;
        }
        
        void SetDeviceFPS(unsigned char *pData);
        int GetDeviceFPS()
        {
            return m_iDeviceFPS;
        }
        
        void SetEncodeFailPerFPS(unsigned char *pData);
        int GetEncodeFailPerFPS()
        {
            return m_iNumberOfEncodeFailPerFPS;
        }
        
        void SetLibraryVersion(unsigned char *pData);
        int GetLibraryVersion()
        {
            return m_iMediaEngineVersion;
        }
        
        void SetLiveVideoQualityLevel(unsigned char *pData);
        int GetLiveVideoQualityLevel()
        {
            return m_iLiveVideoQualityLevel;
        }
        
        void setLiveStreamVideoBitrate(unsigned char *pData);
        int GetLiveStreamVideoBitrate()
        {
            return m_iLiveStreamBitrate;
        }
        
        void setLiveStreamVideoMaxBitrate(unsigned char *pData);
        int GetLiveStreamVideoMaxBitrate()
        {
            return m_iLiveStreamMaxBitrate;
        }
        
	private:

		//unsigned char m_ucPacketType;
		//unsigned char m_cVersionCode;    // 1 byte
		//unsigned int m_iFrameNumber;     // 3 byte
		//unsigned int m_iNumberOfPacket;  // 1 byte
		//unsigned int m_iPacketNumber;    // 1 byte
		//unsigned int m_iTimeStamp;       // 4 byte
		//unsigned int m_iFPS;             // 1 byte
		//int m_iPacketLength;             // 2 byte
		//int m_iDeviceOrientation;             // 2 bit in retransmission signal
		//int m_nVideoQualityLevel;
		//int m_nNetworkType;
		//Total: 15 byte

		int m_iPacketType = 0; //1 byte
		int m_iVersionCode = 0;    // 1 byte
		int m_iHeaderLength = 0;    //1 byte

		int m_iFPS = 0;             // 1 byte
		int m_iOpponentFPS = 0;
		int m_iFPSForceBit = 0;
		int m_iFPSChange = 0;

		long long m_llFrameNumber = 0;     // 4 byte

		int m_iCallInfo = 0;        //1 byte
		int m_iNetworkType = 0;
		int m_iVideoQualityLevel = 0;
		int m_iDeviceOrientation = 0;


		int m_iNumberOfPacket = 0;  // 1 byte
		int m_iPacketNumber = 0;    // 1 byte

		long long m_llTimeStamp = 0;       // 5 byte

		int m_iPacketStartingIndex = 0; //3 byte
		int m_iPacketDataLength = 0;    //3 byte
		//Total: 22 byte

		int m_nSenderDeviceType = 0;	// 1 byte

		//Total: 23 byte

		int m_nNumberOfInset = 0; // 1 byte
		int m_nInsetHeight[3];    // 2 byte Per Height
		int m_nInsetWidth[3];    // 2 byte Per Width

		//Total: 23 byte + 1 byte + m_NumberOfInset * 4 byte. [Current Situation]
		//Total: (23 + 1 + 1 * 4) = 28 byte
        
        
        
        int m_iSigmaValue;  //1 byte
        int m_iBrightnessValue; //1 byte
        
        int m_iDeviceFPS; //5 bit
        int m_iNumberOfEncodeFailPerFPS; //5 bit
        int m_iMediaEngineVersion; //6 bit
        
        //Total: (23 + 1 + 1 * 4) = 32 byte
        
        int m_iLiveVideoQualityLevel; //3 bit
        //Total 32 + 1 = 33 byte
        
        int m_iLiveStreamBitrate; //3 byte
        int m_iLiveStreamMaxBitrate; //3 byte
        //Total 33 + 6 = 39 byte
        

        
	};

} //namespace MediaSDK

#endif //VIDEO_HEADER_H
