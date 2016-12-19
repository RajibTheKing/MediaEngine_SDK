#ifndef ANDROIDTESTCLIENTVE_FTEST_PACKETHEADER_H
#define ANDROIDTESTCLIENTVE_FTEST_PACKETHEADER_H

#define VLOG(a)     CLogPrinter_WriteSpecific6(CLogPrinter::INFO,a);
#include <iostream>
#include <string>
using namespace std;

class CVideoHeader {
public:
    CVideoHeader();
    ~CVideoHeader();

    void setPacketHeader(unsigned char *headerData);
    
	void setPacketHeader(	unsigned char packetType,
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
							unsigned int PacketLength
							);
    
    void ShowDetails(string sTag);
    
    
    void SetPacketType(unsigned char packetType);
    unsigned char GetPacketType();
    void setPacketType(unsigned char *pData);

    int GetHeaderInByteArray(unsigned char* data);
    //hello
    void setVersionCode(unsigned char VersionCode) ;

	void setFrameNumber(long long llFrameNumber);

    void setNumberOfPacket(unsigned int iNumberOfPacket);

    void setPacketNumber(unsigned int iPacketNumber);

	int getTimeStampDirectly(unsigned char *data);

	void setTimeStamp(long long llTimeStamp);

    void setFPS(unsigned int iFPS) ;

    void setPacketDataLength(int iPacketDataLength) ;
    

    //hello1

    unsigned char getVersionCode();

    void setVersionCode(unsigned char * VersionCode) ;

    long long getFrameNumber() ;

    void setFrameNumber(unsigned char * FrameNumber);

    unsigned int getNumberOfPacket() ;

    void setNumberOfPacket(unsigned char * NumberOfPacket) ;

    unsigned int getPacketNumber() ;

    void setPacketNumber(unsigned char * PacketNumber);

    long long getTimeStamp();

    void setTimeStamp(unsigned char * TimeStamp);

    unsigned int getFPS();

    void setFPS(unsigned char * FPS);

    int getPacketLength() ;

    void setPacketDataLength(unsigned char * PacketDataLength) ;

    int GetIntFromChar(unsigned char *packetData, int index,int nLenght);
	long long GetLongLongFromChar(unsigned char *packetData, int index, int nLenght);

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
    
    
    void setHeaderLength(int iHeaderLength);
    void setHeaderLength(unsigned char *pData);
    int GetHeaderLength();
    
    void setPacketStartingIndex(int iPacketStartingIndex);
    void setPacketStartingIndex(unsigned char *pData);
    int GetPacketStartingIndex();

    //hello

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
    

};

#endif //ANDROIDTESTCLIENTVE_FTEST_PACKETHEADER_H
