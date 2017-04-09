#ifndef LIVE_AUDIO_PARSER_H
#define LIVE_AUDIO_PARSER_H

#include <vector>

#include "LiveAudioDecodingQueue.h"

class ILiveAudioParser{
public:
	
	bool m_bIsCurrentlyParsingAudioData;
	bool m_bIsRoleChanging;

public:	
	virtual void ProcessLiveAudio(int iId, int nOffset, unsigned char* uchAudioData, int nDataLength, int *pAudioFramsStartingByte, int nNumberOfAudioFrames, std::vector< std::pair<int, int> > vMissingBlocks) = 0;
};

#endif