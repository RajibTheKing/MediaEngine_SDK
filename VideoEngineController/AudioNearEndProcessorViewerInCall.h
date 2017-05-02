#ifndef AUDIO_NEAR_END_PROCESSOR_VIEWER_IN_CALL_H
#define AUDIO_NEAR_END_PROCESSOR_VIEWER_IN_CALL_H


#include "AudioNearEndDataProcessor.h"


class CAudioCallSession;
class CAudioShortBuffer;


class AudioNearEndProcessorViewerInCall : public AudioNearEndDataProcessor
{

	CAudioShortBuffer *m_pAudioEncodingBuffer;
	CAudioCallSession *m_pAudioCallSession;


public:

	AudioNearEndProcessorViewerInCall(int nServiceType, int nEntityType, CAudioCallSession *pAudioCallSession, CAudioShortBuffer *pAudioEncodingBuffer, bool bIsLiveStreamingRunning);
	~AudioNearEndProcessorViewerInCall();

	void ProcessNearEndData();

};



#endif  // !AUDIO_NEAR_END_PROCESSOR_VIEWER_IN_CALL_H

