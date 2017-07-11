#include "AudioNearEndProcessorThread.h"
#include "LogPrinter.h"
#include "Tools.h"
#include "AudioNearEndDataProcessor.h"
#include "AudioCallSession.h"

namespace MediaSDK
{


	AudioNearEndProcessorThread::AudioNearEndProcessorThread(AudioNearEndDataProcessor *pNearEndProcessor) :
		m_pNearEndDataProcessor(pNearEndProcessor)
	{
		MR_DEBUG("#nearEnd# CAudioNearEndThread::CAudioNearEndThread()");

		m_bAudioNearEndThreadRunning = false;
		m_bAudioNearEndThreadClosed = true;
	}


	AudioNearEndProcessorThread::~AudioNearEndProcessorThread()
	{
		MR_DEBUG("#nearEnd# CAudioNearEndThread::~CAudioNearEndThread()");

		StopAudioNearEndThread();
	}


	void AudioNearEndProcessorThread::AudioNearEndProcedure()
	{
		MR_DEBUG("#nearEnd# CAudioNearEndThread::AudioNearEndProcedure()");
		Tools::SetThreadName("AudioNEThread");
		long long llCapturedTime;

		m_bAudioNearEndThreadRunning = true;
		m_bAudioNearEndThreadClosed = false;

		m_pNearEndDataProcessor->m_pAudioCallSession->m_recordBuffer->Clear();

		while (m_bAudioNearEndThreadRunning)
		{
			if (m_pNearEndDataProcessor != nullptr)
			{
				m_pNearEndDataProcessor->ProcessNearEndData();
			}
		}

		m_bAudioNearEndThreadClosed = true;
	}


	std::thread AudioNearEndProcessorThread::CreateNearEndThread()
	{
		return std::thread([=] { AudioNearEndProcedure(); });
	}


	void AudioNearEndProcessorThread::StartNearEndThread()
	{
		MR_DEBUG("#nearEnd# AudioNearEndProcessorThread::StartNearEndThread()");

		std::thread audioNearEndThread = CreateNearEndThread();
		audioNearEndThread.detach();
	}


	void AudioNearEndProcessorThread::StopAudioNearEndThread()
	{
		MR_DEBUG("#nearEnd# CAudioNearEndThread::StopAudioNearEndThread()");

		m_bAudioNearEndThreadRunning = false;

		while (!m_bAudioNearEndThreadClosed)
		{
			Tools::SOSleep(1);
		}
	}

} //namespace MediaSDK


