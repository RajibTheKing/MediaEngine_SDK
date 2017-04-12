#ifndef LIVE_AUDIO_PARSER_H
#define LIVE_AUDIO_PARSER_H

#include <vector>

#include "LiveAudioDecodingQueue.h"

class ILiveAudioParser{
public:	
	virtual void ProcessLiveAudio(int iId, int nOffset, unsigned char* uchAudioData, int nDataLength, int *pAudioFramsStartingByte, int nNumberOfAudioFrames, std::vector< std::pair<int, int> > vMissingBlocks) = 0;
	virtual void SetRoleChanging(bool bFlag) = 0;
	virtual bool GetRoleChanging() = 0;
	virtual bool IsParsingAudioData() = 0;
	virtual void GenMissingBlock(unsigned char* uchChunkData, int iFrameLeftRange, int iFrameRightRange, std::vector<std::pair<int, int>>&missingBlocks, std::vector<std::pair<int, int>>&vCurrentFrameMissingBlock);
};

#endif