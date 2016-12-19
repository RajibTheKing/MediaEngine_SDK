
#include "AudioHeader.h"
#include "LogPrinter.h"

AudioHeader::AudioHeader()
{
	nNumberOfHeaderElements = sizeof(HeaderBitmapOld)/sizeof(int);

	int headerSizeInBit = 0;
	for (int i = 0; i < nNumberOfHeaderElements; i++)
	{
		headerSizeInBit += HeaderBitmapOld[i];
	}

	m_nHeaderSizeInBit = headerSizeInBit;
	m_nHeaderSizeInByte = (headerSizeInBit + 7) / 8;

	memset(ma_nInformation, 0, sizeof(HeaderBitmapOld));
	memset(ma_uchHeader, 0, m_nHeaderSizeInByte);
}

AudioHeader::AudioHeader(unsigned int * Information)
{
	AudioHeader();
	CopyInformationToHeader(Information);
}

AudioHeader::AudioHeader(unsigned char *Header)
{
	AudioHeader();
	CopyHeaderToInformation(Header);
}

AudioHeader::~AudioHeader()
{
	memset(ma_nInformation, 0, sizeof(HeaderBitmapOld));
	memset(ma_uchHeader, 0, m_nHeaderSizeInByte);
}

int AudioHeader::CopyInformationToHeader(unsigned int * Information)
{
	memcpy(ma_nInformation, Information, m_nHeaderSizeInByte);
	for (int i = 0; i < nNumberOfHeaderElements; i++)
	{
		SetInformation(Information[i], i);
	}
	return m_nHeaderSizeInByte;
}

unsigned int AudioHeader::GetFieldCapacity(int InfoType)
{
	return 1 << HeaderBitmapOld[InfoType];
}


int AudioHeader::GetHeaderInByteArray(unsigned char* data)
{
	memcpy(data, ma_uchHeader, m_nHeaderSizeInByte);
	return m_nHeaderSizeInByte;
}

int AudioHeader::GetHeaderSize()
{
	return m_nHeaderSizeInByte;
}

bool AudioHeader::IsPacketTypeSupported(unsigned int PacketType)
{
	int nPacketTypes = sizeof(SupportedPacketTypesOld) / sizeof(int);
	for (int i = 0; i < nPacketTypes; i++)
	{
		if (SupportedPacketTypesOld[i] == PacketType) return true;
	}
	return false;
}

void AudioHeader::SetInformation(unsigned int Information, int InfoType)
{
	Information = (Information & ((1<<HeaderBitmapOld[InfoType]) - 1));
	ma_nInformation[InfoType] = Information;

	int infoStartBit = 0;
	for (int i = 0; i < InfoType; i++)
	{
		infoStartBit += HeaderBitmapOld[i];
	}
	int infoStartByte = infoStartBit / 8;
	int infoStartBitOfByte = infoStartBit % 8;

	int numberOfBitsIn1stByte;
	if (8 - infoStartBitOfByte >= HeaderBitmapOld[InfoType])
	{
		numberOfBitsIn1stByte = min(HeaderBitmapOld[InfoType], 8 - infoStartBitOfByte);
		ma_uchHeader[infoStartByte] &= ~( ((1 << numberOfBitsIn1stByte) - 1) << (8 - infoStartBitOfByte - numberOfBitsIn1stByte) ) ;
		ma_uchHeader[infoStartByte] |= (Information >> (HeaderBitmapOld[InfoType] - numberOfBitsIn1stByte)) << (8 - infoStartBitOfByte - numberOfBitsIn1stByte);
	}
	else
	{
		numberOfBitsIn1stByte = 8 - infoStartBitOfByte;
		ma_uchHeader[infoStartByte] &=  ~((1<<numberOfBitsIn1stByte) - 1);
		ma_uchHeader[infoStartByte] |= (Information >> (HeaderBitmapOld[InfoType] - numberOfBitsIn1stByte));

		int remainingBits = HeaderBitmapOld[InfoType] - numberOfBitsIn1stByte;
		int remainingBytes = remainingBits/8;
		int nBitsInLastByte = remainingBits % 8;
		int byte = 1;

		for(int i = 0; i < remainingBytes; i ++)
		{
			ma_uchHeader[infoStartByte + byte] = Information >> (HeaderBitmapOld[InfoType] - numberOfBitsIn1stByte - 8 * byte);
			byte++;
		}

		if(nBitsInLastByte)
		{
			ma_uchHeader[infoStartByte + byte] &=  ~ ( ( (1<<nBitsInLastByte) - 1) << (8 - nBitsInLastByte));
			ma_uchHeader[infoStartByte + byte] |= 0xFF & (Information << (8-nBitsInLastByte) );
		}
	}
}

void AudioHeader::CopyHeaderToInformation(unsigned char *Header)
{
	memcpy(ma_uchHeader, Header, m_nHeaderSizeInByte);

	for (int i = 0; i < nNumberOfHeaderElements; i++)
	{
		//CLogPrinter_WriteLog(CLogPrinter::INFO, INSTENT_TEST_LOG,"#PutInformationToArray#");
		PutInformationToArray(i);
	}
}

unsigned int AudioHeader::GetInformation(int InfoType)
{
	return ma_nInformation[InfoType];
}

void AudioHeader::PutInformationToArray(int InfoType)
{
	unsigned long long Information = 0;
	int infoStartBit = 0;
	for (int i = 0; i < InfoType; i++)
	{
		infoStartBit += HeaderBitmapOld[i];
	}
	int infoStartByte = infoStartBit / 8;
	int infoStartBitOfByte = infoStartBit % 8;

	if (infoStartBitOfByte + HeaderBitmapOld[InfoType] <= 8)//fits in 1 byte
	{
		unsigned char temp = (ma_uchHeader[infoStartByte] << infoStartBitOfByte);
		Information = (temp >>( 8 - HeaderBitmapOld[InfoType]));
	}
	else
	{
		int nBitesToCopy = HeaderBitmapOld[InfoType] + infoStartBitOfByte;
		int nBytesToCopy = nBitesToCopy / 8 + (nBitesToCopy % 8 != 0);
		for (int i = 0; i < nBytesToCopy; i++)
		{
			Information <<= 8;
			Information += ma_uchHeader[infoStartByte + i];
		}
		int shift = ((HeaderBitmapOld[InfoType] + infoStartBitOfByte ) % 8);
		if(shift)
			Information >>= 8 - shift;
		Information &= (1 << HeaderBitmapOld[InfoType]) - 1;
	}
	ma_nInformation[InfoType] = Information;
}
