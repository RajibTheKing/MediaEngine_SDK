#ifndef AUDIO_FAR_END_PROCESSOR_PUBLISHER_H
#define AUDIO_FAR_END_PROCESSOR_PUBLISHER_H


#include "AudioFarEndDataProcessor.h"


namespace MediaSDK
{

	class FarEndProcessorPublisher : public AudioFarEndDataProcessor
	{

	public:

		FarEndProcessorPublisher(int nAudioFlowType, int nEntityType, CAudioCallSession *pAudioCallSession, bool bIsLiveStreamingRunning);
		~FarEndProcessorPublisher() { }

		void ProcessFarEndData();
	};


} //namespace MediaSDK


#endif // !AUDIO_FAR_END_PROCESSOR_PUBLISHER_H
