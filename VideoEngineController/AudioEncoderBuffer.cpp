
#include "AudioEncoderBuffer.h"

#include <string.h>
#include "LogPrinter.h"
#include "Tools.h"


CAudioCodecBuffer::CAudioCodecBuffer() :

m_iPushIndex(0),
m_iPopIndex(0),
m_nQueueSize(0),
m_nQueueCapacity(MAX_AUDIO_ENCODING_BUFFER_SIZE),
mt_llPrevOverFlowTime(-1),
m_dAvgOverFlowTime(0),
mt_nOverFlowCounter(0),
mt_llSumOverFlowTime(0)

{
	m_pAudioEnocdingBufferMutex.reset(new CLockHandler);
}

CAudioCodecBuffer::~CAudioCodecBuffer()
{

}

void CAudioCodecBuffer::ResetBuffer()
{
	Locker lock(*m_pAudioEnocdingBufferMutex);

	m_iPushIndex = 0;
	m_iPopIndex = 0;
	m_nQueueSize = 0;
	mt_llPrevOverFlowTime = -1;
	m_dAvgOverFlowTime = 0;
	mt_nOverFlowCounter = 0;
	mt_llSumOverFlowTime = 0;
}

int CAudioCodecBuffer::Queue(short *saCapturedAudioFrameData, int nlength)
{
	Locker lock(*m_pAudioEnocdingBufferMutex);

	memcpy(m_s2aAudioEncodingBuffer[m_iPushIndex], saCapturedAudioFrameData, nlength * 2);

	m_naBufferDataLength[m_iPushIndex] = nlength;
	m_laReceivedTimeList[m_iPushIndex] = m_Tools.CurrentTimestamp();

	if (m_nQueueSize == m_nQueueCapacity)
	{

        if(mt_llPrevOverFlowTime == -1)
        {
            mt_llPrevOverFlowTime = m_Tools.CurrentTimestamp();
        }
        else
        {
            long long llOverFlowTime = m_Tools.CurrentTimestamp() - mt_llPrevOverFlowTime;
            mt_llSumOverFlowTime += llOverFlowTime;
            mt_nOverFlowCounter++;
            m_dAvgOverFlowTime = mt_llSumOverFlowTime * 1.0 / mt_nOverFlowCounter;

			CLogPrinter_WriteLog(CLogPrinter::DEBUGS, QUEUE_OVERFLOW_LOG ,"Audio Buffer OverFlow ( AudioEncoderBuffer ) --> OverFlow Difftime = "+m_Tools.LongLongToString(llOverFlowTime)+", m_dAvgOverFlowTimeDif = "+ m_Tools.DoubleToString(m_dAvgOverFlowTime) );

			mt_llPrevOverFlowTime = m_Tools.CurrentTimestamp();
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

int CAudioCodecBuffer::DeQueue(short *saCapturedAudioFrameData, long long &receivedTime)
{
	Locker lock(*m_pAudioEnocdingBufferMutex);

	if (m_nQueueSize == 0)
	{
		return -1;
	}
	else
	{
		int nlength = m_naBufferDataLength[m_iPopIndex];
		receivedTime = m_laReceivedTimeList[m_iPopIndex];

		memcpy(saCapturedAudioFrameData, m_s2aAudioEncodingBuffer[m_iPopIndex], nlength * 2);

		IncreamentIndex(m_iPopIndex);

		m_nQueueSize--;

		return nlength;
	}
	
	return 1;
}

void CAudioCodecBuffer::IncreamentIndex(int &irIndex)
{
	irIndex++;

	if (irIndex >= m_nQueueCapacity)
		irIndex = 0;
}

int CAudioCodecBuffer::GetQueueSize()
{
	Locker lock(*m_pAudioEnocdingBufferMutex);

	return m_nQueueSize;
}
