
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
		HeaderBitmap[INF_LIVE_VERSIONCODE] = 5;
		HeaderBitmap[INF_LIVE_PACKETNUMBER] = 31;
		HeaderBitmap[INF_LIVE_FRAME_LENGTH] = 12;
		HeaderBitmap[INF_LIVE_TIMESTAMP] = 40;
		HeaderBitmap[INF_LIVE_ECHO_STATE_FLAGS] = 10;
		
		HeaderFieldNames[INF_LIVE_PACKETTYPE] = STRING(INF_LIVE_PACKETTYPE);
		HeaderFieldNames[INF_LIVE_HEADERLENGTH] = STRING(INF_LIVE_HEADERLENGTH);
		HeaderFieldNames[INF_LIVE_VERSIONCODE] = STRING(INF_LIVE_VERSIONCODE);
		HeaderFieldNames[INF_LIVE_PACKETNUMBER] = STRING(INF_LIVE_PACKETNUMBER);
		HeaderFieldNames[INF_LIVE_FRAME_LENGTH] = STRING(INF_LIVE_FRAME_LENGTH);
		HeaderFieldNames[INF_LIVE_TIMESTAMP] = STRING(INF_LIVE_TIMESTAMP);
		HeaderFieldNames[INF_LIVE_ECHO_STATE_FLAGS] = STRING(INF_LIVE_ECHO_STATE_FLAGS);
	}


} //namespace MediaSDK
