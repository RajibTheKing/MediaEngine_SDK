
#ifndef IPV_PACKET_HEADER_H
#define IPV_PACKET_HEADER_H

#include "Tools.h"
#include "Size.h"
#include "LogPrinter.h"

#define VLOG(a)     //CLogPrinter_WriteSpecific6(CLogPrinter::INFO,a);

namespace MediaSDK
{

	class CPacketHeader {
	public:
		CPacketHeader();
		~CPacketHeader();

		void setPacketHeader(unsigned char *headerData);
		void setPacketHeader(unsigned char packetType, unsigned char uchVersion, unsigned int FrameNumber, unsigned int NumberOfPacket, unsigned int PacketNumber, unsigned int TimeStamp, unsigned int FPS, unsigned int PacketLength, int nQualityLevel, int deviceOrientation, int nNetworkType);
		void ShowDetails(string sTag);
		void SetPacketType(unsigned char packetType);
		unsigned char GetPacketType();
		void setPacketType(unsigned char *pData);

		int GetHeaderInByteArray(unsigned char* data);
		//hello
		void setVersionCode(unsigned char VersionCode);

		void setFrameNumber(unsigned int iFrameNumber);

		void setNumberOfPacket(unsigned int iNumberOfPacket);

		void setPacketNumber(unsigned int iPacketNumber);

		int getTimeStampDirectly(unsigned char *data);

		void setTimeStamp(unsigned int iTimeStamp);

		void setFPS(unsigned int iFPS);

		void setPacketLength(int iPacketLength);

		//hello1

		unsigned char getVersionCode();

		void setVersionCode(unsigned char * VersionCode);

		unsigned int getFrameNumber();

		void setFrameNumber(unsigned char * FrameNumber);

		unsigned int getNumberOfPacket();

		void setNumberOfPacket(unsigned char * NumberOfPacket);

		unsigned int getPacketNumber();

		void setPacketNumber(unsigned char * PacketNumber);

		unsigned int getTimeStamp();

		void setTimeStamp(unsigned char * TimeStamp);

		unsigned int getFPS();

		void setFPS(unsigned char * FPS);

		int getPacketLength();

		void setPacketLength(unsigned char * PacketLength);

		int GetIntFromChar(unsigned char *packetData, int index, int nLength);

		unsigned int GetFrameNumberDirectly(unsigned char *packetData);

		int GetOpponentResolution(unsigned char *PacketHeader);

		int GetVideoQualityLevel();
		void SetVideoQualityLevel(int nQualityLevel);
		void SetVideoQualityLevel(unsigned char* data);

		int GetNetworkType();
		void SetNetworkType(int nNetworkType);
		void SetNetworkType(unsigned char* data);

		void SetDeviceOrientation(int deviceOrientation);
		void SetDeviceOrientation(unsigned char *packetData);
		int GetDeviceOrientation();

		//hello

	private:
		unsigned char m_ucPacketType;
		unsigned char m_cVersionCode;    // 1 byte
		unsigned int m_iFrameNumber;     // 3 byte
		unsigned int m_iNumberOfPacket;  // 1 byte
		unsigned int m_iPacketNumber;    // 1 byte
		unsigned int m_iTimeStamp;       // 4 byte
		unsigned int m_iFPS;             // 1 byte
		int m_iPacketLength;             // 2 byte
		int m_iDeviceOrientation;             // 2 bit in retransmission signal
		int m_nVideoQualityLevel;
		int m_nNetworkType;
		//Total: 15 byte

	};

} //namespace MediaSDK

#endif //ANDROIDTESTCLIENTVE_FTEST_PACKETHEADER_H
