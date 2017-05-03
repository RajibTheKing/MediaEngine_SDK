#ifndef AUDIO_NEAR_END_PROCESSOR_VIEWER_H
#define AUDIO_NEAR_END_PROCESSOR_VIEWER_H


#include "AudioNearEndDataProcessor.h"


class CAudioCallSession;
class CAudioShortBuffer;


class AudioNearEndProcessorViewer : public AudioNearEndDataProcessor
{

	CAudioShortBuffer *m_pAudioNearEndBuffer;
	CAudioCallSession *m_pAudioCallSession;


public:

	AudioNearEndProcessorViewer(int nServiceType, int nEntityType, CAudioCallSession *pAudioCallSession, CAudioShortBuffer *pAudioEncodingBuffer, bool bIsLiveStreamingRunning);
	~AudioNearEndProcessorViewer() { }

	void ProcessNearEndData();

};



#endif  // !AUDIO_NEAR_END_PROCESSOR_VIEWER_IN_CALL_H

