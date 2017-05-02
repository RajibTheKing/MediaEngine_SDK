#include "AudioNearEndThread.h"
#include "LogPrinter.h"
#include "Tools.h"
#include "AudioNearEndDataProcessor.h"



CAudioNearEndThread::CAudioNearEndThread(CAudioNearEndDataProcessor *pNearEndProcessor, bool isLiveStreamRunning, int nEntityType) :
m_pNearEndDataProcessor(pNearEndProcessor),
m_bIsLiveStreamingRunning(isLiveStreamRunning),
m_nEntityType(nEntityType),
m_bAudioNearEndThreadRunning(false),
m_bAudioNearEndThreadClosed(true)
{
	MR_DEBUG("CAudioNearEndThread::CAudioNearEndThread()");

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

	long long llCapturedTime;

	m_bAudioNearEndThreadRunning = true;
	m_bAudioNearEndThreadClosed = false;
	
	while (m_bAudioNearEndThreadRunning)
	{
		if (m_bIsLiveStreamingRunning)
		{
			if (ENTITY_TYPE_PUBLISHER == m_nEntityType || ENTITY_TYPE_PUBLISHER_CALLER == m_nEntityType)
			{
				m_pNearEndDataProcessor->LiveStreamNearendProcedurePublisher();
			}
			else if (ENTITY_TYPE_VIEWER == m_nEntityType || ENTITY_TYPE_VIEWER_CALLEE == m_nEntityType)
			{
				m_pNearEndDataProcessor->LiveStreamNearendProcedureViewer();
			}
		}
		else
		{
			m_pNearEndDataProcessor->AudioCallNearendProcedure();
		}
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



