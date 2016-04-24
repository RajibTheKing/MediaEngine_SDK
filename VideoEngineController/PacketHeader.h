#ifndef ANDROIDTESTCLIENTVE_FTEST_PACKETHEADER_H
#define ANDROIDTESTCLIENTVE_FTEST_PACKETHEADER_H
#include "Tools.h"
#include "Size.h"

class CPacketHeader {
public:
    CPacketHeader();
    ~CPacketHeader();

    void setPacketHeader(unsigned char *headerData);
    void setPacketHeader(unsigned char uchVersion, unsigned int FrameNumber, unsigned int NumberOfPacket, unsigned int PacketNumber,unsigned int TimeStamp, unsigned int FPS, unsigned int RetransSignal, unsigned int PacketLength, int deviceOrientation = 0);

    int GetHeaderInByteArray(unsigned char* data);
    //hello
    void setVersionCode(unsigned char VersionCode) ;

    void setFrameNumber(unsigned int m_iFrameNumber);

    void setNumberOfPacket(unsigned int m_iNumberOfPacket);

    void setPacketNumber(unsigned int m_iPacketNumber);

    void setTimeStamp(unsigned int m_iTimeStamp) ;

    void setFPS(unsigned int m_iFPS) ;

    void setRetransSignal(unsigned int m_iRetransSignal) ;

    void setPacketLength(int m_iPacketLength) ;

    //hello1

    unsigned char getVersionCode();

    void setVersionCode(unsigned char * VersionCode) ;

    unsigned int getFrameNumber() ;

    void setFrameNumber(unsigned char * FrameNumber);

    unsigned int getNumberOfPacket() ;

    void setNumberOfPacket(unsigned char * NumberOfPacket) ;

    unsigned int getPacketNumber() ;

    void setPacketNumber(unsigned char * PacketNumber);

    unsigned int getTimeStamp();

    void setTimeStamp(unsigned char * TimeStamp);

    unsigned int getFPS();

    void setFPS(unsigned char * FPS);

    unsigned int getRetransSignal();

    void setRetransSignal(unsigned char * RetransSignal);

    int getPacketLength() ;

    void setPacketLength(unsigned char * PacketLength) ;

    int GetIntFromChar(unsigned char *packetData, int index,int nLenght);

	unsigned int GetFrameNumberDirectly(unsigned char *packetData);

    int SetDeviceOrientation(int deviceOrientation);
    int SetDeviceOrientation(unsigned char *packetData);

    int GetDeviceOrientation();

    //hello

private:
    unsigned char m_cVersionCode;    // 1 byte
    unsigned int m_iFrameNumber;     // 3 byte
    unsigned int m_iNumberOfPacket;  // 1 byte
    unsigned int m_iPacketNumber;    // 1 byte
    unsigned int m_iTimeStamp;       // 4 byte
    unsigned int m_iFPS;             // 1 byte
    unsigned int m_iRetransSignal;   // 1 byte
    int m_iPacketLength;             // 2 byte
    int m_iDeviceOrientation;             // 2 bit in retransmission signal
    //Total: 14 byte

};

#endif //ANDROIDTESTCLIENTVE_FTEST_PACKETHEADER_H
