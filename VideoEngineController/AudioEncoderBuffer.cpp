
#include "AudioEncoderBuffer.h"

#include <string.h>
#include "LogPrinter.h"
#include "Tools.h"


CAudioEncoderBuffer::CAudioEncoderBuffer() :

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

CAudioEncoderBuffer::~CAudioEncoderBuffer()
{

}

int CAudioEncoderBuffer::Queue(short *saCapturedAudioFrameData, int nlength)
{
	Locker lock(*m_pAudioEnocdingBufferMutex);

	memcpy(m_s2aAudioEncodingBuffer[m_iPushIndex], saCapturedAudioFrameData, nlength * 2);

	m_naBufferDataLength[m_iPushIndex] = nlength;

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

			//CLogPrinter_WriteSpecific5(CLogPrinter::DEBUGS, "TheVampire--> OverFlow Difftime = "+m_Tools.LongLongToString(llOverFlowTime)+", m_dAvgOverFlowTimeDif = "+ m_Tools.DoubleToString(m_dAvgOverFlowTime) );

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

int CAudioEncoderBuffer::DeQueue(short *saCapturedAudioFrameData)
{
	Locker lock(*m_pAudioEnocdingBufferMutex);

	if (m_nQueueSize == 0)
	{
		return -1;
	}
	else
	{
		int nlength = m_naBufferDataLength[m_iPopIndex];

		memcpy(saCapturedAudioFrameData, m_s2aAudioEncodingBuffer[m_iPopIndex], nlength * 2);

		IncreamentIndex(m_iPopIndex);

		m_nQueueSize--;

		return nlength;
	}
	
	return 1;
}

void CAudioEncoderBuffer::IncreamentIndex(int &irIndex)
{
	irIndex++;

	if (irIndex >= m_nQueueCapacity)
		irIndex = 0;
}

int CAudioEncoderBuffer::GetQueueSize()
{
	Locker lock(*m_pAudioEnocdingBufferMutex);

	return m_nQueueSize;
}
