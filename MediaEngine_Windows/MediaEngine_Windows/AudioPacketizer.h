#include "AudioMacros.h"

class CAudioCallSession;
class CCommonElementsBucket;
class CAudioPacketHeader;

class AudioPacketizer
{
public:
	AudioPacketizer(CAudioCallSession* audioCallSession, CCommonElementsBucket* pCommonElementsBucket);
	~AudioPacketizer();
	void Packetize(bool bShouldPacketize, unsigned char* uchData, int nDataLength, int nFrameNumber, int packetType, int networkType, int version, long long llRelativeTime, int channel,
		int iPrevRecvdSlotID, int nReceivedPacketsInPrevSlot, long long llFriendID);
private:
	CAudioCallSession* m_pAudioCallSession;
	CAudioPacketHeader m_AudioPacketHeader;
	CCommonElementsBucket* m_pCommonElementsBucket;

	int m_nHeaderLengthWithMediaByte, m_nMaxDataSyzeInEachBlock, m_nHeaderLength;

	unsigned char m_uchAudioBlock[MAX_AUDIO_PACKET_SIZE + 10];
};
