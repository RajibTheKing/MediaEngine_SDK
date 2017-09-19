
#include "AudioHeaderLive.h"
#include "MediaLogger.h"
#include "LogPrinter.h"
#include "Tools.h"

namespace MediaSDK
{

	AudioHeaderLive::AudioHeaderLive()
	{
		MR_DEBUG("#resorce#header# AudioHeaderCall::AudioHeaderCall() - 1");

		m_nNumberOfElementsInAudioHeader = NUMBER_OF_FIELDS_IN_AUDIO_HEADER_LIVE;
		InitArrays();
		InitHeaderBitMap();
		ClearMemories();
	}

	AudioHeaderLive::AudioHeaderLive(unsigned int * Information)
	{
		MR_DEBUG("#resorce#header# AudioHeaderCall::AudioHeaderCall() - 2");

		AudioHeaderLive();
		CopyInformationToHeader(Information);
	}

	AudioHeaderLive::AudioHeaderLive(unsigned char *Header)
	{
		MR_DEBUG("#resorce#header# AudioHeaderCall::AudioHeaderCall() - 3");

		AudioHeaderLive();

		CopyHeaderToInformation(Header);
	}


	void AudioHeaderLive::InitHeaderBitMap()
	{
		HeaderBitmap[INF_LIVE_PACKETTYPE] = 8;
		HeaderBitmap[INF_LIVE_HEADERLENGTH] = 6;
		HeaderBitmap[INF_LIVE_NETWORKTYPE] = 2;
		HeaderBitmap[INF_LIVE_VERSIONCODE] = 5;
		HeaderBitmap[INF_LIVE_PACKETNUMBER] = 31;
		HeaderBitmap[INF_LIVE_BLOCK_LENGTH] = 12;
		HeaderBitmap[INF_LIVE_ECHO_STATE_FLAGS] = 10;
		HeaderBitmap[INF_LIVE_UNUSED_1] = 1;
		HeaderBitmap[INF_LIVE_CHANNELS] = 2;
		HeaderBitmap[INF_LIVE_UNUSED_2] = 3;
		HeaderBitmap[INF_LIVE_TIMESTAMP] = 40;
		HeaderBitmap[INF_LIVE_PACKET_BLOCK_NUMBER] = 4;
		HeaderBitmap[INF_LIVE_TOTAL_PACKET_BLOCKS] = 4;
		HeaderBitmap[INF_LIVE_BLOCK_OFFSET] = 16;
		HeaderBitmap[INF_LIVE_FRAME_LENGTH] = 16;
		


		HeaderFieldNames[INF_LIVE_PACKETTYPE] = STRING(INF_LIVE_PACKETTYPE);
		HeaderFieldNames[INF_LIVE_HEADERLENGTH] = STRING(INF_LIVE_HEADERLENGTH);
		HeaderFieldNames[INF_LIVE_NETWORKTYPE] = STRING(INF_LIVE_NETWORKTYPE);
		HeaderFieldNames[INF_LIVE_VERSIONCODE] = STRING(INF_LIVE_VERSIONCODE);
		HeaderFieldNames[INF_LIVE_PACKETNUMBER] = STRING(INF_LIVE_PACKETNUMBER);
		HeaderFieldNames[INF_LIVE_BLOCK_LENGTH] = STRING(INF_LIVE_BLOCK_LENGTH);
		HeaderFieldNames[INF_LIVE_ECHO_STATE_FLAGS] = STRING(INF_LIVE_ECHO_STATE_FLAGS);
		HeaderFieldNames[INF_LIVE_UNUSED_1] = STRING(INF_LIVE_UNUSED_1);
		HeaderFieldNames[INF_LIVE_CHANNELS] = STRING(INF_LIVE_CHANNELS);
		HeaderFieldNames[INF_LIVE_UNUSED_2] = STRING(INF_LIVE_UNUSED_2);
		HeaderFieldNames[INF_LIVE_TIMESTAMP] = STRING(INF_LIVE_TIMESTAMP);
		HeaderFieldNames[INF_LIVE_PACKET_BLOCK_NUMBER] = STRING(INF_LIVE_PACKET_BLOCK_NUMBER);
		HeaderFieldNames[INF_LIVE_TOTAL_PACKET_BLOCKS] = STRING(INF_LIVE_TOTAL_PACKET_BLOCKS);
		HeaderFieldNames[INF_LIVE_BLOCK_OFFSET] = STRING(INF_LIVE_BLOCK_OFFSET);
		HeaderFieldNames[INF_LIVE_FRAME_LENGTH] = STRING(INF_LIVE_FRAME_LENGTH);

	}


} //namespace MediaSDK
