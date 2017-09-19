#include "AudioHeaderCommon.h"

#include "MediaLogger.h"
#include "LogPrinter.h"
#include "Tools.h"

namespace MediaSDK
{

	AudioHeaderCommon::AudioHeaderCommon()
	{
		MR_DEBUG("#resorce#header# AudioHeaderCommon::AudioHeaderCommon() - 1");

		InitHeaderBitMap();

		int headerSizeInBit = 0;
		for (int i = 0; i < m_nNumberOfElementsInAudioHeader; i++)
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
		HeaderBitmap[INF_ECHO_STATE_FLAGS] = 10;
		HeaderBitmap[INF_UNUSED_1] = 1;
		HeaderBitmap[INF_CHANNELS] = 2;
		HeaderBitmap[INF_UNUSED_2] = 3;
		HeaderBitmap[INF_TIMESTAMP] = 40;
		HeaderBitmap[INF_PACKET_BLOCK_NUMBER] = 4;
		HeaderBitmap[INF_TOTAL_PACKET_BLOCKS] = 4;
		HeaderBitmap[INF_BLOCK_OFFSET] = 16;
		HeaderBitmap[INF_FRAME_LENGTH] = 16;
		


		HeaderFieldNames[INF_PACKETTYPE] = STRING(INF_PACKETTYPE);
		HeaderFieldNames[INF_HEADERLENGTH] = STRING(INF_HEADERLENGTH);
		HeaderFieldNames[INF_NETWORKTYPE] = STRING(INF_NETWORKTYPE);
		HeaderFieldNames[INF_VERSIONCODE] = STRING(INF_VERSIONCODE);
		HeaderFieldNames[INF_PACKETNUMBER] = STRING(INF_PACKETNUMBER);
		HeaderFieldNames[INF_BLOCK_LENGTH] = STRING(INF_BLOCK_LENGTH);
		HeaderFieldNames[INF_ECHO_STATE_FLAGS] = STRING(INF_ECHO_STATE_FLAGS);
		HeaderFieldNames[INF_UNUSED_1] = STRING(INF_UNUSED_1);
		HeaderFieldNames[INF_CHANNELS] = STRING(INF_CHANNELS);
		HeaderFieldNames[INF_UNUSED_2] = STRING(INF_UNUSED_2);
		HeaderFieldNames[INF_TIMESTAMP] = STRING(INF_TIMESTAMP);
		HeaderFieldNames[INF_PACKET_BLOCK_NUMBER] = STRING(INF_PACKET_BLOCK_NUMBER);
		HeaderFieldNames[INF_TOTAL_PACKET_BLOCKS] = STRING(INF_TOTAL_PACKET_BLOCKS);
		HeaderFieldNames[INF_BLOCK_OFFSET] = STRING(INF_BLOCK_OFFSET);
		HeaderFieldNames[INF_FRAME_LENGTH] = STRING(INF_FRAME_LENGTH);

		m_nNumberOfElementsInAudioHeader = NUMBER_OF_FIELDS_IN_AUDIO_HEADER;
		

	}


} //namespace MediaSDK
