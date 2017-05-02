#include "AudioNearEndProcessorThread.h"
#include "LogPrinter.h"
#include "Tools.h"
#include "AudioNearEndDataProcessor.h"



AudioNearEndProcessorThread::AudioNearEndProcessorThread(AudioNearEndDataProcessor *pNearEndProcessor) :
m_pNearEndDataProcessor(pNearEndProcessor)
{
	MR_DEBUG("CAudioNearEndThread::CAudioNearEndThread()");

	m_bAudioNearEndThreadRunning = false;
	m_bAudioNearEndThreadClosed = true;
}


AudioNearEndProcessorThread::~AudioNearEndProcessorThread()
{
	MR_DEBUG("CAudioNearEndThread::~CAudioNearEndThread()");

	StopAudioNearEndThread();
}


void AudioNearEndProcessorThread::AudioNearEndProcedure()
{
	MR_DEBUG("CAudioNearEndThread::AudioNearEndProcedure()");

	long long llCapturedTime;

	m_bAudioNearEndThreadRunning = true;
	m_bAudioNearEndThreadClosed = false;
	
	while (m_bAudioNearEndThreadRunning)
	{
		m_pNearEndDataProcessor->ProcessNearEndData();
	}

	m_bAudioNearEndThreadClosed = true;
}


std::thread AudioNearEndProcessorThread::CreateNearEndThread()
{
	return std::thread([=] { AudioNearEndProcedure(); });
}


void AudioNearEndProcessorThread::StartNearEndThread()
{
	MR_DEBUG("AudioNearEndProcessorThread::StartNearEndThread()");

	std::thread audioNearEndThread = CreateNearEndThread();
	audioNearEndThread.detach();
}


void AudioNearEndProcessorThread::StopAudioNearEndThread()
{
	MR_DEBUG("CAudioNearEndThread::StopAudioNearEndThread()");

	m_bAudioNearEndThreadRunning = false;

	while (!m_bAudioNearEndThreadClosed)
	{
		Tools::SOSleep(1);
	}
}



