#ifndef AUDIO_PACKET_HEADER_H
#define AUDIO_PACKET_HEADER_H

#include "SmartPointer.h"

typedef struct
{
	int packetType;
	int nHeaderLength;
	int networkType;
	int slotNumber;
	int packetNumber;
	int packetLength;
	int recvSlotNumber;
	int numPacketRecv;
	int channel;
	int version;
	long long timestamp;
	int iBlockNumber;
	int nTotalBlocksInThisFrame;
	int nBlockOffset;
	int nFrameLength;
} AudioHeaderParams;

enum AudioHeaderInfoTypes
{
	INF_PACKETTYPE = 0,
	INF_HEADERLENGTH,
	INF_NETWORKTYPE,
	INF_VERSIONCODE,
	INF_PACKETNUMBER,
	INF_BLOCK_LENGTH,
	INF_RECVDSLOTNUMBER,
	INF_NUMPACKETRECVD,
	INF_CHANNELS,
	INF_SLOTNUMBER,
	INF_TIMESTAMP,
	INF_PACKET_BLOCK_NUMBER,
	INF_TOTAL_PACKET_BLOCKS,
	INF_BLOCK_OFFSET,
	INF_FRAME_LENGTH
};

enum AudioPacketTypes
{
	AUDIO_SKIP_PACKET_TYPE = 0,
	AUDIO_NORMAL_PACKET_TYPE = 1,
	AUDIO_NOVIDEO_PACKET_TYPE = 2,
	AUDIO_VERSION_PACKET_TYPE = 3,
	AUDIO_CHANNEL_PACKET_TYPE = 7,
	AUDIO_OPUS_PACKET_TYPE = 9,
	AUDIO_LIVE_CALLEE_PACKET_TYPE = 10,
	AUDIO_LIVE_PUBLISHER_PACKET_TYPE_MUXED = 11,
	AUDIO_LIVE_PUBLISHER_PACKET_TYPE_NONMUXED = 12
};

static int SupportedPacketTypes[] =
{
	AUDIO_SKIP_PACKET_TYPE,
	AUDIO_NORMAL_PACKET_TYPE,
	AUDIO_NOVIDEO_PACKET_TYPE,
	AUDIO_VERSION_PACKET_TYPE
}; //Only for Call

enum AudioHeaderTypes
{
	HEADER_COMMON,
	HEADER_CHANNEL,
	HEADER_CALL
};

enum MaxSizes{
	MAXFIELDSINHEADER = 15,
	MAXHEADERSIZE = 100
};

class AudioPacketHeader
{
public:

	//Factory Method to get the header instance

	static SmartPointer<AudioPacketHeader> GetInstance(AudioHeaderTypes type);

	//Constructor
	AudioPacketHeader(){}
	virtual ~AudioPacketHeader(){};

	virtual void SetHeaderAllInByteArray(unsigned char* header, int packetType, int nHeaderLength, int networkType, int slotNumber, int packetNumber, int packetLength, int recvSlotNumber,
		int numPacketRecv, int channel, int version, long long timestamp, int iBlockNumber, int nTotalBlocksInThisFrame, int nBlockOffset, int nFrameLength) = 0;
	//virtual void SetHeaderAllInByteArray(unsigned char* header, AudioHeaderParams& params) = 0;
	virtual void CopyHeaderToInformation(unsigned char *Header) = 0;
	virtual int GetHeaderInByteArray(unsigned char* data) = 0;

	virtual void SetInformation(long long Information, int InfoType) = 0;
	virtual long long GetInformation(int InfoType) = 0;

	virtual long long GetFieldCapacity(int InfoType) = 0;

	virtual bool IsPacketTypeSupported(unsigned int PacketType) = 0;
	virtual bool IsPacketTypeSupported() = 0;

	virtual void showDetails(char prefix[]) = 0;

	virtual int GetHeaderSize()
	{
		return m_nHeaderSizeInByte;
	}

protected:

	int HeaderBitmap[MAXFIELDSINHEADER];

	AudioHeaderParams m_headerParams;

	int nNumberOfHeaderElements;
	unsigned int m_nHeaderSizeInBit;
	unsigned int m_nHeaderSizeInByte;
	unsigned int m_nProcessingHeaderSizeInByte;

	long long m_arrllInformation[MAXFIELDSINHEADER];
	unsigned char ma_uchHeader[MAXHEADERSIZE];

};

#endif