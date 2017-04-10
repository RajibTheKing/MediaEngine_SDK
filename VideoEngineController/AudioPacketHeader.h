#ifndef AUDIO_PACKET_HEADER_H
#define AUDIO_PACKET_HEADER_H

#include "Tools.h"
#include "Size.h"

#define INF_PACKETTYPE 0
#define INF_HEADERLENGTH 1
#define INF_NETWORKTYPE 2
#define INF_VERSIONCODE 3
#define INF_PACKETNUMBER 4
#define INF_BLOCK_LENGTH 5
#define INF_RECVDSLOTNUMBER 6
#define INF_NUMPACKETRECVD 7
#define INF_CHANNELS 8
#define INF_SLOTNUMBER 9
#define INF_TIMESTAMP 10
#define INF_PACKET_BLOCK_NUMBER 11
#define INF_TOTAL_PACKET_BLOCKS 12
#define INF_BLOCK_OFFSET 13
#define INF_FRAME_LENGTH 14


/////////PacketTypes//////
#define AUDIO_SKIP_PACKET_TYPE 0
#define AUDIO_NORMAL_PACKET_TYPE 1
#define AUDIO_NOVIDEO_PACKET_TYPE 2
#define AUDIO_VERSION_PACKET_TYPE 3
#define AUDIO_CHANNEL_PACKET_TYPE 7
#define AUDIO_OPUS_PACKET_TYPE 9
#define AUDIO_NONMUXED_LIVE_NONCALL_PACKET_TYPE 10
#define AUDIO_MUXED_PACKET_TYPE 11
#define AUDIO_NONMUXED_LIVE_CALL_PACKET_TYPE 12
#define AUDIO_LIVE_CALLEE_PACKET_TYPE 13
#define AUDIO_LIVE_PUBLISHER_PACKET_TYPE_MUXED 14
#define AUDIO_LIVE_PUBLISHER_PACKET_TYPE_NONMUXED 15


#define MAXFIELDSINHEADER 15
#define MAXHEADERSIZE 100


//#define __AUDIO_HEADER_LENGTH__ 7


static int HeaderBitmap[] =
{
	8 /*INF_PACKETTYPE*/,
	6 /*INF_HEADERLENGTH*/,
	2 /*INF_NETWORKTYPE*/,
	5 /*INF_VERSIONCODE*/,
	31 /*INF_PACKETNUMBER*/,
	12 /*INF_BLOCK_LENGTH*/,
	3 /*INF_RECVDSLOTNUMBER*/,
	8 /*INF_NUMPACKETRECVD*/,
	2 /*INF_CHANNELS*/,
	3 /*INF_SLOTNUMBER*/,
	40 /*INF_TIMESTAMP*/,
	4 /*INF_BLOCK_NUMBER*/,
	4 /*INF_TOTAL_BLOCK*/,
	16 /*INF_BLOCK_OFFSET*/,
	16 /*INF_FRAME_LENGTH*/
};

static int SupportedPacketTypes[] =
{
	AUDIO_SKIP_PACKET_TYPE,
	AUDIO_NORMAL_PACKET_TYPE,
	AUDIO_NOVIDEO_PACKET_TYPE,
	AUDIO_VERSION_PACKET_TYPE
}; //Only for Call


class CAudioPacketHeader {


	unsigned int m_nHeaderSizeInBit;
	unsigned int m_nHeaderSizeInByte;
	unsigned int m_nProcessingHeaderSizeInByte;
	long long m_arrllInformation[MAXFIELDSINHEADER];

	unsigned char ma_uchHeader[MAXHEADERSIZE];
	int nNumberOfHeaderElements;
	//int CopyInformationToHeader(unsigned int * Information);
	bool PutInformationToArray(int InfoType);

public:
	CAudioPacketHeader();
	//CAudioPacketHeader(unsigned int * Information);
	CAudioPacketHeader(unsigned char *Header);
	~CAudioPacketHeader();

	void SetHeaderAllInByteArray(unsigned char* header, int packetType, int nHeaderLength, int networkType, int slotNumber, int packetNumber, int packetLength, int recvSlotNumber,
		int numPacketRecv, int channel, int version, long long timestamp, int iBlockNumber, int nTotalBlocksInThisFrame, int nBlockOffset, int nFrameLength);

	void GetHeaderInfoAll(unsigned char* header, int &nHeaderLength, int &nFrameNumber, int &iBlockNumber, int &nNumberOfBlocks, int &nBlockLength, int &iOffsetOfBlock, int &nFrameLength);

	void CopyHeaderToInformation(unsigned char *Header);
	int GetHeaderInByteArray(unsigned char* data);

	int GetHeaderSize();

	void SetInformation(long long Information, int InfoType);
	long long GetInformation(int InfoType);

	long long GetFieldCapacity(int InfoType);

	bool IsPacketTypeSupported(unsigned int PacketType);
	bool IsPacketTypeSupported();

	void showDetails(string prefix);

};

#endif