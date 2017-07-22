
#ifndef IPV_VIDEO_HEADER_H
#define IPV_VIDEO_HEADER_H

#define VLOG(a)     //CLogPrinter_WriteSpecific6(CLogPrinter::INFO,a);
#include <iostream>
#include <string>
#include <bitset>

#define VIDEO_HEADER_LENGTH 32

namespace MediaSDK
{
	using namespace std;

	class CVideoHeader {
	public:
		CVideoHeader();
		~CVideoHeader();

		void setPacketHeader(unsigned char *headerData);

        void setPacketHeader(unsigned char packetType,
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
                             int iMediaEngineVersion
			);
        
        int GetHeaderInByteArray(unsigned char* data);
        
        int GetIntFromChar(unsigned char *packetData, int index, int nLenght);
        long long GetLongLongFromChar(unsigned char *packetData, int index, int nLenght);
        
        long long getTimeStampDirectly(unsigned char *data);
        long long GetFrameNumberDirectly(unsigned char *packetData);
        int GetFrameHeaderLengthDirectly(unsigned char *pData);
        int GetOpponentResolution(unsigned char *PacketHeader);

		void ShowDetails(string sTag);

        void setPacketType(unsigned char *pData);
        unsigned char GetPacketType();

        void setVersionCode(unsigned char * VersionCode);
		unsigned char getVersionCode();
        
        void setHeaderLength(unsigned char *pData);
        int GetHeaderLength();
        
        void setFPS(unsigned char * FPS);
        unsigned int getFPS();
        
        void setFrameNumber(unsigned char * FrameNumber);
		long long getFrameNumber();
        
        void SetNetworkType(unsigned char* data);
        int GetNetworkType();
        
        void SetDeviceOrientation(unsigned char *packetData);
        int GetDeviceOrientation();
        
        void SetVideoQualityLevel(unsigned char* data);
        int GetVideoQualityLevel();

        void setNumberOfPacket(unsigned char * NumberOfPacket);
		unsigned int getNumberOfPacket();
        
        void setPacketNumber(unsigned char * PacketNumber);
		unsigned int getPacketNumber();

        void setTimeStamp(unsigned char * TimeStamp);
		long long getTimeStamp();

        void setPacketStartingIndex(unsigned char *pData);
        int GetPacketStartingIndex();

        void setPacketDataLength(unsigned char * PacketDataLength);
		int getPacketLength();

		void setSenderDeviceType(unsigned char * PacketDataLength);
		int getSenderDeviceType();


		//Inset Information
		void setNumberOfInset(unsigned char *pData);
		int GetNumberOfInset();
        
		void setInsetHeights(unsigned char *pData, int nNumberOfInsets);
		void GetInsetHeights(int *pHeightValues, int nNumberOfInsets);

		void setInsetWidths(unsigned char *pData, int nNumberOfInsets);
		void GetInsetWidths(int *pHeightValues, int nNumberOfInsets);
        
        //More Information
        void setSigmaValue(unsigned char *pData);
        int getSigmaValue();
        
        void setBrightnessValue(unsigned char *pData);
        int getBrightnessValue();
        
        void setDeviceFPS(unsigned char *pData);
        int getDeviceFPS();
        
        void setEncodeFailPerFPS(unsigned char *pData);
        int getEncodeFailPerFPS();
        
        void setLibraryVersion(unsigned char *pData);
        int getLibraryVersion();
        
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
        

        
	};

} //namespace MediaSDK

#endif //VIDEO_HEADER_H
