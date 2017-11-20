#ifndef AUDIO_NEAR_END_PROCESSOR_VIEWER_H
#define AUDIO_NEAR_END_PROCESSOR_VIEWER_H


#include "AudioNearEndDataProcessor.h"


namespace MediaSDK
{

	class CAudioCallSession;
	class CAudioShortBuffer;
	class AudioDeviceInformation;


	class AudioNearEndProcessorViewer : public AudioNearEndDataProcessor
	{

	public:

		AudioNearEndProcessorViewer(int nServiceType, int nEntityType, CAudioCallSession *pAudioCallSession, SharedPointer<CAudioShortBuffer> pAudioEncodingBuffer, bool bIsLiveStreamingRunning);
		~AudioNearEndProcessorViewer();

		void ProcessNearEndData();	

	private:
		AudioDeviceInformation *m_pAudioDeviceInformation;

		int m_nTotalSentFrameSize;
	};

}  //namespace MediaSDK


#endif  // !AUDIO_NEAR_END_PROCESSOR_VIEWER_IN_CALL_H

