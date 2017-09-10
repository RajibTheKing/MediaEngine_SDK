#ifndef AUDIO_PLAYING_THREAD_H
#define AUDIO_PLAYING_THREAD_H

#include <thread>

namespace MediaSDK
{
	class AudioFarEndDataProcessor;

	class AudioPlayingThread
	{
	private:

		bool m_bAudioPlayingThreadRunning;
		bool m_bAudioPlayingThreadClosed;

		AudioFarEndDataProcessor *m_pFarEndDataProcessor = nullptr;


	protected:

		void AudioPlayingProcedure();
		std::thread CreatePlayingThread();


	public:

		AudioPlayingThread(AudioFarEndDataProcessor *pFarEndProcessor);
		~AudioPlayingThread();

		void StartPlayingThread();
		void StopPlayingThread();

	};
}


#endif //AUDIO_PLAYING_THREAD_H