#ifndef LIVE_AUDIO_PARSER_FOR_CALLEE_H
#define LIVE_AUDIO_PARSER_FOR_CALLEE_H


#include "LiveAudioParser.h"
#include "SmartPointer.h"


namespace MediaSDK
{
	class AudioPacketHeader;
	class AudioSessionStatistics;
	class SessionStatisticsInterface;

	class CLiveAudioParserForCallee : public ILiveAudioParser
	{

	private:
		std::vector<LiveAudioDecodingQueue*> m_vAudioFarEndBufferVector;
		SharedPointer<CLockHandler> m_pLiveReceiverMutex;
		SharedPointer<AudioPacketHeader> m_pAudioPacketHeader;
		bool m_bIsCurrentlyParsingAudioData;
		bool m_bIsRoleChanging;
		long long m_llLastProcessedFrameNo;

		SessionStatisticsInterface *m_pSessionStatInterface = nullptr;

	public:

		CLiveAudioParserForCallee(std::vector<LiveAudioDecodingQueue*> vAudioFarEndBufferVector, SessionStatisticsInterface *pSessionStatInterface);
		virtual ~CLiveAudioParserForCallee();

		virtual void ProcessLiveAudio(int iId, int nOffset, unsigned char* uchAudioData, int nDataLength, int *pAudioFramsStartingByte, int nNumberOfAudioFrames, std::vector< std::pair<int, int> > vMissingBlocks);
		virtual void SetRoleChanging(bool bFlag);
		virtual bool GetRoleChanging();
		virtual bool IsParsingAudioData();
		virtual void GenMissingBlock(unsigned char* uchChunkData, int iFrameLeftRange, int iFrameRightRange, std::vector<std::pair<int, int>>&missingBlocks, std::vector<std::pair<int, int>>&vCurrentFrameMissingBlock);
	};

} //namespace MediaSDK


#endif