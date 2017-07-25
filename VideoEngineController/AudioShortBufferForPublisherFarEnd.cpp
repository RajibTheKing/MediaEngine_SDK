
#include "AudioShortBufferForPublisherFarEnd.h"

#include <string.h>
#include "LogPrinter.h"
#include "Tools.h"


namespace MediaSDK
{
	AudioShortBufferForPublisherFarEnd::AudioShortBufferForPublisherFarEnd() :

		m_iPushIndex(0),
		m_iPopIndex(0),
		m_nQueueSize(0),
		m_nQueueCapacity(MAX_AUDIO_ENCODING_BUFFER_SIZE),
		mt_llPrevOverFlowTime(-1),
		m_dAvgOverFlowTime(0),
		mt_nOverFlowCounter(0),
		mt_llSumOverFlowTime(0)
	{
		m_pAudioShortBufferForPublisherFarEndrMutex.reset(new CLockHandler);
	}

	AudioShortBufferForPublisherFarEnd::AudioShortBufferForPublisherFarEnd(int iQueueSize) :

		m_iPushIndex(0),
		m_iPopIndex(0),
		m_nQueueSize(0),
		m_nQueueCapacity((iQueueSize < MAX_AUDIO_ENCODING_BUFFER_SIZE ? iQueueSize : MAX_AUDIO_ENCODING_BUFFER_SIZE)),
		mt_llPrevOverFlowTime(-1),
		m_dAvgOverFlowTime(0),
		mt_nOverFlowCounter(0),
		mt_llSumOverFlowTime(0)
	{
		m_pAudioShortBufferForPublisherFarEndrMutex.reset(new CLockHandler);
	}

	AudioShortBufferForPublisherFarEnd::~AudioShortBufferForPublisherFarEnd()
	{

	}

	void AudioShortBufferForPublisherFarEnd::ResetBuffer()
	{
		AudioShortBufferPublisherLock lock(*m_pAudioShortBufferForPublisherFarEndrMutex);

		m_iPushIndex = 0;
		m_iPopIndex = 0;
		m_nQueueSize = 0;
		mt_llPrevOverFlowTime = -1;
		m_dAvgOverFlowTime = 0;
		mt_nOverFlowCounter = 0;
		mt_llSumOverFlowTime = 0;
	}

	int AudioShortBufferForPublisherFarEnd::EnQueue(short *saCapturedAudioFrameData, int nlength, long long llTimeStump, MuxHeader pMuxHeader)
	{
		LOG18("#18@# ENCO BUFFER SIZE %d -> HEAD %lld", m_nQueueCapacity, pMuxHeader.getCalleeId());
		AudioShortBufferPublisherLock lock(*m_pAudioShortBufferForPublisherFarEndrMutex);

		memcpy(m_s2aAudioEncodingBuffer[m_iPushIndex], saCapturedAudioFrameData, nlength * 2);

		m_naBufferDataLength[m_iPushIndex] = nlength;
		m_laReceivedTimeList[m_iPushIndex] = llTimeStump;
		m_pMuxHeaderBuffer[m_iPushIndex] = pMuxHeader;

		if (m_nQueueSize == m_nQueueCapacity)
		{

			if (mt_llPrevOverFlowTime == -1)
			{
				mt_llPrevOverFlowTime = Tools::CurrentTimestamp();
			}
			else
			{
				long long llOverFlowTime = Tools::CurrentTimestamp() - mt_llPrevOverFlowTime;
				mt_llSumOverFlowTime += llOverFlowTime;
				mt_nOverFlowCounter++;
				m_dAvgOverFlowTime = mt_llSumOverFlowTime * 1.0 / mt_nOverFlowCounter;

				CLogPrinter_WriteLog(CLogPrinter::DEBUGS, QUEUE_OVERFLOW_LOG, "Audio Buffer OverFlow ( AudioEncoderBuffer ) --> OverFlow Difftime = " + Tools::LongLongToString(llOverFlowTime) + ", m_dAvgOverFlowTimeDif = " + Tools::DoubleToString(m_dAvgOverFlowTime));

				mt_llPrevOverFlowTime = Tools::CurrentTimestamp();
			}


			IncreamentIndex(m_iPopIndex);
		}
		else
		{
			m_nQueueSize++;
		}

		IncreamentIndex(m_iPushIndex);

		return 1;
	}

	int AudioShortBufferForPublisherFarEnd::DeQueue(short *saCapturedAudioFrameData, long long &receivedTime, MuxHeader &pMuxHeader)
	{
		AudioShortBufferPublisherLock lock(*m_pAudioShortBufferForPublisherFarEndrMutex);

		if (m_nQueueSize == 0)
		{
			return -1;
		}
		else
		{
			int nlength = m_naBufferDataLength[m_iPopIndex];
			receivedTime = m_laReceivedTimeList[m_iPopIndex];
			pMuxHeader = m_pMuxHeaderBuffer[m_iPopIndex];
			memcpy(saCapturedAudioFrameData, m_s2aAudioEncodingBuffer[m_iPopIndex], nlength * 2);

			IncreamentIndex(m_iPopIndex);

			m_nQueueSize--;

			return nlength;
		}

		return 1;
	}

	void AudioShortBufferForPublisherFarEnd::IncreamentIndex(int &irIndex)
	{
		irIndex++;

		if (irIndex >= m_nQueueCapacity)
			irIndex = 0;
	}

	int AudioShortBufferForPublisherFarEnd::GetQueueSize()
	{
		AudioShortBufferPublisherLock lock(*m_pAudioShortBufferForPublisherFarEndrMutex);

		return m_nQueueSize;
	}


	int AudioShortBufferForPublisherFarEnd::DeQueueForCallee(short *saCapturedAudioFrameData, long long &receivedTime, MuxHeader &pMuxHeader, int iCalleeFrameNoSentByPublisher)
	{
		AudioShortBufferPublisherLock lock(*m_pAudioShortBufferForPublisherFarEndrMutex);

		if (m_nQueueSize == 0)
		{
			return -1;
		}
		else
		{
			int nlength = m_naBufferDataLength[m_iPopIndex];
			receivedTime = m_laReceivedTimeList[m_iPopIndex];
			if (receivedTime > iCalleeFrameNoSentByPublisher)
			{
				return -1;
			}
			memcpy(saCapturedAudioFrameData, m_s2aAudioEncodingBuffer[m_iPopIndex], nlength * 2);
			pMuxHeader = m_pMuxHeaderBuffer[m_iPopIndex];

			IncreamentIndex(m_iPopIndex);

			m_nQueueSize--;

			return nlength;
		}

		return 1;
	}

} //namespace MediaSDK
