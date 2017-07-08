#ifndef AUDIO_NEAR_END_THREAD_H
#define AUDIO_NEAR_END_THREAD_H


#include <thread>


namespace MediaSDK
{

	class AudioNearEndDataProcessor;

	class AudioNearEndProcessorThread
	{

	private:

		bool m_bAudioNearEndThreadRunning;
		bool m_bAudioNearEndThreadClosed;

		AudioNearEndDataProcessor *m_pNearEndDataProcessor = nullptr;


	protected:

		void AudioNearEndProcedure();
		std::thread CreateNearEndThread();


	public:

		AudioNearEndProcessorThread(AudioNearEndDataProcessor *pNearEndProcessor);
		~AudioNearEndProcessorThread();

		void StartNearEndThread();
		void StopAudioNearEndThread();

	};

} //namespace MediaSDK


#endif  // !AUDIO_NEAR_END_THREAD_H


