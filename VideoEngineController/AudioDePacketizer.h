#ifndef _AUDIO_DE_PACKETIZER_H_
#define _AUDIO_DE_PACKETIZER_H_

#include "AudioMacros.h"

class CAudioPacketHeader;
class CAudioCallSession;

class AudioDePacketizer
{
public:
	AudioDePacketizer(CAudioCallSession * pAudioCallSession);
	~AudioDePacketizer();
	bool dePacketize(unsigned char* uchBlock, int iBlockNo, int iTotalBlock, int iBlockLength, int iBlockOffset, int iPacketNumber, int nPacketLength);
	int GetCompleteFrame(unsigned char* uchFrame);
private:
	int m_iBlockOkayFlag;
	int m_iAudioHeaderLength;
	int m_iPreviousPacketNumber;
	int m_nFrameLength;
	CAudioPacketHeader* m_pAudioPacketHeader;
	CAudioCallSession* m_pAudioCallSession;
	void SentIncompleteFrame(int iLastPacketNumber);

	unsigned char m_uchAudioStorageBuffer[MAX_AUDIO_DECODER_FRAME_SIZE];
	short m_saDataToPlay[MAX_AUDIO_DECODER_FRAME_SIZE];
};

#endif