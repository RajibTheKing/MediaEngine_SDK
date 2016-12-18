
#include "AudioPacketHeader.h"
#include "LogPrinter.h"

CAudioPacketHeader::CAudioPacketHeader()
{
	nNumberOfHeaderElements = sizeof(HeaderBitmap)/sizeof(int);

	int headerSizeInBit = 0;
	for (int i = 0; i < nNumberOfHeaderElements; i++)
	{
		headerSizeInBit += HeaderBitmap[i];
	}

	m_nHeaderSizeInBit = headerSizeInBit;
	m_nHeaderSizeInByte = (headerSizeInBit + 7) / 8;

	memset(ma_nInformation, 0, sizeof(HeaderBitmap));
	memset(ma_uchHeader, 0, m_nHeaderSizeInByte);
}

CAudioPacketHeader::CAudioPacketHeader(unsigned int * Information)
{
	CAudioPacketHeader();
	CopyInformationToHeader(Information);
}

CAudioPacketHeader::CAudioPacketHeader(unsigned char *Header)
{
	CAudioPacketHeader();
	CopyHeaderToInformation(Header);
}

CAudioPacketHeader::~CAudioPacketHeader()
{
	memset(ma_nInformation, 0, sizeof(HeaderBitmap));
	memset(ma_uchHeader, 0, m_nHeaderSizeInByte);
}

int CAudioPacketHeader::CopyInformationToHeader(unsigned int * Information)
{
	memcpy(ma_nInformation, Information, m_nHeaderSizeInByte);
	for (int i = 0; i < nNumberOfHeaderElements; i++)
	{
		SetInformation(Information[i], i);
	}
	return m_nHeaderSizeInByte;
}

unsigned int CAudioPacketHeader::GetFieldCapacity(int InfoType)
{
	return 1 << HeaderBitmap[InfoType];
}


int CAudioPacketHeader::GetHeaderInByteArray(unsigned char* data)
{
	memcpy(data, ma_uchHeader, m_nHeaderSizeInByte);
	return m_nHeaderSizeInByte;
}

int CAudioPacketHeader::GetHeaderSize()
{
	return m_nHeaderSizeInByte;
}

bool CAudioPacketHeader::IsPacketTypeSupported(unsigned int PacketType)
{
	int nPacketTypes = sizeof(SupportedPacketTypes) / sizeof(int);
	for (int i = 0; i < nPacketTypes; i++)
	{
		if (SupportedPacketTypes[i] == PacketType) return true;
	}
	return false;
}

bool CAudioPacketHeader::IsPacketTypeSupported()
{
	unsigned int iPackeType = GetInformation(PACKETTYPE);
	return IsPacketTypeSupported(iPackeType);
}

void CAudioPacketHeader::SetInformation(unsigned int Information, int InfoType)
{
    Information = (Information & ((1LL<<HeaderBitmap[InfoType]) - 1));
	ma_nInformation[InfoType] = Information;

	int infoStartBit = 0;
	for (int i = 0; i < InfoType; i++)
	{
		infoStartBit += HeaderBitmap[i];
	}
	int infoStartByte = infoStartBit / 8;
	int infoStartBitOfByte = infoStartBit % 8;

	int numberOfBitsIn1stByte;
	if (8 - infoStartBitOfByte >= HeaderBitmap[InfoType])
	{
		numberOfBitsIn1stByte = min(HeaderBitmap[InfoType], 8 - infoStartBitOfByte);
		ma_uchHeader[infoStartByte] &= ~( ((1 << numberOfBitsIn1stByte) - 1) << (8 - infoStartBitOfByte - numberOfBitsIn1stByte) ) ;
		ma_uchHeader[infoStartByte] |= (Information >> (HeaderBitmap[InfoType] - numberOfBitsIn1stByte)) << (8 - infoStartBitOfByte - numberOfBitsIn1stByte);
	}
	else
	{
		numberOfBitsIn1stByte = 8 - infoStartBitOfByte;
		ma_uchHeader[infoStartByte] &=  ~((1<<numberOfBitsIn1stByte) - 1);
		ma_uchHeader[infoStartByte] |= (Information >> (HeaderBitmap[InfoType] - numberOfBitsIn1stByte));

		int remainingBits = HeaderBitmap[InfoType] - numberOfBitsIn1stByte;
		int remainingBytes = remainingBits / 8;
		int nBitsInLastByte = remainingBits % 8;
		int byte = 1;

		for(int i = 0; i < remainingBytes; i ++)
		{
			ma_uchHeader[infoStartByte + byte] = Information >> (HeaderBitmap[InfoType] - numberOfBitsIn1stByte - 8 * byte);
			byte++;
		}

		if(nBitsInLastByte)
		{
			ma_uchHeader[infoStartByte + byte] &=  ~ ( ( (1<<nBitsInLastByte) - 1) << (8 - nBitsInLastByte));
			ma_uchHeader[infoStartByte + byte] |= 0xFF & (Information << (8-nBitsInLastByte) );
		}
	}
}

void CAudioPacketHeader::CopyHeaderToInformation(unsigned char *Header)
{
	memcpy(ma_uchHeader, Header, m_nHeaderSizeInByte);

	for (int i = 0; i < nNumberOfHeaderElements; i++)
	{
		//CLogPrinter_WriteLog(CLogPrinter::INFO, INSTENT_TEST_LOG,"#PutInformationToArray#");
		PutInformationToArray(i);
	}
}

unsigned int CAudioPacketHeader::GetInformation(int InfoType)
{
	return ma_nInformation[InfoType];
}

void CAudioPacketHeader::PutInformationToArray(int InfoType)
{
	unsigned long long Information = 0;
	int infoStartBit = 0;
	for (int i = 0; i < InfoType; i++)
	{
		infoStartBit += HeaderBitmap[i];
	}
	int infoStartByte = infoStartBit / 8;
	int infoStartBitOfByte = infoStartBit % 8;

    
	if (infoStartBitOfByte + HeaderBitmap[InfoType] <= 8)//fits in 1 byte
	{
		unsigned char temp = (ma_uchHeader[infoStartByte] << infoStartBitOfByte);
		Information = (temp >>( 8 - HeaderBitmap[InfoType]));
	}
	else
	{
		int nBitesToCopy = HeaderBitmap[InfoType] + infoStartBitOfByte;
		int nBytesToCopy = nBitesToCopy / 8 + (nBitesToCopy % 8 != 0);
		for (int i = 0; i < nBytesToCopy; i++)
		{
			Information <<= 8;
			Information += ma_uchHeader[infoStartByte + i];
		}

		int shift = ((HeaderBitmap[InfoType] + infoStartBitOfByte ) % 8);
		if(shift)
			Information >>= 8 - shift;
		Information &= (1LL << HeaderBitmap[InfoType]) - 1;
	}
	ma_nInformation[InfoType] = Information;
}
