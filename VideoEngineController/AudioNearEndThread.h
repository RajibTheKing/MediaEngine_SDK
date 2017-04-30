#ifndef AUDIO_NEAR_END_THREAD_H
#define AUDIO_NEAR_END_THREAD_H


#include <thread>



class CAudioNearEndThread
{
private:

	bool m_bAudioNearEndThreadRunning;
	bool m_bAudioNearEndThreadClosed;


protected:

	void AudioNearEndProcedure();
	std::thread StartAudioNearEndThread();
	void StopAudioNearEndThread();


public:

	CAudioNearEndThread();
	~CAudioNearEndThread();
};



#endif  // !AUDIO_NEAR_END_THREAD_H


