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

		short m_saCalleeSentData[MAX_AUDIO_FRAME_Length];
		SmartPointer<AudioMixer> m_pAudioMixer;

	};

} //namespace MediaSDK

#endif  // !AUDIO_FAR_END_PROCESSOR_VIEWER_H

