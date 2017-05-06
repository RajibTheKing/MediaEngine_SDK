#ifndef AUDIO_NEAR_END_PROCESSOR_CALL_H
#define AUDIO_NEAR_END_PROCESSOR_CALL_H

#include "AudioNearEndDataProcessor.h"


class CAudioCallSession;
class CAudioShortBuffer;


class AudioNearEndProcessorCall : public AudioNearEndDataProcessor
{

	CAudioShortBuffer *m_pAudioNearEndBuffer;


public:
	
	AudioNearEndProcessorCall(int nServiceType, int nEntityType, CAudioCallSession *pAudioCallSession, CAudioShortBuffer *pAudioEncodingBuffer, bool bIsLiveStreamingRunning);
	~AudioNearEndProcessorCall() { }

	void ProcessNearEndData();

};


#endif  // !AUDIO_NEAR_END_PROCESSOR_CALL_H

