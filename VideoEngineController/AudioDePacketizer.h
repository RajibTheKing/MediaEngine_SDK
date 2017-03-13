#ifndef _AUDIO_DE_PACKETIZER_H_
#define _AUDIO_DE_PACKETIZER_H_

#include "AudioPacketHeader.h"
#include "AudioMacros.h"

class AudioDePacketizer
{
public:
	AudioDePacketizer();
	~AudioDePacketizer();

private:
	bool dePacketize(unsigned char* uchBlock);
	
	int m_iBlockOkayFlag;
	int m_iAudioHeaderLength;
	int m_iPreviousPacketNumber;

	CAudioPacketHeader m_pAudioPacketHeader;

	unsigned char m_uchAudioStorageBuffer[MAX_AUDIO_FRAME_Length];
};

#endif