
#include "AudioHeaderCall.h"
#include "MediaLogger.h"
#include "LogPrinter.h"
#include "Tools.h"

namespace MediaSDK
{

	AudioHeaderCall::AudioHeaderCall()
	{
		MR_DEBUG("#resorce#header# AudioHeaderCall::AudioHeaderCall() - 1");

		m_nNumberOfElementsInAudioHeader = NUMBER_OF_FIELDS_IN_AUDIO_HEADER_CALL;
		HeaderBitmap = new int[m_nNumberOfElementsInAudioHeader];
		HeaderFieldNames = new string[m_nNumberOfElementsInAudioHeader];
		m_arrllInformation = new long long[m_nNumberOfElementsInAudioHeader];

		InitHeaderBitMap();

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

	AudioHeaderCall::AudioHeaderCall(unsigned int * Information)
	{
		MR_DEBUG("#resorce#header# AudioHeaderCall::AudioHeaderCall() - 2");

		AudioHeaderCall();
		CopyInformationToHeader(Information);
	}

	AudioHeaderCall::AudioHeaderCall(unsigned char *Header)
	{
		MR_DEBUG("#resorce#header# AudioHeaderCall::AudioHeaderCall() - 3");

		AudioHeaderCall();

		CopyHeaderToInformation(Header);
	}

	AudioHeaderCall::~AudioHeaderCall()
	{
		MR_DEBUG("#resorce#header# AudioHeaderCall::~AudioHeaderCall()");

		memset(ma_uchHeader, 0, m_nHeaderSizeInByte);
		delete[] HeaderBitmap;
		delete[] HeaderFieldNames;
		delete[] m_arrllInformation;
	}

	void AudioHeaderCall::InitHeaderBitMap()
	{
		HeaderBitmap[INF_CALL_PACKETTYPE] = 8;
		HeaderBitmap[INF_CALL_HEADERLENGTH] = 6;
		HeaderBitmap[INF_CALL_NETWORKTYPE] = 2;
		HeaderBitmap[INF_CALL_VERSIONCODE] = 5;
		HeaderBitmap[INF_CALL_PACKETNUMBER] = 31;
		HeaderBitmap[INF_CALL_BLOCK_LENGTH] = 12;
		HeaderBitmap[INF_CALL_ECHO_STATE_FLAGS] = 10;
		HeaderBitmap[INF_CALL_UNUSED_1] = 1;
		HeaderBitmap[INF_CALL_CHANNELS] = 2;
		HeaderBitmap[INF_CALL_UNUSED_2] = 3;
		HeaderBitmap[INF_CALL_TIMESTAMP] = 40;
		HeaderBitmap[INF_CALL_PACKET_BLOCK_NUMBER] = 4;
		HeaderBitmap[INF_CALL_TOTAL_PACKET_BLOCKS] = 4;
		HeaderBitmap[INF_CALL_BLOCK_OFFSET] = 16;
		HeaderBitmap[INF_CALL_FRAME_LENGTH] = 16;
		


		HeaderFieldNames[INF_CALL_PACKETTYPE] = STRING(INF_CALL_PACKETTYPE);
		HeaderFieldNames[INF_CALL_HEADERLENGTH] = STRING(INF_CALL_HEADERLENGTH);
		HeaderFieldNames[INF_CALL_NETWORKTYPE] = STRING(INF_CALL_NETWORKTYPE);
		HeaderFieldNames[INF_CALL_VERSIONCODE] = STRING(INF_CALL_VERSIONCODE);
		HeaderFieldNames[INF_CALL_PACKETNUMBER] = STRING(INF_CALL_PACKETNUMBER);
		HeaderFieldNames[INF_CALL_BLOCK_LENGTH] = STRING(INF_CALL_BLOCK_LENGTH);
		HeaderFieldNames[INF_CALL_ECHO_STATE_FLAGS] = STRING(INF_CALL_ECHO_STATE_FLAGS);
		HeaderFieldNames[INF_CALL_UNUSED_1] = STRING(INF_CALL_UNUSED_1);
		HeaderFieldNames[INF_CALL_CHANNELS] = STRING(INF_CALL_CHANNELS);
		HeaderFieldNames[INF_CALL_UNUSED_2] = STRING(INF_CALL_UNUSED_2);
		HeaderFieldNames[INF_CALL_TIMESTAMP] = STRING(INF_CALL_TIMESTAMP);
		HeaderFieldNames[INF_CALL_PACKET_BLOCK_NUMBER] = STRING(INF_CALL_PACKET_BLOCK_NUMBER);
		HeaderFieldNames[INF_CALL_TOTAL_PACKET_BLOCKS] = STRING(INF_CALL_TOTAL_PACKET_BLOCKS);
		HeaderFieldNames[INF_CALL_BLOCK_OFFSET] = STRING(INF_CALL_BLOCK_OFFSET);
		HeaderFieldNames[INF_CALL_FRAME_LENGTH] = STRING(INF_CALL_FRAME_LENGTH);

		
		

	}


} //namespace MediaSDK
