#include "Tools.h"
#include "Size.h"

#define PACKETTYPE 0
#define HEADERLENGTH 1
#define NETWORKTYPE 2
#define VERSIONCODE 3
#define PACKETNUMBER 4
#define PACKETLENGTH 5
#define RECVDSLOTNUMBER 6
#define NUMPACKETRECVD 7
#define CHANNELS 8
#define SLOTNUMBER 9
#define TIMESTAMP 10

/////////PacketTypes//////
#define AUDIO_SKIP_PACKET_TYPE 0
#define AUDIO_NORMAL_PACKET_TYPE 1
#define AUDIO_NOVIDEO_PACKET_TYPE 2
#define AUDIO_VERSION_PACKET_TYPE 3
#define AUDIO_OPUS_PACKET_TYPE 9
#define AUDIO_NONMUXED_PACKET_TYPE 10
#define AUDIO_MUXED_PACKET_TYPE 11

#define MAXFIELDSINHEADER 15
#define MAXHEADERSIZE 100


//#define __AUDIO_HEADER_LENGTH__ 7


static int HeaderBitmap[] =
{
	8 /*PACKETTYPE*/,
	6 /*HEADERLENGTH*/,
	2 /*NETWORKTYPE*/,
	5 /*VERSIONCODE*/,
	31 /*PACKETNUMBER*/,
	12 /*PACKETLENGTH*/,
	3 /*RECVDSLOTNUMBER*/,
	8 /*NUMPACKETRECVD*/,
	2 /*CHANNELS*/,
	3 /*SLOTNUMBER*/,
	40  /*TIMESTAMP*/
};

static int SupportedPacketTypes[] =
{
	AUDIO_SKIP_PACKET_TYPE,
	AUDIO_NORMAL_PACKET_TYPE,
	AUDIO_NOVIDEO_PACKET_TYPE,
	AUDIO_VERSION_PACKET_TYPE,
	AUDIO_OPUS_PACKET_TYPE,
	AUDIO_NONMUXED_PACKET_TYPE,
	AUDIO_MUXED_PACKET_TYPE
};


class CAudioLiveHeader {


	unsigned int m_nHeaderSizeInBit;
	unsigned int m_nHeaderSizeInByte;
	long long m_arrllInformation[MAXFIELDSINHEADER];

	unsigned char ma_uchHeader[MAXHEADERSIZE];
	int nNumberOfHeaderElements;
//	int CopyInformationToHeader(unsigned int * Information);
	void PutInformationToArray(int InfoType);

public:
	CAudioPacketHeader();
//	CAudioPacketHeader(unsigned int * Information);
	CAudioPacketHeader(unsigned char *Header);
	~CAudioPacketHeader();

	void CopyHeaderToInformation(unsigned char *Header);
	int GetHeaderInByteArray(unsigned char* data);

	int GetHeaderSize();

	void SetInformation(long long Information, int InfoType);
	long long GetInformation(int InfoType);

	long long GetFieldCapacity(int InfoType);

	bool IsPacketTypeSupported(unsigned int PacketType);
	bool IsPacketTypeSupported();

};