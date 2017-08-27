#ifndef AUDIO_FAR_END_PROCESSOR_VIEWER_H 
#define AUDIO_FAR_END_PROCESSOR_VIEWER_H


#include "AudioFarEndDataProcessor.h"
#include "SmartPointer.h"

namespace MediaSDK
{
	class AudioMixer;

	class FarEndProcessorViewer : public AudioFarEndDataProcessor
	{

	public:

		FarEndProcessorViewer(int nServiceType, int nEntityType, CAudioCallSession *pAudioCallSession, bool bIsLiveStreamingRunning);
		~FarEndProcessorViewer() { }

		void ProcessFarEndData();

	private:

		short m_saCalleeSentData[AUDIO_MAX_FRAME_LENGTH_IN_BYTE];
		SharedPointer<AudioMixer> m_pAudioMixer;

	};

} //namespace MediaSDK

#endif  // !AUDIO_FAR_END_PROCESSOR_VIEWER_H

