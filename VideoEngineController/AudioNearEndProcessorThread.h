#ifndef AUDIO_NEAR_END_THREAD_H
#define AUDIO_NEAR_END_THREAD_H


#include <thread>


namespace MediaSDK
{

	class AudioNearEndDataProcessor;

	class AudioNearEndProcessorThread
	{

	public:

		AudioNearEndProcessorThread(AudioNearEndDataProcessor *pNearEndProcessor);
		~AudioNearEndProcessorThread();

		void StartNearEndThread();
		void StopAudioNearEndThread();


	protected:

		void AudioNearEndProcedure();
		std::thread CreateNearEndThread();

	private:

		bool m_bAudioNearEndThreadRunning;
		bool m_bAudioNearEndThreadClosed;

		AudioNearEndDataProcessor *m_pNearEndDataProcessor = nullptr;
	};

} //namespace MediaSDK


#endif  // !AUDIO_NEAR_END_THREAD_H


