//
// Created by ipvision on 10/23/2016.
//

#include "LiveAudioDecodingQueue.h"
#include "ThreadTools.h"
#include "LogPrinter.h"

LiveAudioDecodingQueue::LiveAudioDecodingQueue() :
        m_iPushIndex(0),
        m_iPopIndex(0),
        m_nQueueSize(0),
        m_nQueueCapacity(LIVE_AUDIO_DECODING_QUEUE_SIZE)
{
    m_pLiveAudioDecodingQueueMutex.reset(new CLockHandler);
}

LiveAudioDecodingQueue::~LiveAudioDecodingQueue()
{
    SHARED_PTR_DELETE(m_pLiveAudioDecodingQueueMutex);
}

void LiveAudioDecodingQueue::ResetBuffer()
{
    Locker lock(*m_pLiveAudioDecodingQueueMutex);

    m_iPushIndex = 0;
    m_iPopIndex = 0;
    m_nQueueSize = 0;
}

int LiveAudioDecodingQueue::Queue(unsigned char *saReceivedAudioFrameData, int nLength)
{
    Locker lock(*m_pLiveAudioDecodingQueueMutex);

    memcpy(m_uchBuffer[m_iPushIndex], saReceivedAudioFrameData, nLength);

    m_naBufferDataLength[m_iPushIndex] = nLength;
    

    if (m_nQueueSize == m_nQueueCapacity)
    {
        IncreamentIndex(m_iPopIndex);
    }
    else
    {
        m_nQueueSize++;
    }

    IncreamentIndex(m_iPushIndex);

    return 1;
}

int LiveAudioDecodingQueue::DeQueue(unsigned char *saReceivedAudioFrameData)
{
    Locker lock(*m_pLiveAudioDecodingQueueMutex);

    if (m_nQueueSize == 0)
    {
        return -1;
    }
    else
    {
        int length = m_naBufferDataLength[m_iPopIndex];

        memcpy(saReceivedAudioFrameData, m_uchBuffer[m_iPopIndex], length);

        IncreamentIndex(m_iPopIndex);

        m_nQueueSize--;

        return length;
    }
}

void LiveAudioDecodingQueue::IncreamentIndex(int &irIndex)
{
    irIndex++;

    if (irIndex >= m_nQueueCapacity)
        irIndex = 0;
}

int LiveAudioDecodingQueue::GetQueueSize()
{
    Locker lock(*m_pLiveAudioDecodingQueueMutex);

    return m_nQueueSize;
}
