#ifndef LIVE_AUDIO_PARSER_FOR_PUBLISHER_H
#define LIVE_AUDIO_PARSER_FOR_PUBLISHER_H

#include "LiveAudioParser.h"
#include "SmartPointer.h"


namespace MediaSDK
{
	class AudioPacketHeader;	
	class CAudioCallSession;
	class SessionStatisticsInterface;

	class CLiveAudioParserForPublisher : public ILiveAudioParser
	{

	private:
		std::vector<LiveAudioDecodingQueue*> m_vAudioFarEndBufferVector;
		SharedPointer<CLockHandler> m_pLiveReceiverMutex;
		SharedPointer<AudioPacketHeader> m_pAudioPacketHeader;
		bool m_bIsCurrentlyParsingAudioData;
		bool m_bIsRoleChanging;

		SessionStatisticsInterface *m_pSessionStatInterface = nullptr;

	public:
		CLiveAudioParserForPublisher(std::vector<LiveAudioDecodingQueue*> vAudioFarEndBufferVector, SessionStatisticsInterface *pAudioCallSession);
		virtual ~CLiveAudioParserForPublisher();

		virtual void ProcessLiveAudio(int iId, int nOffset, unsigned char* uchAudioData, int nDataLength, int *pAudioFramsStartingByte, int nNumberOfAudioFrames, std::vector< std::pair<int, int> > vMissingBlocks);
		virtual void SetRoleChanging(bool bFlag);
		virtual bool GetRoleChanging();
		virtual bool IsParsingAudioData();
		virtual void GenMissingBlock(unsigned char* uchAudioData, int nFrameLeftRange, int nFrameRightRange, std::vector<std::pair<int, int>>&vMissingBlocks, std::vector<std::pair<int, int>>&vCurrentFrameMissingBlock);
	};

} //namespace MediaSDK

#endif