#include "Tools.h"
#include "Size.h"

#define PACKETTYPE 0
#define NETWORKTYPE 1
#define SLOTNUMBER 2
#define PACKETNUMBER 3
#define PACKETLENGTH 4
#define RECVDSLOTNUMBER 5
#define NUMPACKETRECVD 6
#define CHANNELS 7
#define SAMPLERATE 8

/////////PacketTypes//////
#define AUDIO_SKIP_PACKET_TYPE 0
#define AUDIO_NORMAL_PACKET_TYPE 1
#define AUDIO_NOVIDEO_PACKET_TYPE 2

#define MAXFIELDSINHEADER 15
#define MAXHEADERSIZE 100


static int HeaderBitmap[] =
{
	6 /*PACKETTYPE*/,
	2 /*NETWORKTYPE*/,
	3 /*SLOTNUMBER*/,
	13 /*PACKETNUMBER*/,
	14 /*PACKETLENGTH*/,
	3 /*RECVDSLOTNUMBER*/,
	8 /*NUMPACKETRECVD*/,
	2 /*CHANNELS*/,
	5 /*SAMPLERATE*/
};


class CAudioPacketHeader {


	unsigned int m_nHeaderSizeInBit;
	unsigned int m_nHeaderSizeInByte;
	unsigned int ma_nInformation[MAXFIELDSINHEADER];

	unsigned char ma_uchHeader[MAXHEADERSIZE];
	int nNumberOfHeaderElements;

public:
	CAudioPacketHeader();
	CAudioPacketHeader(unsigned int * Information);
	CAudioPacketHeader(unsigned char *Header);
	~CAudioPacketHeader();

	int CopyInformationToHeader(unsigned int * Information);
	void CopyHeaderToInformation(unsigned char *Header);

	int GetHeaderInByteArray(unsigned char* data);
	int GetHeaderSize();

	void SetInformation(unsigned int Information, int InfoType);
	void PutInformationToArray(int InfoType);
	unsigned int GetInformation(int InfoType);

	unsigned int GetFieldCapacity(int InfoType);

};