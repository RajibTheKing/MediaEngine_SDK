#ifndef AUDIO_FAR_END_PROCESSOR_VIEWER_H 
#define AUDIO_FAR_END_PROCESSOR_VIEWER_H


#include "AudioFarEndDataProcessor.h"

namespace MediaSDK
{

	class FarEndProcessorViewer : public AudioFarEndDataProcessor
	{

	public:

		FarEndProcessorViewer(long long llFriendID, int nServiceType, int nEntityType, CAudioCallSession *pAudioCallSession, CCommonElementsBucket* pCommonElementsBucket, bool bIsLiveStreamingRunning);
		~FarEndProcessorViewer() { }

		void ProcessFarEndData();

	};

} //namespace MediaSDK

#endif  // !AUDIO_FAR_END_PROCESSOR_VIEWER_H

