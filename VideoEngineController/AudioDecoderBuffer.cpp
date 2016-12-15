
#include "AudioDecoderBuffer.h"

#include <string.h>
#include "Tools.h"
#include "LogPrinter.h"


CAudioByteBuffer::CAudioByteBuffer() :
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

CAudioByteBuffer::~CAudioByteBuffer()
{

}

void CAudioByteBuffer::ResetBuffer()
{
	Locker lock(*m_pAudioDecodingBufferMutex);

	m_iPushIndex = 0;
	m_iPopIndex = 0;
	m_nQueueSize = 0;
}

int CAudioByteBuffer::Queue(unsigned char *saReceivedAudioFrameData, int nLength)
{
	Locker lock(*m_pAudioDecodingBufferMutex);

	//LOGE("fahad --> CAudioDocderBuffer:Queue nLength = %d", nLength);

	memcpy(m_s2aAudioDecodingBuffer[m_iPushIndex], saReceivedAudioFrameData, nLength);

	m_naBufferDataLength[m_iPushIndex] = nLength;

#ifdef QUEUE_OVERFLOW_LOG
	m_BufferInsertionTime[m_iPushIndex] =  m_Tools.CurrentTimestamp();
#endif

	if (m_nQueueSize == m_nQueueCapacity)
	{
#ifdef QUEUE_OVERFLOW_LOG
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

#endif
		IncreamentIndex(m_iPopIndex);
	}
	else
	{
		m_nQueueSize++;
	}

	IncreamentIndex(m_iPushIndex);

	return 1;
}

int CAudioByteBuffer::DeQueue(unsigned char *saReceivedAudioFrameData)
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

void CAudioByteBuffer::IncreamentIndex(int &irIndex)
{
	irIndex++;

	if (irIndex >= m_nQueueCapacity)
		irIndex = 0;
}

int CAudioByteBuffer::GetQueueSize()
{
	Locker lock(*m_pAudioDecodingBufferMutex);

	return m_nQueueSize;
}
