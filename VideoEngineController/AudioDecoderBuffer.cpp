
#include "AudioDecoderBuffer.h"

#include <string.h>
#include "Tools.h"
#include "LogPrinter.h"

CAudioDecoderBuffer::CAudioDecoderBuffer() :
m_iPushIndex(0),
m_iPopIndex(0),
m_nQueueSize(0),
m_nQueueCapacity(MAX_AUDIO_DECODER_BUFFER_SIZE)
{
	m_pAudioDecodingBufferMutex.reset(new CLockHandler);
    

    mt_lPrevOverFlowTime = -1;
    mt_dAvgOverFlowTime = 0;
    mt_nOverFlowCount = 0;
    mt_lSumOverFlowTime = 0;
    
}

CAudioDecoderBuffer::~CAudioDecoderBuffer()
{

}

int CAudioDecoderBuffer::Queue(unsigned char *saReceivedAudioFrameData, int nLength)
{
	Locker lock(*m_pAudioDecodingBufferMutex);

	memcpy(m_s2aAudioDecodingBuffer[m_iPushIndex], saReceivedAudioFrameData, nLength);

	m_naBufferDataLength[m_iPushIndex] = nLength;
	m_BufferInsertionTime[m_iPushIndex] =  m_Tools.CurrentTimestamp();

	if (m_nQueueSize == m_nQueueCapacity)
	{

        if(mt_lPrevOverFlowTime == -1)
        {
            mt_lPrevOverFlowTime = m_Tools.CurrentTimestamp();
        }
        else
        {
            long long lOverFlowTime = m_Tools.CurrentTimestamp() - mt_lPrevOverFlowTime;
            mt_lSumOverFlowTime += lOverFlowTime;
            mt_nOverFlowCount ++;
            mt_dAvgOverFlowTime = mt_lSumOverFlowTime * 1.0 / mt_nOverFlowCount;


			CLogPrinter_WriteLog(CLogPrinter::DEBUGS, QUEUE_OVERFLOW_LOG ,"Audio Buffer OverFlow ( AudioDecodingBuffer )--> OverFlow DifftimeDecode  = "+m_Tools.LongLongToString(lOverFlowTime)+", mt_dAvgOverFlowTime = "+ m_Tools.DoubleToString(mt_dAvgOverFlowTime) );
            mt_lPrevOverFlowTime = m_Tools.CurrentTimestamp();
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

int CAudioDecoderBuffer::DeQueue(unsigned char *saReceivedAudioFrameData)
{
	Locker lock(*m_pAudioDecodingBufferMutex);

	if (m_nQueueSize == 0)
	{
		return -1;
	}
	else
	{
		int length = m_naBufferDataLength[m_iPopIndex];

		memcpy(saReceivedAudioFrameData, m_s2aAudioDecodingBuffer[m_iPopIndex], length);

		IncreamentIndex(m_iPopIndex);

		m_nQueueSize--;

		return length;
	}
}

void CAudioDecoderBuffer::IncreamentIndex(int &irIndex)
{
	irIndex++;

	if (irIndex >= m_nQueueCapacity)
		irIndex = 0;
}

int CAudioDecoderBuffer::GetQueueSize()
{
	Locker lock(*m_pAudioDecodingBufferMutex);

	return m_nQueueSize;
}
