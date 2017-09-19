#ifndef AUDIO_PACKET_HEADER_H
#define AUDIO_PACKET_HEADER_H


#include "AudioTypes.h"
#include "SmartPointer.h"
using namespace std;


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
		
		NUMBER_OF_FIELDS_IN_AUDIO_HEADER
	};

	enum AudioPacketTypes
	{
		AUDIO_SKIP_PACKET_TYPE = 0,
		AUDIO_NORMAL_PACKET_TYPE = 1,
		AUDIO_NOVIDEO_PACKET_TYPE = 2,
		AUDIO_VERSION_PACKET_TYPE = 3,
		AUDIO_CHANNEL_PACKET_TYPE = 7,
		AUDIO_OPUS_PACKET_TYPE = 9,
		AUDIO_LIVE_CALLEE_PACKET_TYPE = 10,
		AUDIO_LIVE_PUBLISHER_PACKET_TYPE_MUXED = 11,
		AUDIO_LIVE_PUBLISHER_PACKET_TYPE_NONMUXED = 12,
		LIVE_CALLEE_PACKET_TYPE_OPUS = 13,
		LIVE_PUBLISHER_PACKET_TYPE_OPUS = 14
	};

	static int SupportedPacketTypes[] =
	{
		AUDIO_SKIP_PACKET_TYPE,
		AUDIO_NORMAL_PACKET_TYPE,
		AUDIO_NOVIDEO_PACKET_TYPE,
		AUDIO_VERSION_PACKET_TYPE
	}; //Only for Call


	enum MaxSizes{
		MAXHEADERSIZE = 100
	};

	class AudioPacketHeader
	{
	public:

		//Factory Method to get the header instance

		static SharedPointer<AudioPacketHeader> GetInstance(AudioHeaderTypes type);

		int m_nNumberOfElementsInAudioHeader;

		//Constructor
		AudioPacketHeader(){}
		virtual ~AudioPacketHeader(){};

		void CopyHeaderToInformation(unsigned char *Header);
		int GetHeaderInByteArray(unsigned char* data);

		void SetInformation(long long Information, int InfoType);
		long long GetInformation(int InfoType);

		long long GetFieldCapacity(int InfoType);

		bool IsPacketTypeSupported(unsigned int PacketType);
		bool IsPacketTypeSupported();

		void ShowDetails(char prefix[]);

		bool PutInformationToArray(int InfoType);

		int GetHeaderSize();

	protected:
		int CopyInformationToHeader(unsigned int * Information);

	protected:
		int HeaderBitmap[NUMBER_OF_FIELDS_IN_AUDIO_HEADER];
		string HeaderFieldNames[NUMBER_OF_FIELDS_IN_AUDIO_HEADER];

		unsigned int m_nHeaderSizeInBit;
		unsigned int m_nHeaderSizeInByte;
		unsigned int m_nProcessingHeaderSizeInByte;

		long long m_arrllInformation[NUMBER_OF_FIELDS_IN_AUDIO_HEADER];
		unsigned char ma_uchHeader[MAXHEADERSIZE];

	};

} //namespace MediaSDK

#endif