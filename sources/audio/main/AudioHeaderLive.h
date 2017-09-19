#ifndef AUDIO_HEADER_LIVE_H
#define AUDIO_HEADER_LIVE_H


namespace MediaSDK
{
	class AudioPacketHeader;

	enum AudioHeaderInfoTypesLive
	{
		INF_LIVE_PACKETTYPE = 0,
		INF_LIVE_HEADERLENGTH,		
		INF_LIVE_VERSIONCODE,
		INF_LIVE_PACKETNUMBER,
		INF_LIVE_FRAME_LENGTH,
		INF_LIVE_TIMESTAMP,
		INF_LIVE_ECHO_STATE_FLAGS,		

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

