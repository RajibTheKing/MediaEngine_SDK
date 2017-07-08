#ifndef AUDIO_NEAR_END_PROCESSOR_VIEWER_H
#define AUDIO_NEAR_END_PROCESSOR_VIEWER_H


#include "AudioNearEndDataProcessor.h"


namespace MediaSDK
{

	class CAudioCallSession;
	class CAudioShortBuffer;


	class AudioNearEndProcessorViewer : public AudioNearEndDataProcessor
	{

		SmartPointer<CAudioShortBuffer> m_pAudioNearEndBuffer;
		CAudioCallSession *m_pAudioCallSession;


	public:

		AudioNearEndProcessorViewer(int nServiceType, int nEntityType, CAudioCallSession *pAudioCallSession, SmartPointer<CAudioShortBuffer> pAudioEncodingBuffer, bool bIsLiveStreamingRunning);
		~AudioNearEndProcessorViewer() { }

		void ProcessNearEndData();

	};

}  //namespace MediaSDK


#endif  // !AUDIO_NEAR_END_PROCESSOR_VIEWER_IN_CALL_H

