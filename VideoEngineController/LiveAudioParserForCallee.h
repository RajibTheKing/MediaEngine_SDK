#ifndef LIVE_AUDIO_PARSER_FOR_CALLEE_H
#define LIVE_AUDIO_PARSER_FOR_CALLEE_H

#include "LiveAudioParser.h"
#include "SmartPointer.h"
#include "LockHandler.h"

class CAudioPacketHeader;

class CLiveAudioParserForCallee : public ILiveAudioParser{
private:
	std::vector<LiveAudioDecodingQueue*> m_vAudioFarEndBufferVector;
	SmartPointer<CLockHandler> m_pLiveReceiverMutex;
	CAudioPacketHeader *m_pAudioPacketHeader;

public:

	CLiveAudioParserForCallee(std::vector<LiveAudioDecodingQueue*> vAudioFarEndBufferVector);

	virtual ~CLiveAudioParserForCallee();

	virtual void ProcessLiveAudio(int iId, int nOffset, unsigned char* uchAudioData, int nDataLength, int *pAudioFramsStartingByte, int nNumberOfAudioFrames, std::vector< std::pair<int, int> > vMissingBlocks);
};

#endif