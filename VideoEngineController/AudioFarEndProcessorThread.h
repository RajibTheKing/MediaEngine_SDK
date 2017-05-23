#ifndef AUDIO_FAR_END_PROCESSOR_THREAD_H
#define AUDIO_FAR_END_PROCESSOR_THREAD_H


#include <thread>

namespace MediaSDK
{

	class AudioFarEndDataProcessor;

	class AudioFarEndProcessorThread
	{
	private:

		bool m_bAudioFarEndThreadRunning;
		bool m_bAudioFarEndThreadClosed;

		AudioFarEndDataProcessor *m_pFarEndDataProcessor = nullptr;


	protected:

		void AudioFarEndProcedure();
		std::thread CreateFarEndThread();


	public:

		AudioFarEndProcessorThread(AudioFarEndDataProcessor *pFarEndProcessor);
		~AudioFarEndProcessorThread();

		void StartFarEndThread();
		void StopFarEndThread();

	};

} // namespace MediaSDK



#endif  // !AUDIO_FAR_END_PROCESSOR_THREAD_H

