#ifndef AUDIO_NEAR_END_THREAD_H
#define AUDIO_NEAR_END_THREAD_H


#include <thread>
#include "Size.h"


class CAudioNearEndDataProcessor;


class CAudioNearEndThread
{
private:

	bool m_bAudioNearEndThreadRunning;
	bool m_bAudioNearEndThreadClosed;
	bool m_bIsLiveStreamingRunning;

	int m_nEntityType;

	CAudioNearEndDataProcessor *m_pNearEndDataProcessor = nullptr;


protected:

	void AudioNearEndProcedure();
	std::thread StartAudioNearEndThread();
	void StopAudioNearEndThread();


public:

	CAudioNearEndThread(CAudioNearEndDataProcessor *pNearEndProcessor, bool isLiveStreamRunning, int nEntityType);
	~CAudioNearEndThread();
};



#endif  // !AUDIO_NEAR_END_THREAD_H


