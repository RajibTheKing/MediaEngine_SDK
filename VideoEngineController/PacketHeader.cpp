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
    m_iPacketLength = 0;
    m_nNetworkType = 0;
    m_nVideoQualityLevel = 0;
    m_iDeviceOrientation = 0;
}
CPacketHeader::~CPacketHeader()
{

}

#define PACKET_TYPE_INDEX 0
#define FPS_INDEX 1
#define FRAME_NUMBER_INDEX 2
#define CALL_INFO_INDEX 5
#define VERSION_CODE_INDEX 6
#define NUMBER_OF_PACKET_INDEX 7
#define PACKET_NUMBER_INDEX 8
#define TIMESTAMP_INDEX 9
#define PACKET_LENGTH_INDEX 13

#define QUALITY_BITS_N      3
#define ORIENTATION_BITS_N  2

#define QUALITY_LEVEL_BITSET    ((1<<QUALITY_BITS_N) - 1)
#define ORIENTATION_BITSET      ((1<<ORIENTATION_BITS_N) - 1)

void CPacketHeader::setPacketHeader(unsigned char *headerData)
{
    setPacketType(headerData + PACKET_TYPE_INDEX);
    setFPS(headerData + FPS_INDEX);
    setFrameNumber(headerData + FRAME_NUMBER_INDEX);
    setVersionCode(headerData + VERSION_CODE_INDEX);
    setNumberOfPacket(headerData + NUMBER_OF_PACKET_INDEX);
    setPacketNumber(headerData + PACKET_NUMBER_INDEX);
    setTimeStamp(headerData + TIMESTAMP_INDEX);
    setPacketLength(headerData + PACKET_LENGTH_INDEX);

    //CallInfoByte
    SetDeviceOrientation(headerData + CALL_INFO_INDEX);
    SetVideoQualityLevel(headerData + CALL_INFO_INDEX);
    SetNetworkType(headerData + CALL_INFO_INDEX);
}



void CPacketHeader::setPacketHeader(unsigned char packetType,
                                    unsigned char uchVersion,
                                    unsigned int FrameNumber,
                                    unsigned int NumberOfPacket,
                                    unsigned int PacketNumber,
                                    unsigned int TimeStamp,
                                    unsigned int FPS,
                                    unsigned int PacketLength,
                                    int nQualityLevel,
                                    int deviceOrientation,
                                    int nNetworkType)
{
    SetPacketType(packetType);
    setVersionCode(uchVersion);
    m_iFrameNumber = FrameNumber;
    setFrameNumber(FrameNumber);
    setNumberOfPacket(NumberOfPacket);
    setPacketNumber(PacketNumber);
    setTimeStamp(TimeStamp);
    setFPS(FPS);
    setPacketLength(PacketLength);

    //CallInfoByte
    SetDeviceOrientation(deviceOrientation);
    SetVideoQualityLevel(nQualityLevel);
    SetNetworkType(nNetworkType);
}

void CPacketHeader::ShowDetails(string sTag){
    VLOG("#PKT#  ->"+sTag+"#  PT: "+Tools::IntegertoStringConvert(m_ucPacketType)
    +"  FN:"+Tools::IntegertoStringConvert(m_iFrameNumber)
    +" NP:"+Tools::IntegertoStringConvert(m_iNumberOfPacket)
    +" PN:"+Tools::IntegertoStringConvert(m_iPacketNumber)
    +" PLen:"+Tools::IntegertoStringConvert(m_iPacketLength)
    +" Ver:"+Tools::IntegertoStringConvert(m_cVersionCode)
    +" TS:"+Tools::IntegertoStringConvert(m_iTimeStamp)
    +" QL:"+Tools::IntegertoStringConvert(m_nVideoQualityLevel)
    +" NT:"+Tools::IntegertoStringConvert(m_nNetworkType)
    +" OR:"+Tools::IntegertoStringConvert(m_iDeviceOrientation)
    );
}
int CPacketHeader::GetHeaderInByteArray(unsigned char* data)
{
    int index = 0;
    data[index++] = m_ucPacketType;
    data[index++] = m_iFPS;

    data[index++] = (m_iFrameNumber >> 16);
    data[index++] = (m_iFrameNumber >> 8);
    data[index++] = m_iFrameNumber;

    //CallInfoByte
    data[index] = 0;
    data[index] |= (m_nNetworkType & 1);    //0th BIT
    data[index] |= (m_iDeviceOrientation & ORIENTATION_BITSET) << 1;  //1,2 BITs
    data[index] |= (m_nVideoQualityLevel & QUALITY_LEVEL_BITSET) << 3;  //3,4 BITs

    index++;

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

void CPacketHeader::SetPacketType(unsigned char packetType){
    m_ucPacketType = packetType;
}

unsigned char CPacketHeader::GetPacketType(){
    return m_ucPacketType;
}

void CPacketHeader::setVersionCode(unsigned char cVersionCode) {
    m_cVersionCode = cVersionCode;
}

void CPacketHeader::setFrameNumber(unsigned int iFrameNumber) {
    m_iFrameNumber = iFrameNumber;
}

void CPacketHeader::setNumberOfPacket(unsigned int iNumberOfPacket) {
    m_iNumberOfPacket = iNumberOfPacket;
}

void CPacketHeader::setPacketNumber(unsigned int iPacketNumber) {
    m_iPacketNumber = iPacketNumber;
}

void CPacketHeader::setTimeStamp(unsigned int iTimeStamp) {
    m_iTimeStamp = iTimeStamp;
}

void CPacketHeader::setFPS(unsigned int iFPS) {
    m_iFPS = iFPS;
}

void CPacketHeader::setPacketLength(int iPacketLength) {
    m_iPacketLength = iPacketLength;
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
void CPacketHeader::setPacketType(unsigned char *pData)
{
    m_ucPacketType = pData[0];
}

void CPacketHeader::setFrameNumber(unsigned char *FrameNumber)
{
    m_iFrameNumber = GetIntFromChar(FrameNumber, 0, 3);
}

unsigned int CPacketHeader::getNumberOfPacket()
{
    return m_iNumberOfPacket;
}

void CPacketHeader::setNumberOfPacket(unsigned char *NumberOfPacket)
{
    m_iNumberOfPacket = GetIntFromChar(NumberOfPacket, 0, 1);
}

unsigned int CPacketHeader::getPacketNumber()
{
    return m_iPacketNumber;
}

void CPacketHeader::setPacketNumber(unsigned char *PacketNumber)
{
    m_iPacketNumber = GetIntFromChar(PacketNumber, 0, 1);
}

int CPacketHeader::getTimeStampDirectly(unsigned char *data)
{
	return GetIntFromChar(data + TIMESTAMP_INDEX, 0, 4);
}

unsigned int CPacketHeader::getTimeStamp()
{
    return m_iTimeStamp;
}

void CPacketHeader::setTimeStamp(unsigned char *TimeStamp)
{
    m_iTimeStamp = GetIntFromChar(TimeStamp, 0, 4);
}

unsigned int CPacketHeader::getFPS()
{
    return m_iFPS;
}

void CPacketHeader::setFPS(unsigned char *FPS)
{
    m_iFPS  = GetIntFromChar(FPS, 0, 1);
}

int CPacketHeader::getPacketLength()
{
    return m_iPacketLength;
}

void CPacketHeader::setPacketLength(unsigned char *PacketLength)
{
    m_iPacketLength = GetIntFromChar(PacketLength, 0, 2);
}

unsigned int CPacketHeader::GetFrameNumberDirectly(unsigned char *packetData)
{
	return GetIntFromChar(packetData + FRAME_NUMBER_INDEX, 0, 3);
    
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

int CPacketHeader::GetOpponentResolution(unsigned char *PacketHeader)
{
    return PacketHeader[CALL_INFO_BYTE_INDEX_WITHOUT_MEDIA] & 0x06;
}

void CPacketHeader::SetNetworkType(unsigned char* data){
    m_nNetworkType = (data[0] & 1);
}

void CPacketHeader::SetDeviceOrientation(unsigned char *packetData)
{
    m_iDeviceOrientation = (GetIntFromChar(packetData, 0, 1) >> 1 ) & ORIENTATION_BITSET;
}

void CPacketHeader::SetVideoQualityLevel(unsigned char* data){
    m_nVideoQualityLevel = (GetIntFromChar(data, 0, 1) >> 3 ) & QUALITY_LEVEL_BITSET;
}

void CPacketHeader::SetNetworkType(int nNetworkType){
    m_nNetworkType = nNetworkType;
}

void CPacketHeader::SetVideoQualityLevel(int nQualityLevel){
    m_nVideoQualityLevel = nQualityLevel;
}

void CPacketHeader::SetDeviceOrientation(int deviceOrientation)
{
    m_iDeviceOrientation = deviceOrientation;
}

int CPacketHeader::GetDeviceOrientation() {
    return m_iDeviceOrientation;
}

int CPacketHeader::GetVideoQualityLevel(){
    return m_nVideoQualityLevel;
}

int CPacketHeader::GetNetworkType(){
    return m_nNetworkType;
}