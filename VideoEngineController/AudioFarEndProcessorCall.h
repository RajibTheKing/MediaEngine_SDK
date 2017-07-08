#ifndef AUDIO_FAR_END_PROCESSOR_CALL_H
#define AUDIO_FAR_END_PROCESSOR_CALL_H


#include "AudioFarEndDataProcessor.h"


namespace MediaSDK
{

	class FarEndProcessorCall : public AudioFarEndDataProcessor
	{
		
	public:

		FarEndProcessorCall(int nServiceType, int nEntityType, CAudioCallSession *pAudioCallSession, bool bIsLiveStreamingRunning);
		~FarEndProcessorCall() { }
		bool m_bProcessFarendDataStarted;

		void ProcessFarEndData();
	};

} //namespace MediaSDK


#endif  // !AUDIO_FAR_END_PROCESSOR_CALL_H
