#ifndef AUDIO_FAR_END_PROCESSOR_CALL_H
#define AUDIO_FAR_END_PROCESSOR_CALL_H


#include "AudioFarEndDataProcessor.h"


class FarEndProcessorCall : public AudioFarEndDataProcessor
{


public:

	FarEndProcessorCall(long long llFriendID, int nServiceType, int nEntityType, CAudioCallSession *pAudioCallSession, CCommonElementsBucket* pCommonElementsBucket, bool bIsLiveStreamingRunning);
	~FarEndProcessorCall() { }

	void ProcessFarEndData();
};



#endif  // !AUDIO_FAR_END_PROCESSOR_CALL_H
