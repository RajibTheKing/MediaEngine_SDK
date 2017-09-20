#include "AudioPacketHeader.h"
#include "AudioHeaderCall.h"
#include "AudioHeaderLive.h"
#include "AudioSessionOptions.h"

#include "MediaLogger.h"
#include "LogPrinter.h"
#include "Tools.h"



namespace MediaSDK
{

	SharedPointer<AudioPacketHeader> AudioPacketHeader::GetInstance(AudioHeaderTypes type)
	{
		AudioPacketHeader* pPacketHeader = nullptr;
		switch (type)
		{
		case HEADER_LIVE:
			pPacketHeader = new AudioHeaderLive();
			break;
		case HEADER_COMMON:
			pPacketHeader = new AudioHeaderCall();
			break;
		case HEADER_CHANNEL:
		case HEADER_CALL:
		default:
			pPacketHeader = nullptr;
			break;
		}

		SharedPointer<AudioPacketHeader> pHeader(pPacketHeader);

		return pHeader;
	}

	AudioPacketHeader::~AudioPacketHeader()
	{
		MR_DEBUG("#resorce#header# AudioHeaderCall::~AudioHeaderCall()");

		memset(ma_uchHeader, 0, m_nHeaderSizeInByte);
		delete[] HeaderBitmap;
		delete[] HeaderFieldNames;
		delete[] m_arrllInformation;
	}

	void AudioPacketHeader::InitArrays()
	{
		HeaderBitmap = new int[m_nNumberOfElementsInAudioHeader];
		HeaderFieldNames = new string[m_nNumberOfElementsInAudioHeader];
		m_arrllInformation = new long long[m_nNumberOfElementsInAudioHeader];
	}

	void AudioPacketHeader::ClearMemories()
	{
		int headerSizeInBit = 0;
		for (int i = 0; i < m_nNumberOfElementsInAudioHeader; i++)
		{
			headerSizeInBit += HeaderBitmap[i];
		}

		m_nHeaderSizeInBit = headerSizeInBit;
		m_nHeaderSizeInByte = (headerSizeInBit + 7) / 8;
		m_nProcessingHeaderSizeInByte = m_nHeaderSizeInByte;
		memset(m_arrllInformation, 0, m_nNumberOfElementsInAudioHeader * sizeof(long long));
		memset(ma_uchHeader, 0, m_nHeaderSizeInByte);
	}

	int AudioPacketHeader::CopyInformationToHeader(unsigned int * Information)
	{
		memcpy(m_arrllInformation, Information, m_nHeaderSizeInByte);
		for (int i = 0; i < m_nNumberOfElementsInAudioHeader; i++)
		{
			SetInformation(Information[i], i);
		}
		return m_nHeaderSizeInByte;
	}

	long long AudioPacketHeader::GetFieldCapacity(int InfoType)
	{
		return 1LL << HeaderBitmap[InfoType];
	}


	int AudioPacketHeader::GetHeaderInByteArray(unsigned char* data)
	{
		memcpy(data, ma_uchHeader, m_nHeaderSizeInByte);
		return m_nHeaderSizeInByte;
	}

	int AudioPacketHeader::GetHeaderSize()
	{
		return m_nHeaderSizeInByte;
	}

	bool AudioPacketHeader::IsPacketTypeSupported(unsigned int PacketType)
	{
		int nPacketTypes = sizeof(SupportedPacketTypes) / sizeof(int);
		for (int i = 0; i < nPacketTypes; i++)
		{
			if (SupportedPacketTypes[i] == PacketType) return true;
		}
		return false;
	}

	bool AudioPacketHeader::IsPacketTypeSupported()
	{
		unsigned int iPackeType = GetInformation(INF_CALL_PACKETTYPE);
		return IsPacketTypeSupported(iPackeType);
	}

	void AudioPacketHeader::SetInformation(long long Information, int InfoType)
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

	void AudioPacketHeader::CopyHeaderToInformation(unsigned char *Header)
	{
		m_nProcessingHeaderSizeInByte = m_nHeaderSizeInByte;

		memcpy(ma_uchHeader, Header, m_nHeaderSizeInByte);

		for (int i = 0; i < m_nNumberOfElementsInAudioHeader; i++)
		{
			//CLogPrinter_WriteLog(CLogPrinter::INFO, INSTENT_TEST_LOG,"#PutInformationToArray#");
			PutInformationToArray(i);
			if (INF_CALL_HEADERLENGTH == i && m_nHeaderSizeInByte != m_arrllInformation[INF_CALL_HEADERLENGTH]) {

				m_nProcessingHeaderSizeInByte = m_arrllInformation[INF_CALL_HEADERLENGTH];
				HITLER("XXP@#@#MARUF H LEN UPDATED ..%u", m_nProcessingHeaderSizeInByte);
			}
		}
	}

	long long AudioPacketHeader::GetInformation(int InfoType)
	{
		return m_arrllInformation[InfoType];
	}

	bool AudioPacketHeader::PutInformationToArray(int InfoType)
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

	void AudioPacketHeader::ShowDetails(char prefix[])
	{
		string str = string(prefix) + " # ";
		for (int i = 0; i < m_nNumberOfElementsInAudioHeader; i++)
		{
			str += HeaderFieldNames[i];
			str += "=";
			str += Tools::getText(m_arrllInformation[i]);
			str += ", ";
		}
		MediaLog(LOG_DEBUG, "%s\n", str.c_str());
	}

} //namespace MediaSDK
