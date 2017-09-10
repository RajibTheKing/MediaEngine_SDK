#ifndef AUDIO_NEAR_END_PROCESSOR_VIEWER_H
#define AUDIO_NEAR_END_PROCESSOR_VIEWER_H


#include "AudioNearEndDataProcessor.h"


namespace MediaSDK
{

	class CAudioCallSession;
	class CAudioShortBuffer;


	class AudioNearEndProcessorViewer : public AudioNearEndDataProcessor
	{

	public:

		AudioNearEndProcessorViewer(int nServiceType, int nEntityType, CAudioCallSession *pAudioCallSession, SharedPointer<CAudioShortBuffer> pAudioEncodingBuffer, bool bIsLiveStreamingRunning);
		~AudioNearEndProcessorViewer() { }

		void ProcessNearEndData();


	private:

		int m_nRawFrameSize;
	};

}  //namespace MediaSDK


#endif  // !AUDIO_NEAR_END_PROCESSOR_VIEWER_IN_CALL_H

