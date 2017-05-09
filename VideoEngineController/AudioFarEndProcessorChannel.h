#ifndef AUDIO_FAR_END_PROCESSOR_CHANNEL_H
#define AUDIO_FAR_END_PROCESSOR_CHANNEL_H

#include "AudioFarEndDataProcessor.h"

namespace MediaSDK
{

	class FarEndProcessorChannel : public AudioFarEndDataProcessor
	{

	public:

		FarEndProcessorChannel(long long llFriendID, int nServiceType, int nEntityType, CAudioCallSession *pAudioCallSession, CCommonElementsBucket* pCommonElementsBucket, bool bIsLiveStreamingRunning);
		~FarEndProcessorChannel() { }

		void ProcessFarEndData();

	};

} //namespace MediaSDK


#endif  // !AUDIO_FAR_END_PROCESSOR_CHANNEL_H
