#ifndef AUDIO_FAR_END_PROCESSOR_CALL_H
#define AUDIO_FAR_END_PROCESSOR_CALL_H


#include "AudioFarEndDataProcessor.h"


namespace MediaSDK
{

	class FarEndProcessorCall : public AudioFarEndDataProcessor
	{
		
	public:

		FarEndProcessorCall(int nAudioFlowType, int nEntityType, CAudioCallSession *pAudioCallSession, bool bIsLiveStreamingRunning);
		~FarEndProcessorCall() { }
		bool m_bProcessFarendDataStarted;

		void ProcessFarEndData();

	private:

		void DequeueData(int &m_nDecodingFrameSize);
		bool IsPacketProcessableInNormalCall(int &nCurrentAudioPacketType, int &nVersion);


	private:
		bool IsQueueEmpty();
		int m_iAudioVersionFriend = -1;
		int m_iLastFarEndFrameNumber;
	};

} //namespace MediaSDK

#endif