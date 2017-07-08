#ifndef AUDIO_NEAR_END_PROCESSOR_CALL_H
#define AUDIO_NEAR_END_PROCESSOR_CALL_H


#include "AudioNearEndDataProcessor.h"


namespace MediaSDK
{

	class CAudioCallSession;
	class CAudioShortBuffer;


	class AudioNearEndProcessorCall : public AudioNearEndDataProcessor
	{

		SmartPointer<CAudioShortBuffer> m_pAudioNearEndBuffer;


	public:

		AudioNearEndProcessorCall(int nServiceType, int nEntityType, CAudioCallSession *pAudioCallSession, SmartPointer<CAudioShortBuffer> pAudioEncodingBuffer, bool bIsLiveStreamingRunning);
		~AudioNearEndProcessorCall() { }

		void ProcessNearEndData();

	};

} //namespace MediaSDK

#endif  // !AUDIO_NEAR_END_PROCESSOR_CALL_H

