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



#define QUALITY_BITS_N      3
#define ORIENTATION_BITS_N  2

#define QUALITY_LEVEL_BITSET    ((1<<QUALITY_BITS_N) - 1)
#define ORIENTATION_BITSET      ((1<<ORIENTATION_BITS_N) - 1)


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



void CVideoHeader::setPacketHeader(	unsigned char packetType,
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
                                    )
{
    SetPacketType(packetType);
    setVersionCode(uchVersion);
	setHeaderLength(iHeaderLength);
	setFPS(iFPSbyte);
	setFrameNumber(llFrameNumber);

	//CallInfoByte
	SetNetworkType(iNetworkType);
	SetDeviceOrientation(iDeviceOrientation);
	SetVideoQualityLevel(iQualityLevel);
	
	setNumberOfPacket(NumberOfPacket);
    setPacketNumber(PacketNumber);

	setTimeStamp(llTimeStamp);
	setPacketStartingIndex(iPacketStartingIndex);
	setPacketDataLength(PacketLength);


}

void CVideoHeader::ShowDetails(string sTag)
{
    string sLog = "#PKT#  -> "+sTag
    +" PT: "+Tools::getText(m_iPacketType)
    +" VC:"+Tools::getText(m_iVersionCode)
    +" HL:"+Tools::getText(m_iHeaderLength)
    +" Fps:"+Tools::getText(m_iFPS)
    +" FN:"+Tools::getText(m_llFrameNumber)
    +" NT:"+Tools::getText(m_iNetworkType)
    +" DO:"+Tools::getText(m_iDeviceOrientation)
    +" QL:"+Tools::getText(m_iVideoQualityLevel)
    +" NP:"+Tools::getText(m_iNumberOfPacket)
    +" PN:"+Tools::getText(m_iPacketNumber)
    +" TM:"+Tools::getText(m_llTimeStamp)
    +" SIndex:"+Tools::getText(m_iPacketStartingIndex)
    +" Len:"+Tools::getText(m_iPacketDataLength)
    ;
    
    printf("%s\n", sLog.c_str());
    
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
	data[index++] = (m_llFrameNumber >> 24);
    data[index++] = (m_llFrameNumber >> 16);
    data[index++] = (m_llFrameNumber >> 8);
    data[index++] = (m_llFrameNumber >> 0);

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
	data[index++] = (m_llTimeStamp >> 32);
    data[index++] = (m_llTimeStamp >> 24);
    data[index++] = (m_llTimeStamp >> 16);
    data[index++] = (m_llTimeStamp >> 8);
    data[index++] = (m_llTimeStamp >> 0);

	//Packet Starting Index
	data[index++] = (m_iPacketStartingIndex >> 16);
	data[index++] = (m_iPacketStartingIndex >> 8);
	data[index++] = (m_iPacketStartingIndex >> 0);

	//Packet Data Length
	data[index++] = (m_iPacketDataLength >> 16);
	data[index++] = (m_iPacketDataLength >> 8);
	data[index++] = (m_iPacketDataLength >> 0);

    return index;
}

void CVideoHeader::SetPacketType(unsigned char packetType)
{
    m_iPacketType = packetType;
}

unsigned char CVideoHeader::GetPacketType(){
    return (unsigned char)m_iPacketType;
}

void CVideoHeader::setVersionCode(unsigned char cVersionCode) {
    m_iVersionCode = cVersionCode;
}

void CVideoHeader::setFrameNumber(long long llFrameNumber) {
    m_llFrameNumber = llFrameNumber;
}

void CVideoHeader::setNumberOfPacket(unsigned int iNumberOfPacket) {
    m_iNumberOfPacket = iNumberOfPacket;
}

void CVideoHeader::setPacketNumber(unsigned int iPacketNumber) {
    m_iPacketNumber = iPacketNumber;
}

void CVideoHeader::setTimeStamp(long long llTimeStamp) {
    m_llTimeStamp = llTimeStamp;
}

void CVideoHeader::setFPS(unsigned int iFPS) {
    m_iFPS = iFPS;
}

void CVideoHeader::setPacketDataLength(int iPacketDataLength) {
    m_iPacketDataLength = iPacketDataLength;
}

unsigned char CVideoHeader::getVersionCode() {
	return (unsigned char)m_iVersionCode;
}

void CVideoHeader::setVersionCode(unsigned char *VersionCodeByte) {
    m_iVersionCode = VersionCodeByte[0];
}

long long CVideoHeader::getFrameNumber()
{
    return m_llFrameNumber;
}
void CVideoHeader::setPacketType(unsigned char *pData)
{
    m_iPacketType = pData[0];
}

void CVideoHeader::setFrameNumber(unsigned char *pData)
{
    m_llFrameNumber = GetLongLongFromChar(pData, 0, 4);
}

unsigned int CVideoHeader::getNumberOfPacket()
{
    return m_iNumberOfPacket;
}

void CVideoHeader::setNumberOfPacket(unsigned char *pData)
{
	m_iNumberOfPacket = GetIntFromChar(pData, 0, 1);
}

unsigned int CVideoHeader::getPacketNumber()
{
    return m_iPacketNumber;
}

void CVideoHeader::setPacketNumber(unsigned char *pData)
{
	m_iPacketNumber = GetIntFromChar(pData, 0, 1);
}

long long CVideoHeader::getTimeStampDirectly(unsigned char *pData)
{
	return GetIntFromChar(pData + TIMESTAMP_INDEX, 0, 5);
}

long long CVideoHeader::getTimeStamp()
{
    return m_llTimeStamp;
}

void CVideoHeader::setTimeStamp(unsigned char *pData)
{
    m_llTimeStamp = GetLongLongFromChar(pData, 0, 5);
}

unsigned int CVideoHeader::getFPS()
{
    return m_iFPS;
}

void CVideoHeader::setFPS(unsigned char *pData)
{
	m_iFPS = GetIntFromChar(pData, 0, 1);
}

int CVideoHeader::getPacketLength()
{
    return m_iPacketDataLength;
}

void CVideoHeader::setPacketDataLength(unsigned char *pData)
{
	m_iPacketDataLength = GetIntFromChar(pData, 0, 3);
}

long long CVideoHeader::GetFrameNumberDirectly(unsigned char *pData)
{
	return GetLongLongFromChar(pData + FRAME_NUMBER_INDEX, 0, 4);
    
}

int CVideoHeader::GetIntFromChar(unsigned char *packetData, int index, int nLenght)
{
    int result = 0;
    int interval = 8;
    int startPoint = (nLenght - 1) << 3;
    for(int i=startPoint; i >= 0 ; i-=interval)
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
    m_iDeviceOrientation = (GetIntFromChar(packetData, 0, 1) >> 1 ) & ORIENTATION_BITSET;
}

void CVideoHeader::SetVideoQualityLevel(unsigned char* data){
    m_iVideoQualityLevel = (GetIntFromChar(data, 0, 1) >> 3 ) & QUALITY_LEVEL_BITSET;
}

void CVideoHeader::SetNetworkType(int nNetworkType){
    m_iNetworkType = nNetworkType;
}

void CVideoHeader::SetVideoQualityLevel(int nQualityLevel){
    m_iVideoQualityLevel = nQualityLevel;
}

void CVideoHeader::SetDeviceOrientation(int deviceOrientation)
{
    m_iDeviceOrientation = deviceOrientation;
}

int CVideoHeader::GetDeviceOrientation() {
    return m_iDeviceOrientation;
}

int CVideoHeader::GetVideoQualityLevel(){
    return m_iVideoQualityLevel;
}

int CVideoHeader::GetNetworkType(){
    return m_iNetworkType;
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



