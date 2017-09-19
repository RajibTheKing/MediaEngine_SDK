#ifndef AUDIO_HEADER_COMMON_H
#define AUDIO_HEADER_COMMON_H


#include "AudioPacketHeader.h"


namespace MediaSDK
{
	enum AudioHeaderInfoTypes
	{
		INF_LIVE_PACKETTYPE = 0,
		INF_LIVE_HEADERLENGTH,
		INF_LIVE_NETWORKTYPE,
		INF_LIVE_VERSIONCODE,
		INF_LIVE_PACKETNUMBER,
		INF_LIVE_BLOCK_LENGTH,
		INF_LIVE_ECHO_STATE_FLAGS,
		INF_LIVE_UNUSED_1,
		INF_LIVE_CHANNELS,
		INF_LIVE_UNUSED_2,
		INF_LIVE_TIMESTAMP,
		INF_LIVE_PACKET_BLOCK_NUMBER,
		INF_LIVE_TOTAL_PACKET_BLOCKS,
		INF_LIVE_BLOCK_OFFSET,
		INF_LIVE_FRAME_LENGTH,

		NUMBER_OF_FIELDS_IN_AUDIO_HEADER_LIVE
	};

	class AudioHeaderLive : public AudioPacketHeader
	{
	public:

		AudioHeaderLive();
		AudioHeaderLive(unsigned int * Information);
		AudioHeaderLive(unsigned char *Header);

	private:
		void InitHeaderBitMap();

	};

} //namespace MediaSDK

#endif

