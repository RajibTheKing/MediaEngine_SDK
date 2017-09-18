#ifndef AUDIO_NEAR_END_PROCESSOR_CALL_H
#define AUDIO_NEAR_END_PROCESSOR_CALL_H


#include "AudioNearEndDataProcessor.h"


namespace MediaSDK
{

	class CAudioCallSession;
	class CAudioShortBuffer;


	class AudioNearEndProcessorCall : public AudioNearEndDataProcessor
	{

	public:

		AudioNearEndProcessorCall(int nServiceType, int nEntityType, CAudioCallSession *pAudioCallSession, SharedPointer<CAudioShortBuffer> pAudioEncodingBuffer, bool bIsLiveStreamingRunning, const bool &isVideoCallRunning);
		~AudioNearEndProcessorCall() { }

		void ProcessNearEndData();


	protected:

		void SentToNetwork(long long llRelativeTime);
		void DecideToChangeComplexity(int iEncodingTime);


	private:

		const bool &m_bIsVideoCallRunning;
		int m_nEncodedFrameSize;
	};

} //namespace MediaSDK

#endif  // !AUDIO_NEAR_END_PROCESSOR_CALL_H

