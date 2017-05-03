#include "AudioFarEndProcessorThread.h"
#include "LogPrinter.h"
#include "Tools.h"
#include "AudioFarEndDataProcessor.h"



AudioFarEndProcessorThread::AudioFarEndProcessorThread(AudioFarEndDataProcessor *pFarEndProcessor) :
m_pFarEndDataProcessor(pFarEndProcessor)
{
	MR_DEBUG("#farEnd# AudioFarEndProcessorThread::AudioFarEndProcessorThread()");

	m_bAudioFarEndThreadRunning = false;
	m_bAudioFarEndThreadClosed = true;
}


AudioFarEndProcessorThread::~AudioFarEndProcessorThread()
{
	MR_DEBUG("#farEnd# AudioFarEndProcessorThread::~AudioFarEndProcessorThread()");

	StopFarEndThread();
}


void AudioFarEndProcessorThread::AudioFarEndProcedure()
{
	MR_DEBUG("#farEnd# AudioFarEndProcessorThread::AudioFarEndProcedure()");

	long long llCapturedTime;

	m_bAudioFarEndThreadRunning = true;
	m_bAudioFarEndThreadClosed = false;

	while (m_bAudioFarEndThreadRunning)
	{
		if (m_pFarEndDataProcessor != nullptr)
		{
			m_pFarEndDataProcessor->ProcessFarEndData();
		}
	}

	m_bAudioFarEndThreadClosed = true;
}


std::thread AudioFarEndProcessorThread::CreateFarEndThread()
{
	return std::thread([=] { AudioFarEndProcedure(); });
}


void AudioFarEndProcessorThread::StartFarEndThread()
{
	MR_DEBUG("#farEnd# AudioFarEndProcessorThread::StartFarEndThread()");

	std::thread audioFarEndThread = CreateFarEndThread();
	audioFarEndThread.detach();
}


void AudioFarEndProcessorThread::StopFarEndThread()
{
	MR_DEBUG("#farEnd# AudioFarEndProcessorThread::StopFarEndThread()");

	m_bAudioFarEndThreadRunning = false;

	while (!m_bAudioFarEndThreadClosed)
	{
		Tools::SOSleep(1);
	}
}


