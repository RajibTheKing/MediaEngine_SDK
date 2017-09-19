#ifndef AUDIO_HEADER_COMMON_H
#define AUDIO_HEADER_COMMON_H


#include "AudioPacketHeader.h"


namespace MediaSDK
{
	enum AudioHeaderInfoTypes
	{
		INF_CALL_PACKETTYPE = 0,
		INF_CALL_HEADERLENGTH,
		INF_CALL_NETWORKTYPE,
		INF_CALL_VERSIONCODE,
		INF_CALL_PACKETNUMBER,
		INF_CALL_BLOCK_LENGTH,
		INF_CALL_ECHO_STATE_FLAGS,
		INF_CALL_UNUSED_1,
		INF_CALL_CHANNELS,
		INF_CALL_UNUSED_2,
		INF_CALL_TIMESTAMP,
		INF_CALL_PACKET_BLOCK_NUMBER,
		INF_CALL_TOTAL_PACKET_BLOCKS,
		INF_CALL_BLOCK_OFFSET,
		INF_CALL_FRAME_LENGTH,

		NUMBER_OF_FIELDS_IN_AUDIO_HEADER_CALL
	};

	class AudioHeaderCall : public AudioPacketHeader
	{
	public:

		AudioHeaderCall();
		AudioHeaderCall(unsigned int * Information);
		AudioHeaderCall(unsigned char *Header);

	private:
		void InitHeaderBitMap();

	};

} //namespace MediaSDK

#endif

