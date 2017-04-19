#include "AudioHeaderCommon.h"

#include "LogPrinter.h"


AudioHeaderCommon::AudioHeaderCommon()
{
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
	AudioHeaderCommon();
	CopyInformationToHeader(Information);
}

AudioHeaderCommon::AudioHeaderCommon(unsigned char *Header)
{
	AudioHeaderCommon();

	CopyHeaderToInformation(Header);
}

AudioHeaderCommon::~AudioHeaderCommon()
{
	memset(m_arrllInformation, 0, sizeof(m_arrllInformation));
	memset(ma_uchHeader, 0, m_nHeaderSizeInByte);
}

void AudioHeaderCommon::InitHeaderBitMap()
{
	HeaderBitmap[0] = 8 /*INF_PACKETTYPE*/;
	HeaderBitmap[1] = 6 /*INF_HEADERLENGTH*/;
	HeaderBitmap[2] = 2 /*INF_NETWORKTYPE*/;
	HeaderBitmap[3] = 5 /*INF_VERSIONCODE*/;
	HeaderBitmap[4] = 31 /*INF_PACKETNUMBER*/;
	HeaderBitmap[5] = 12 /*INF_BLOCK_LENGTH*/;
	HeaderBitmap[6] = 3 /*INF_RECVDSLOTNUMBER*/;
	HeaderBitmap[7] = 8 /*INF_NUMPACKETRECVD*/;
	HeaderBitmap[8] = 2 /*INF_CHANNELS*/;
	HeaderBitmap[9] = 3 /*INF_SLOTNUMBER*/;
	HeaderBitmap[10] = 40 /*INF_TIMESTAMP*/;
	HeaderBitmap[11] = 4 /*INF_BLOCK_NUMBER*/;
	HeaderBitmap[12] = 4 /*INF_TOTAL_BLOCK*/;
	HeaderBitmap[13] = 16 /*INF_BLOCK_OFFSET*/;
	HeaderBitmap[14] = 16 /*INF_FRAME_LENGTH*/;
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

	if (infoStartBit + HeaderBitmap[InfoType] >(m_nProcessingHeaderSizeInByte << 3))
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

void AudioHeaderCommon::showDetails(char prefix[])
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

void AudioHeaderCommon::SetHeaderAllInByteArray(unsigned char* header, const AudioHeaderFields& params)
{
	SetInformation(params.packetType, INF_PACKETTYPE);
	SetInformation(params.headerLength, INF_HEADERLENGTH);
	SetInformation(params.packetNumber, INF_PACKETNUMBER);
	SetInformation(params.slotNumber, INF_SLOTNUMBER);
	SetInformation(params.blockLength, INF_BLOCK_LENGTH);
	SetInformation(params.recvSlotNumber, INF_RECVDSLOTNUMBER);
	SetInformation(params.numPacketRecv, INF_NUMPACKETRECVD);
	SetInformation(params.version, INF_VERSIONCODE);
	SetInformation(params.timestamp, INF_TIMESTAMP);
	SetInformation(params.networkType, INF_NETWORKTYPE);
	SetInformation(params.channel, INF_CHANNELS);
	SetInformation(params.blockNumber, INF_PACKET_BLOCK_NUMBER);
	SetInformation(params.totalBlocksInThisFrame, INF_TOTAL_PACKET_BLOCKS);
	SetInformation(params.blockOffset, INF_BLOCK_OFFSET);
	SetInformation(params.frameLength, INF_FRAME_LENGTH);

	showDetails("@#BUILD");

	GetHeaderInByteArray(header);
}


void AudioHeaderCommon::SetHeaderAllInByteArray(unsigned char* header, int packetType, int nHeaderLength, int networkType, int slotNumber, int packetNumber, int packetLength, int recvSlotNumber,
	int numPacketRecv, int channel, int version, long long timestamp, int iBlockNumber, int nTotalBlocksInThisFrame, int nBlockOffset, int nFrameLength)
{
	//LOGEF("##EN### BuildAndGetHeader ptype %d ntype %d slotnumber %d packetnumber %d plength %d reslnumber %d npacrecv %d channel %d version %d time %lld",
	//	packetType, networkType, slotNumber, packetNumber, packetLength, recvSlotNumber, numPacketRecv, channel, version, timestamp);
	SetInformation(packetType, INF_PACKETTYPE);
	SetInformation(nHeaderLength, INF_HEADERLENGTH);
	SetInformation(packetNumber, INF_PACKETNUMBER);
	SetInformation(slotNumber, INF_SLOTNUMBER);
	SetInformation(packetLength, INF_BLOCK_LENGTH);
	SetInformation(recvSlotNumber, INF_RECVDSLOTNUMBER);
	SetInformation(numPacketRecv, INF_NUMPACKETRECVD);
	SetInformation(version, INF_VERSIONCODE);
	SetInformation(timestamp, INF_TIMESTAMP);
	SetInformation(networkType, INF_NETWORKTYPE);
	SetInformation(channel, INF_CHANNELS);
	SetInformation(iBlockNumber, INF_PACKET_BLOCK_NUMBER);
	SetInformation(nTotalBlocksInThisFrame, INF_TOTAL_PACKET_BLOCKS);
	SetInformation(nBlockOffset, INF_BLOCK_OFFSET);
	SetInformation(nFrameLength, INF_FRAME_LENGTH);

	showDetails("@#BUILD");

	GetHeaderInByteArray(header);
}

void AudioHeaderCommon::GetHeaderInfoAll(unsigned char* header, int &nHeaderLength, int &nFrameNumber, int &iBlockNumber, int &nNumberOfBlocks, int &nBlockLength, int &iOffsetOfBlock, int &nFrameLength)
{
	CopyHeaderToInformation(header);

	// packetType = GetInformation(INF_PACKETTYPE);
	nHeaderLength = GetInformation(INF_HEADERLENGTH);
	// networkType = GetInformation(INF_NETWORKTYPE);
	// slotNumber = GetInformation(INF_SLOTNUMBER);
	nFrameNumber = GetInformation(INF_PACKETNUMBER);
	nBlockLength = GetInformation(INF_BLOCK_LENGTH);
	// recvSlotNumber = GetInformation(INF_RECVDSLOTNUMBER);
	//numPacketRecv = GetInformation(INF_NUMPACKETRECVD);
	// channel = GetInformation(INF_CHANNELS);
	// version = GetInformation(INF_VERSIONCODE);
	// timestamp = GetInformation(INF_TIMESTAMP);

	iBlockNumber = GetInformation(INF_PACKET_BLOCK_NUMBER);
	nNumberOfBlocks = GetInformation(INF_TOTAL_PACKET_BLOCKS);
	iOffsetOfBlock = GetInformation(INF_BLOCK_OFFSET);
	nFrameLength = GetInformation(INF_FRAME_LENGTH);

	if (iBlockNumber == -1)
	{
		iBlockNumber = 0;
	}

	if (nNumberOfBlocks == -1)
	{
		nNumberOfBlocks = 1;
		iOffsetOfBlock = 0;
		nFrameLength = nBlockLength;
	}

	showDetails("@#PARSE");
}
