#include "AudioPlayingThread.h"
#include "LogPrinter.h"
#include "Tools.h"
#include "AudioFarEndDataProcessor.h"

namespace MediaSDK
{

	AudioPlayingThread::AudioPlayingThread(AudioFarEndDataProcessor *pFarEndProcessor) :
		m_pFarEndDataProcessor(pFarEndProcessor)
	{
		MR_DEBUG("#farEnd# AudioFarEndProcessorThread::AudioFarEndProcessorThread()");

		m_bAudioPlayingThreadRunning = false;
		m_bAudioPlayingThreadClosed = true;
	}


	AudioPlayingThread::~AudioPlayingThread()
	{
		MR_DEBUG("#farEnd# AudioFarEndProcessorThread::~AudioFarEndProcessorThread()");

		StopPlayingThread();
	}

	void AudioPlayingThread::AudioPlayingProcedure()
	{
		MR_DEBUG("#farEnd# AudioFarEndProcessorThread::AudioFarEndProcedure()");

		m_bAudioPlayingThreadRunning = true;
		m_bAudioPlayingThreadClosed = false;

		Tools::SOSleep(200);

		while (m_bAudioPlayingThreadRunning)
		{
			if (m_pFarEndDataProcessor != nullptr)
			{
				m_pFarEndDataProcessor->ProcessPlayingData();
			}
		}

		m_bAudioPlayingThreadClosed = true;
	}

	std::thread AudioPlayingThread::CreatePlayingThread()
	{
		return std::thread([=] { AudioPlayingProcedure(); });
	}


	void AudioPlayingThread::StartPlayingThread()
	{
		MR_DEBUG("#farEnd# AudioFarEndProcessorThread::StartFarEndThread()");

		std::thread audioPlayingThread = CreatePlayingThread();
		audioPlayingThread.detach();
	}


	void AudioPlayingThread::StopPlayingThread()
	{
		MR_DEBUG("#farEnd# AudioFarEndProcessorThread::StopFarEndThread()");

		m_bAudioPlayingThreadRunning = false;

		while (!m_bAudioPlayingThreadClosed)
		{
			Tools::SOSleep(1);
		}
	}
}