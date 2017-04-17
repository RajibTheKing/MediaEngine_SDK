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
	InfoPacketType = 0,
	InfoHeaderLength,
	InfoNetwrokType,
	InfoVersionCode,
	InfoPacketNumber,
	InfoBlockLength,
	InfoRecvdSlotNumber,
	InfoNumPacketReceived,
	InfoChannel,
	InfoSlotNumber,
	InfoTimestamp,
	InfoPacketBlockNumber,
	InfoTotalPacketBlocks,
	InfoBlockOffset,
	InfoFrameLength
};

enum AudioPacketTypes
{
	PacketAudioSkip,
	PacketAudioNormal,
	PacketNoVideo,
	PacketVersion,
	PacketAudioChannel,
	PacketAudioOpus,
	PacketAudioLiveCallee,
	PacketAudioLivePublisherMuxed,
	PacketAudioLivePublisherNonMuxed
};

static int SupportedPacketTypes[] =
{
	PacketAudioSkip,
	PacketAudioNormal,
	PacketNoVideo,
	PacketVersion
}; //Only for Call

enum AudioHeaderTypes
{
	HeaderCommon,
	HeaderChannel,
	HeaderVoiceCall
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

	enum MaxSizes{
		MaxFieldsInHeader = 15,
		MaxHeaderSize = 100
	};

	int HeaderBitmap[MaxFieldsInHeader];

	AudioHeaderParams m_headerParams;

	int nNumberOfHeaderElements;
	unsigned int m_nHeaderSizeInBit;
	unsigned int m_nHeaderSizeInByte;
	unsigned int m_nProcessingHeaderSizeInByte;

	long long m_arrllInformation[MaxFieldsInHeader];
	unsigned char ma_uchHeader[MaxHeaderSize];

};

#endif