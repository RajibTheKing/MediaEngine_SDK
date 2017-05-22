#ifndef AUDIO_NEAR_END_PROCEDURE_PUBLISHER_H
#define AUDIO_NEAR_END_PROCEDURE_PUBLISHER_H


#include "AudioNearEndDataProcessor.h"

namespace MediaSDK
{

	class CAudioCallSession;
	class CAudioShortBuffer;


	class AudioNearEndProcessorPublisher : public AudioNearEndDataProcessor
	{
		CAudioShortBuffer *m_pAudioNearEndBuffer;


	public:

		AudioNearEndProcessorPublisher(int nServiceType, int nEntityType, CAudioCallSession *pAudioCallSession, CAudioShortBuffer *pAudioEncodingBuffer, bool bIsLiveStreamingRunning);
		~AudioNearEndProcessorPublisher() { }

		void ProcessNearEndData();
	};

} //namespace MediaSDK

#endif  // !AUDIO_NEAR_END_PROCEDURE_PUBLISHER_H
