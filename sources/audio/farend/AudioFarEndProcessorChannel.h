#ifndef AUDIO_FAR_END_PROCESSOR_CHANNEL_H
#define AUDIO_FAR_END_PROCESSOR_CHANNEL_H


#include "AudioFarEndDataProcessor.h"


namespace MediaSDK
{

	class FarEndProcessorChannel : public AudioFarEndDataProcessor
	{

	public:

		FarEndProcessorChannel(int nServiceType, int nEntityType, CAudioCallSession *pAudioCallSession, bool bIsLiveStreamingRunning);
		~FarEndProcessorChannel() { }

		void ProcessFarEndData();
	
	private: 
		int m_iLastFarEndFrameNumber;
	};

} //namespace MediaSDK


#endif  // !AUDIO_FAR_END_PROCESSOR_CHANNEL_H
