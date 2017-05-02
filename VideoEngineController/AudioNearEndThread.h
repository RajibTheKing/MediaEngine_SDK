#ifndef AUDIO_NEAR_END_THREAD_H
#define AUDIO_NEAR_END_THREAD_H


#include <thread>
#include "Size.h"


class CAudioShortBuffer;


class CAudioNearEndThread
{
private:

	bool m_bAudioNearEndThreadRunning;
	bool m_bAudioNearEndThreadClosed;

	int m_nNearEndDataLen;

	short m_saAudioRecorderFrame[MAX_AUDIO_FRAME_Length];      //Always contains UnMuxed Data
	CAudioShortBuffer *m_pAudioEncodingBuffer = nullptr;

#ifdef DUMP_FILE
	FILE *m_fNearEndRawData = nullptr;
#endif

protected:

	void AudioNearEndProcedure();
	std::thread StartAudioNearEndThread();
	void StopAudioNearEndThread();

	void DumpNearEndData();


public:

	CAudioNearEndThread(CAudioShortBuffer *audioNearEndBuffer);
	~CAudioNearEndThread();
};



#endif  // !AUDIO_NEAR_END_THREAD_H


