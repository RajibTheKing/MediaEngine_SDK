#include "AudioNearEndThread.h"
#include "LogPrinter.h"
#include "Tools.h"




CAudioNearEndThread::CAudioNearEndThread()
{
	MR_DEBUG("CAudioNearEndThread::CAudioNearEndThread()");

	m_bAudioNearEndThreadRunning = false;
	m_bAudioNearEndThreadClosed = true;

	std::thread audioNearEndThread = StartAudioNearEndThread();
	audioNearEndThread.detach();
}


CAudioNearEndThread::~CAudioNearEndThread()
{
	MR_DEBUG("CAudioNearEndThread::~CAudioNearEndThread()");

	StopAudioNearEndThread();
}


void CAudioNearEndThread::AudioNearEndProcedure()
{
	MR_DEBUG("CAudioNearEndThread::AudioNearEndProcedure()");

	m_bAudioNearEndThreadRunning = true;
	m_bAudioNearEndThreadClosed = false;


	while (m_bAudioNearEndThreadRunning)
	{

	}

	m_bAudioNearEndThreadClosed = true;
}


std::thread CAudioNearEndThread::StartAudioNearEndThread()
{
	return std::thread([=] { AudioNearEndProcedure(); });
}


void CAudioNearEndThread::StopAudioNearEndThread()
{
	MR_DEBUG("CAudioNearEndThread::StopAudioNearEndThread()");

	m_bAudioNearEndThreadRunning = false;

	while (!m_bAudioNearEndThreadClosed)
	{
		Tools::SOSleep(1);
	}
}


