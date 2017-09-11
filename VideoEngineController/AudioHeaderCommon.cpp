#include "AudioHeaderCommon.h"

#include "LogPrinter.h"

namespace MediaSDK
{

	AudioHeaderCommon::AudioHeaderCommon()
	{
		MR_DEBUG("#resorce#header# AudioHeaderCommon::AudioHeaderCommon() - 1");

		InitHeaderBitMap();

		nNumberOfHeaderElements = sizeof(HeaderBitmap) / sizeof(int);

		int headerSizeInBit = 0;
		for (int i = 0; i < nNumberOfHeaderElements; i++)
		{
			headerSizeInBit += HeaderBitmap[i];
		}

		m_nHeaderSizeInBit = headerSizeInBit;
		m_nHeaderSizeInByte = (headerSizeInBit + 7) / 8;
		m_nProcessingHeaderSizeInByte = m_nHeaderSizeInByte;
		memset(m_arrllInformation, 0, sizeof(m_arrllInformation));
		memset(ma_uchHeader, 0, m_nHeaderSizeInByte);
	}

	AudioHeaderCommon::AudioHeaderCommon(unsigned int * Information)
	{
		MR_DEBUG("#resorce#header# AudioHeaderCommon::AudioHeaderCommon() - 2");

		AudioHeaderCommon();
		CopyInformationToHeader(Information);
	}

	AudioHeaderCommon::AudioHeaderCommon(unsigned char *Header)
	{
		MR_DEBUG("#resorce#header# AudioHeaderCommon::AudioHeaderCommon() - 3");

		AudioHeaderCommon();

		CopyHeaderToInformation(Header);
	}

	AudioHeaderCommon::~AudioHeaderCommon()
	{
		MR_DEBUG("#resorce#header# AudioHeaderCommon::~AudioHeaderCommon()");

		memset(m_arrllInformation, 0, sizeof(m_arrllInformation));
		memset(ma_uchHeader, 0, m_nHeaderSizeInByte);
	}

	void AudioHeaderCommon::InitHeaderBitMap()
	{
		HeaderBitmap[INF_PACKETTYPE] = 8;
		HeaderBitmap[INF_HEADERLENGTH] = 6;
		HeaderBitmap[INF_NETWORKTYPE] = 2;
		HeaderBitmap[INF_VERSIONCODE] = 5;
		HeaderBitmap[INF_PACKETNUMBER] = 31;
		HeaderBitmap[INF_BLOCK_LENGTH] = 12;
		HeaderBitmap[INF_CHANNELS] = 2;
		HeaderBitmap[INF_TIMESTAMP] = 40;
		HeaderBitmap[INF_PACKET_BLOCK_NUMBER] = 4;
		HeaderBitmap[INF_TOTAL_PACKET_BLOCKS] = 4;
		HeaderBitmap[INF_BLOCK_OFFSET] = 16;
		HeaderBitmap[INF_FRAME_LENGTH] = 16;

		HeaderFieldNames[INF_PACKETTYPE] = "INF_PACKETTYPE";
		HeaderFieldNames[INF_HEADERLENGTH] = "INF_HEADERLENGTH";
		HeaderFieldNames[INF_NETWORKTYPE] = "INF_NETWORKTYPE";
		HeaderFieldNames[INF_VERSIONCODE] = "INF_VERSIONCODE";
		HeaderFieldNames[INF_PACKETNUMBER] = "INF_PACKETNUMBER";
		HeaderFieldNames[INF_BLOCK_LENGTH] = "INF_BLOCK_LENGTH";
		HeaderFieldNames[INF_CHANNELS] = "INF_CHANNELS";
		HeaderFieldNames[INF_TIMESTAMP] = "INF_TIMESTAMP";
		HeaderFieldNames[INF_PACKET_BLOCK_NUMBER] = "INF_PACKET_BLOCK_NUMBER";
		HeaderFieldNames[INF_TOTAL_PACKET_BLOCKS] = "INF_TOTAL_PACKET_BLOCKS";
		HeaderFieldNames[INF_BLOCK_OFFSET] = "INF_BLOCK_OFFSET";
		HeaderFieldNames[INF_FRAME_LENGTH] = "INF_FRAME_LENGTH";

	}

	int AudioHeaderCommon::CopyInformationToHeader(unsigned int * Information)
	{
		memcpy(m_arrllInformation, Information, m_nHeaderSizeInByte);
		for (int i = 0; i < nNumberOfHeaderElements; i++)
		{
			SetInformation(Information[i], i);
		}
		return m_nHeaderSizeInByte;
	}

	long long AudioHeaderCommon::GetFieldCapacity(int InfoType)
	{
		return 1LL << HeaderBitmap[InfoType];
	}


	int AudioHeaderCommon::GetHeaderInByteArray(unsigned char* data)
	{
		memcpy(data, ma_uchHeader, m_nHeaderSizeInByte);
		return m_nHeaderSizeInByte;
	}

	int AudioHeaderCommon::GetHeaderSize()
	{
		return m_nHeaderSizeInByte;
	}

	bool AudioHeaderCommon::IsPacketTypeSupported(unsigned int PacketType)
	{
		int nPacketTypes = sizeof(SupportedPacketTypes) / sizeof(int);
		for (int i = 0; i < nPacketTypes; i++)
		{
			if (SupportedPacketTypes[i] == PacketType) return true;
		}
		return false;
	}

	bool AudioHeaderCommon::IsPacketTypeSupported()
	{
		unsigned int iPackeType = GetInformation(INF_PACKETTYPE);
		return IsPacketTypeSupported(iPackeType);
	}

	void AudioHeaderCommon::SetInformation(long long Information, int InfoType)
	{
		Information = (Information & ((1LL << HeaderBitmap[InfoType]) - 1));
		m_arrllInformation[InfoType] = Information;

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
			ma_uchHeader[infoStartByte] &= ~(((1 << numberOfBitsIn1stByte) - 1) << (8 - infoStartBitOfByte - numberOfBitsIn1stByte));
			ma_uchHeader[infoStartByte] |= (Information >> (HeaderBitmap[InfoType] - numberOfBitsIn1stByte)) << (8 - infoStartBitOfByte - numberOfBitsIn1stByte);
		}
		else
		{
			numberOfBitsIn1stByte = 8 - infoStartBitOfByte;
			ma_uchHeader[infoStartByte] &= ~((1 << numberOfBitsIn1stByte) - 1);
			ma_uchHeader[infoStartByte] |= (Information >> (HeaderBitmap[InfoType] - numberOfBitsIn1stByte));

			int remainingBits = HeaderBitmap[InfoType] - numberOfBitsIn1stByte;
			int remainingBytes = remainingBits / 8;
			int nBitsInLastByte = remainingBits % 8;
			int byte = 1;

			for (int i = 0; i < remainingBytes; i++)
			{
				ma_uchHeader[infoStartByte + byte] = Information >> (HeaderBitmap[InfoType] - numberOfBitsIn1stByte - 8 * byte);
				byte++;
			}

			if (nBitsInLastByte)
			{
				ma_uchHeader[infoStartByte + byte] &= ~(((1 << nBitsInLastByte) - 1) << (8 - nBitsInLastByte));
				ma_uchHeader[infoStartByte + byte] |= 0xFF & (Information << (8 - nBitsInLastByte));
			}
		}
	}

	void AudioHeaderCommon::CopyHeaderToInformation(unsigned char *Header)
	{
		m_nProcessingHeaderSizeInByte = m_nHeaderSizeInByte;

		memcpy(ma_uchHeader, Header, m_nHeaderSizeInByte);

		for (int i = 0; i < nNumberOfHeaderElements; i++)
		{
			//CLogPrinter_WriteLog(CLogPrinter::INFO, INSTENT_TEST_LOG,"#PutInformationToArray#");
			PutInformationToArray(i);
			if (INF_HEADERLENGTH == i && m_nHeaderSizeInByte != m_arrllInformation[INF_HEADERLENGTH]) {

				m_nProcessingHeaderSizeInByte = m_arrllInformation[INF_HEADERLENGTH];
				HITLER("XXP@#@#MARUF H LEN UPDATED ..%u", m_nProcessingHeaderSizeInByte);
			}
		}
	}

	long long AudioHeaderCommon::GetInformation(int InfoType)
	{
		return m_arrllInformation[InfoType];
	}

	bool AudioHeaderCommon::PutInformationToArray(int InfoType)
	{
		unsigned long long Information = 0;
		int infoStartBit = 0;
		for (int i = 0; i < InfoType; i++)
		{
			infoStartBit += HeaderBitmap[i];
		}
		int infoStartByte = infoStartBit / 8;
		int infoStartBitOfByte = infoStartBit % 8;

		if (infoStartBit + HeaderBitmap[InfoType] > (m_nProcessingHeaderSizeInByte << 3))
		{
			HITLER("XXP@#@#MARUF INFO type = %d , sum = %d bitsize %u", InfoType, infoStartByte + HeaderBitmap[InfoType], (m_nProcessingHeaderSizeInByte << 3));
			m_arrllInformation[InfoType] = -1;
			return false;
		}

		if (infoStartBitOfByte + HeaderBitmap[InfoType] <= 8)//fits in 1 byte
		{
			unsigned char temp = (ma_uchHeader[infoStartByte] << infoStartBitOfByte);
			Information = (temp >> (8 - HeaderBitmap[InfoType]));
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

			int shift = ((HeaderBitmap[InfoType] + infoStartBitOfByte) % 8);
			if (shift)
				Information >>= 8 - shift;
			Information &= (1LL << HeaderBitmap[InfoType]) - 1;
		}
		m_arrllInformation[InfoType] = Information;
		return true;
	}

	void AudioHeaderCommon::ShowDetails(char prefix[])
	{
		HITLER("%s #-> "
			"PT = %lld "
			"HL = %lld "
			"NT = %lld "
			"VC = %lld "
			"PN = %lld "
			"PL = %lld "
			"RECVDSN = %lld "
			"NPRECVD = %lld "
			"C = %lld "
			"SN = %lld "
			"TS = %lld "
			"BN = %lld "
			"TB = %lld "
			"BO = %lld "
			"FL = %lld ",
			prefix,
			m_arrllInformation[0],
			m_arrllInformation[1],
			m_arrllInformation[2],
			m_arrllInformation[3],
			m_arrllInformation[4],
			m_arrllInformation[5],
			m_arrllInformation[6],
			m_arrllInformation[7],
			m_arrllInformation[8],
			m_arrllInformation[9],
			m_arrllInformation[10],
			m_arrllInformation[11],
			m_arrllInformation[12],
			m_arrllInformation[13],
			m_arrllInformation[14]);
	}

} //namespace MediaSDK
