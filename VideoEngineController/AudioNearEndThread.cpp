#include "AudioNearEndThread.h"
#include "LogPrinter.h"
#include "Tools.h"
#include "AudioEncoderBuffer.h"



CAudioNearEndThread::CAudioNearEndThread(CAudioShortBuffer *audioNearEndBuffer) :
m_pAudioEncodingBuffer(audioNearEndBuffer),
m_bAudioNearEndThreadRunning(false),
m_bAudioNearEndThreadClosed(true)
{
	MR_DEBUG("CAudioNearEndThread::CAudioNearEndThread()");

#ifdef DUMP_FILE
	m_fNearEndRawData = fopen("/sdcard/NearEndRawData.pcm", "wb");
#endif

	std::thread audioNearEndThread = StartAudioNearEndThread();
	audioNearEndThread.detach();
}


CAudioNearEndThread::~CAudioNearEndThread()
{
	MR_DEBUG("CAudioNearEndThread::~CAudioNearEndThread()");

	StopAudioNearEndThread();

#ifdef DUMP_FILE
	if (m_fNearEndRawData != nullptr)
	{
		fclose(m_fNearEndRawData);
		m_fNearEndRawData = nullptr;
	}
#endif
}


void CAudioNearEndThread::AudioNearEndProcedure()
{
	MR_DEBUG("CAudioNearEndThread::AudioNearEndProcedure()");

	long long llCapturedTime;

	m_bAudioNearEndThreadRunning = true;
	m_bAudioNearEndThreadClosed = false;
	
	while (m_bAudioNearEndThreadRunning)
	{
		if (m_pAudioEncodingBuffer->GetQueueSize() == 0)
		{
			Tools::SOSleep(10);
		}
		else
		{
			m_nNearEndDataLen = m_pAudioEncodingBuffer->DeQueue(m_saAudioRecorderFrame, llCapturedTime);
			DumpNearEndData();
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


void CAudioNearEndThread::DumpNearEndData()
{
#ifdef DUMP_FILE
	MR_DEBUG("CAudioNearEndThread::DumpNearEndData()");
	fwrite(m_saAudioRecorderFrame, 2, m_nNearEndDataLen, m_fNearEndRawData);
#endif
}


