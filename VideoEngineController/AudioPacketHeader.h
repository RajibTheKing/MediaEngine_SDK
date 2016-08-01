#include "Tools.h"
#include "Size.h"

#define PACKETTYPE 0
#define NETWORKTYPE 1
#define SLOTNUMBER 2
#define PACKETNUMBER 3
#define PACKETLENGTH 4
#define RECVDSLOTNUMBER 5
#define NUMPACKETRECVD 6

#define MAXFIELDSINHEADER 10
#define MAXHEADERSIZE 100


static int HeaderBitmap[] =
{
	6 /*PacketType*/,
	2 /*NetworkType*/,
	3 /*SlotNumber*/,
	13 /*PacketNumber*/,
	16 /*PacketLength*/,
	3 /*RecvdSlotNumber*/,
	5 /*NumPacketRecvd*/,
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