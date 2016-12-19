//
// Created by ipvision on 1/23/2016.
//

#include "VideoHeader.h"
//#include "LogPrinter.h"


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



#define QUALITY_BITS_N      3
#define ORIENTATION_BITS_N  2

#define QUALITY_LEVEL_BITSET    ((1<<QUALITY_BITS_N) - 1)
#define ORIENTATION_BITSET      ((1<<ORIENTATION_BITS_N) - 1)


CVideoHeader::CVideoHeader()
{
    m_iPacketType = 0; //1 byte
    m_iVersionCode = 0;    // 1 byte
    m_iHeaderLength = 0;    //1 byte
    
    m_iFPS = 0;             // 1 byte
        m_iOpponentFPS = 0;
        m_iFPSForceBit = 0;
        m_iFPSChange = 0;
    
    m_llFrameNumber = 0;     // 4 byte
    
    m_iCallInfo = 0;        //1 byte
        m_iNetworkType = 0;
        m_iVideoQualityLevel = 0;
        m_iDeviceOrientation = 0;
    
    
    m_iNumberOfPacket = 0;  // 1 byte
    m_iPacketNumber = 0;    // 1 byte
    
    m_llTimeStamp = 0;       // 5 byte

    m_iPacketStartingIndex = 0; //3 byte
    m_iPacketDataLength = 0;    //3 byte
    
    
    
}
CVideoHeader::~CVideoHeader()
{

}

void CVideoHeader::setPacketHeader(unsigned char *headerData)
{
    setPacketType(headerData + PACKET_TYPE_INDEX);
    setVersionCode(headerData + VERSION_CODE_INDEX);
    setHeaderLength(headerData + HEADER_LENGTH_INDEX);
    setFPS(headerData + FPS_INDEX);
    setFrameNumber(headerData + FRAME_NUMBER_INDEX);
    
    //CallInfoByte
    SetDeviceOrientation(headerData + CALL_INFO_INDEX);
    SetVideoQualityLevel(headerData + CALL_INFO_INDEX);
    SetNetworkType(headerData + CALL_INFO_INDEX);
    
    setNumberOfPacket(headerData + NUMBER_OF_PACKET_INDEX);
    setPacketNumber(headerData + PACKET_NUMBER_INDEX);
    setTimeStamp(headerData + TIMESTAMP_INDEX);
    
    setPacketStartingIndex(headerData + PACKET_STARTING_INDEX);
    setPacketDataLength(headerData + PACKET_DATA_LENGTH_INDEX);

    
}



void CVideoHeader::setPacketHeader(unsigned char packetType,
                                    unsigned char uchVersion,
                                    unsigned int iHeaderLength,
                                    unsigned int iFPSbyte,
                                    long long llFrameNumber,
                                    int nNetworkType,
                                    int deviceOrientation,
                                    int nQualityLevel,
                                    unsigned int NumberOfPacket,
                                    unsigned int PacketNumber,
                                    long long llTimeStamp,
                                    unsigned int iPacketStartingIndex,
                                    unsigned int PacketLength
                                    )
{
    SetPacketType(packetType);
    setVersionCode(uchVersion);
    m_llFrameNumber = FrameNumber;
    setFrameNumber(FrameNumber);
    setNumberOfPacket(NumberOfPacket);
    setPacketNumber(PacketNumber);
    setTimeStamp(TimeStamp);
    setFPS(FPS);
    setPacketDataLength(PacketLength);

    //CallInfoByte
    SetDeviceOrientation(deviceOrientation);
    SetVideoQualityLevel(nQualityLevel);
    SetNetworkType(nNetworkType);
}

void CVideoHeader::ShowDetails(string sTag){
    VLOG("#PKT#  ->"+sTag+"#  PT: "+Tools::IntegertoStringConvert(m_ucPacketType)
    +"  FN:"+Tools::IntegertoStringConvert(m_iFrameNumber)
    +" NP:"+Tools::IntegertoStringConvert(m_iNumberOfPacket)
    +" PN:"+Tools::IntegertoStringConvert(m_iPacketNumber)
    +" PLen:"+Tools::IntegertoStringConvert(m_iPacketDataLength)
    +" Ver:"+Tools::IntegertoStringConvert(m_cVersionCode)
    +" TS:"+Tools::IntegertoStringConvert(m_iTimeStamp)
    +" QL:"+Tools::IntegertoStringConvert(m_nVideoQualityLevel)
    +" NT:"+Tools::IntegertoStringConvert(m_nNetworkType)
    +" OR:"+Tools::IntegertoStringConvert(m_iDeviceOrientation)
    );
}
int CVideoHeader::GetHeaderInByteArray(unsigned char* data)
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

    data[index++] = (m_iPacketDataLength >> 8);
    data[index++] = m_iPacketDataLength;
    return PACKET_HEADER_LENGTH;
}

void CVideoHeader::SetPacketType(unsigned char packetType)
{
    m_ucPacketType = packetType;
}

unsigned char CVideoHeader::GetPacketType(){
    return m_ucPacketType;
}

void CVideoHeader::setVersionCode(unsigned char cVersionCode) {
    m_cVersionCode = cVersionCode;
}

void CVideoHeader::setFrameNumber(unsigned int iFrameNumber) {
    m_iFrameNumber = iFrameNumber;
}

void CVideoHeader::setNumberOfPacket(unsigned int iNumberOfPacket) {
    m_iNumberOfPacket = iNumberOfPacket;
}

void CVideoHeader::setPacketNumber(unsigned int iPacketNumber) {
    m_iPacketNumber = iPacketNumber;
}

void CVideoHeader::setTimeStamp(unsigned int iTimeStamp) {
    m_iTimeStamp = iTimeStamp;
}

void CVideoHeader::setFPS(unsigned int iFPS) {
    m_iFPS = iFPS;
}

void CVideoHeader::setPacketDataLength(int iPacketDataLength) {
    m_iPacketDataLength = iPacketDataLength;
}

unsigned char CVideoHeader::getVersionCode() {
    return m_cVersionCode;
}

void CVideoHeader::setVersionCode(unsigned char *VersionCodeByte) {
    m_cVersionCode = VersionCodeByte[0];
}

unsigned int CVideoHeader::getFrameNumber()
{
    return m_iFrameNumber;
}
void CVideoHeader::setPacketType(unsigned char *pData)
{
    m_ucPacketType = pData[0];
}

void CVideoHeader::setFrameNumber(unsigned char *FrameNumber)
{
    m_iFrameNumber = GetIntFromChar(FrameNumber, 0, 3);
}

unsigned int CVideoHeader::getNumberOfPacket()
{
    return m_iNumberOfPacket;
}

void CVideoHeader::setNumberOfPacket(unsigned char *NumberOfPacket)
{
    m_iNumberOfPacket = GetIntFromChar(NumberOfPacket, 0, 1);
}

unsigned int CVideoHeader::getPacketNumber()
{
    return m_iPacketNumber;
}

void CVideoHeader::setPacketNumber(unsigned char *PacketNumber)
{
    m_iPacketNumber = GetIntFromChar(PacketNumber, 0, 1);
}

int CVideoHeader::getTimeStampDirectly(unsigned char *data)
{
	return GetIntFromChar(data + TIMESTAMP_INDEX, 0, 4);
}

unsigned int CVideoHeader::getTimeStamp()
{
    return m_iTimeStamp;
}

void CVideoHeader::setTimeStamp(unsigned char *TimeStamp)
{
    m_iTimeStamp = GetIntFromChar(TimeStamp, 0, 4);
}

unsigned int CVideoHeader::getFPS()
{
    return m_iFPS;
}

void CVideoHeader::setFPS(unsigned char *FPS)
{
    m_iFPS  = GetIntFromChar(FPS, 0, 1);
}

int CVideoHeader::getPacketLength()
{
    return m_iPacketDataLength;
}

void CVideoHeader::setPacketDataLength(unsigned char *PacketDataLength)
{
    m_iPacketDataLength = GetIntFromChar(PacketDataLength, 0, 3);
}

unsigned int CVideoHeader::GetFrameNumberDirectly(unsigned char *packetData)
{
	return GetIntFromChar(packetData + FRAME_NUMBER_INDEX, 0, 3);
    
}

int CVideoHeader::GetIntFromChar(unsigned char *packetData, int index, int nLenght)
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

int CVideoHeader::GetOpponentResolution(unsigned char *PacketHeader)
{
    return PacketHeader[CALL_INFO_BYTE_INDEX_WITHOUT_MEDIA] & 0x06;
}

void CVideoHeader::SetNetworkType(unsigned char* data){
    m_nNetworkType = (data[0] & 1);
}

void CVideoHeader::SetDeviceOrientation(unsigned char *packetData)
{
    m_iDeviceOrientation = (GetIntFromChar(packetData, 0, 1) >> 1 ) & ORIENTATION_BITSET;
}

void CVideoHeader::SetVideoQualityLevel(unsigned char* data){
    m_nVideoQualityLevel = (GetIntFromChar(data, 0, 1) >> 3 ) & QUALITY_LEVEL_BITSET;
}

void CVideoHeader::SetNetworkType(int nNetworkType){
    m_nNetworkType = nNetworkType;
}

void CVideoHeader::SetVideoQualityLevel(int nQualityLevel){
    m_nVideoQualityLevel = nQualityLevel;
}

void CVideoHeader::SetDeviceOrientation(int deviceOrientation)
{
    m_iDeviceOrientation = deviceOrientation;
}

int CVideoHeader::GetDeviceOrientation() {
    return m_iDeviceOrientation;
}

int CVideoHeader::GetVideoQualityLevel(){
    return m_nVideoQualityLevel;
}

int CVideoHeader::GetNetworkType(){
    return m_nNetworkType;
}


void CVideoHeader::setHeaderLength(int iHeaderLength)
{
    m_iHeaderLength = iHeaderLength;
}
void CVideoHeader::setHeaderLength(unsigned char *pData)
{
    m_iHeaderLength = pData[0];
}
int CVideoHeader::GetHeaderLength()
{
    return m_iHeaderLength;
}



void CVideoHeader::setPacketStartingIndex(int iPacketStartingIndex)
{
    m_iPacketStartingIndex = iPacketStartingIndex;
}

void CVideoHeader::setPacketStartingIndex(unsigned char *pData)
{
    m_iPacketStartingIndex = GetIntFromChar(pData, 0, 3);
}

int CVideoHeader::GetPacketStartingIndex()
{
    return m_iPacketStartingIndex;
}



