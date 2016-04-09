//
// Created by ipvision on 1/23/2016.
//

#include "PacketHeader.h"
#include "LogPrinter.h"

CPacketHeader::CPacketHeader()
{
    m_cVersionCode = 0;    // 1 byte
    m_iFrameNumber = 0;     // 3 byte
    m_iNumberOfPacket = 0;  // 1 byte
    m_iPacketNumber = 0;    // 1 byte
    m_iTimeStamp = 0;       // 4 byte
    m_iFPS = 0;             // 1 byte
    m_iRetransSignal = 0;   // 1 byte
    m_iPacketLength = 0;
}
CPacketHeader::~CPacketHeader()
{

}

void CPacketHeader::setPacketHeader(unsigned char *headerData)
{
    if(headerData[VERSION_BYTE_INDEX]) {        //Version > 0
        setFPS(headerData);
        setFrameNumber(headerData + 1);
        setRetransSignal(headerData + 4);
        setVersionCode(headerData + 5);
        setNumberOfPacket(headerData + 6);
        setPacketNumber(headerData + 7);
        setTimeStamp(headerData + 8);
        setPacketLength(headerData + 12);
    }
    else
    {
        setFPS(headerData);
        setFrameNumber(headerData + 1);
        setRetransSignal(headerData + 4);
        setVersionCode(headerData + 5);
        setNumberOfPacket(headerData + 7);
        setPacketNumber(headerData + 11);
        setPacketLength(headerData + 14);
        setTimeStamp(headerData + 16);
    }
}

void CPacketHeader::setPacketHeader(unsigned char uchVersion, unsigned int FrameNumber, unsigned int NumberOfPacket, unsigned int PacketNumber,
                             unsigned int TimeStamp, unsigned int FPS, unsigned int RetransSignal, unsigned int PacketLength)
{
    m_cVersionCode = uchVersion;
    setFrameNumber(FrameNumber);
    setNumberOfPacket(NumberOfPacket);
    setPacketNumber(PacketNumber);
    setTimeStamp(TimeStamp);
    setFPS(FPS);
    setRetransSignal(RetransSignal);
    setPacketLength(PacketLength);
}

int CPacketHeader::GetHeaderInByteArray(unsigned char* data)
{
    int index = 0;

    if(m_cVersionCode) {
        data[index++] = m_iFPS;

        data[index++] = (m_iFrameNumber >> 16);
        data[index++] = (m_iFrameNumber >> 8);
        data[index++] = m_iFrameNumber;

        data[index++] = m_iRetransSignal;

        data[index++] = m_cVersionCode;

        data[index++] = m_iNumberOfPacket;
        data[index++] = m_iPacketNumber;

        data[index++] = (m_iTimeStamp >> 24);
        data[index++] = (m_iTimeStamp >> 16);
        data[index++] = (m_iTimeStamp >> 8);
        data[index++] = m_iTimeStamp;

        data[index++] = (m_iPacketLength >> 8);
        data[index++] = m_iPacketLength;
        return PACKET_HEADER_LENGTH;
    }
    else
    {
        memset(data, 0, PACKET_HEADER_LENGTH_NO_VERSION);

        index = 0;
        data[index] = m_iFPS;

        index = 1;
        data[index++] = (m_iFrameNumber >> 16);
        data[index++] = (m_iFrameNumber >> 8);
        data[index++] = m_iFrameNumber;

        index = 4;
        data[index] = m_iRetransSignal;

        index = 5;
//        data[index] = m_cVersionCode;

        index = 7;
        data[index] = m_iNumberOfPacket;

        index = 11;
        data[index] = m_iPacketNumber;

        index = 14;
        data[index++] = (m_iPacketLength >> 8);
        data[index++] = m_iPacketLength;

        index = 16;
        data[index++] = (m_iTimeStamp >> 24);
        data[index++] = (m_iTimeStamp >> 16);
        data[index++] = (m_iTimeStamp >> 8);
        data[index++] = m_iTimeStamp;
        return PACKET_HEADER_LENGTH_NO_VERSION;
    }
    return 1;
}


void CPacketHeader::setVersionCode(unsigned char m_cVersionCode) {
    CPacketHeader::m_cVersionCode = m_cVersionCode;
}

void CPacketHeader::setFrameNumber(unsigned int m_iFrameNumber) {
    CPacketHeader::m_iFrameNumber = m_iFrameNumber;
}

void CPacketHeader::setNumberOfPacket(unsigned int m_iNumberOfPacket) {
    CPacketHeader::m_iNumberOfPacket = m_iNumberOfPacket;
}

void CPacketHeader::setPacketNumber(unsigned int m_iPacketNumber) {
    CPacketHeader::m_iPacketNumber = m_iPacketNumber;
}

void CPacketHeader::setTimeStamp(unsigned int m_iTimeStamp) {
    CPacketHeader::m_iTimeStamp = m_iTimeStamp;
}

void CPacketHeader::setFPS(unsigned int m_iFPS) {
    CPacketHeader::m_iFPS = m_iFPS;
}

void CPacketHeader::setRetransSignal(unsigned int m_iRetransSignal) {
    CPacketHeader::m_iRetransSignal = m_iRetransSignal;
}

void CPacketHeader::setPacketLength(int m_iPacketLength) {
    CPacketHeader::m_iPacketLength = m_iPacketLength;
}

unsigned char CPacketHeader::getVersionCode() {
    return m_cVersionCode;
}

void CPacketHeader::setVersionCode(unsigned char *VersionCodeByte) {
    m_cVersionCode = VersionCodeByte[0];
}

unsigned int CPacketHeader::getFrameNumber()
{
    return m_iFrameNumber;
}

void CPacketHeader::setFrameNumber(unsigned char *FrameNumber)
{
    CPacketHeader::m_iFrameNumber = GetIntFromChar(FrameNumber, 0, 3);
}

unsigned int CPacketHeader::getNumberOfPacket()
{
    return m_iNumberOfPacket;
}

void CPacketHeader::setNumberOfPacket(unsigned char *NumberOfPacket)
{
    CPacketHeader::m_iNumberOfPacket = GetIntFromChar(NumberOfPacket, 0, 1);
}

unsigned int CPacketHeader::getPacketNumber()
{
    return m_iPacketNumber;
}

void CPacketHeader::setPacketNumber(unsigned char *PacketNumber)
{
    CPacketHeader::m_iPacketNumber = GetIntFromChar(PacketNumber, 0, 1);
}

unsigned int CPacketHeader::getTimeStamp()
{
    return m_iTimeStamp;
}

void CPacketHeader::setTimeStamp(unsigned char *TimeStamp)
{
    CPacketHeader::m_iTimeStamp = GetIntFromChar(TimeStamp, 0, 4);
}

unsigned int CPacketHeader::getFPS()
{
    return m_iFPS;
}

void CPacketHeader::setFPS(unsigned char *FPS)
{
    CPacketHeader::m_iFPS  = GetIntFromChar(FPS, 0, 1);
}

unsigned int CPacketHeader::getRetransSignal()
{
    return m_iRetransSignal;
}

void CPacketHeader::setRetransSignal(unsigned char *RetransSignal)
{
    CPacketHeader::m_iRetransSignal = GetIntFromChar(RetransSignal, 0, 1);
}

int CPacketHeader::getPacketLength()
{
    return m_iPacketLength;
}

void CPacketHeader::setPacketLength(unsigned char *PacketLength)
{
    CPacketHeader::m_iPacketLength = GetIntFromChar(PacketLength, 0, 2);
}

unsigned int CPacketHeader::GetFrameNumberDirectly(unsigned char *packetData)
{
	return GetIntFromChar(packetData + 1, 0, 3);
    
}

int CPacketHeader::GetIntFromChar(unsigned char *packetData, int index, int nLenght)
{
    int result = 0;
    int interval = 8;
    int startPoint = (nLenght - 1) << 3;
    for(int i=startPoint; i >= 0 ; i-=interval)
    {
        result += (packetData[index++] & 0xFF) << i;
    }

    return result;
}

void CPacketHeader::SetResolutionBit(unsigned char *PacketHeader, int value)
{
    PacketHeader[RETRANSMISSION_SIG_BYTE_INDEX_WITHOUT_MEDIA] |= value;
}
int CPacketHeader::GetOpponentResolution(unsigned char *PacketHeader)
{
    
    return PacketHeader[RETRANSMISSION_SIG_BYTE_INDEX_WITHOUT_MEDIA] & 0x06;
}

void CPacketHeader::SetNetworkTypeBit(unsigned char *PacketHeader , int value)
{
    PacketHeader[RETRANSMISSION_SIG_BYTE_INDEX_WITHOUT_MEDIA] |= value;
}
int CPacketHeader::GetOpponentNetworkType(unsigned char *PacketHeader)
{
    return PacketHeader[RETRANSMISSION_SIG_BYTE_INDEX_WITHOUT_MEDIA] & 0x01;
}



