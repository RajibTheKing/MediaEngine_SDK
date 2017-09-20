#include "AudioPacketHeader.h"
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
		InitArrays();
		InitHeaderBitMap();
		ClearMemories();
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
