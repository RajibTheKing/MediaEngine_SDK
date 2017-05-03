#ifndef AUDIO_FAR_END_PROCESSOR_PUBLISHER_H
#define AUDIO_FAR_END_PROCESSOR_PUBLISHER_H


#include "AudioFarEndDataProcessor.h"


class FarEndProcessorPublisher : public AudioFarEndDataProcessor
{

public:

	FarEndProcessorPublisher(long long llFriendID, int nServiceType, int nEntityType, CAudioCallSession *pAudioCallSession, CCommonElementsBucket* pCommonElementsBucket, bool bIsLiveStreamingRunning);
	~FarEndProcessorPublisher() { }

	void ProcessFarEndData();
};




#endif // !AUDIO_FAR_END_PROCESSOR_PUBLISHER_H
