//
// Created by ipvision on 10/23/2016.
//

#include "LiveVideoDecodingQueue.h"
#include "ThreadTools.h"
#include "LogPrinter.h"

LiveVideoDecodingQueue::LiveVideoDecodingQueue() :
        m_iPushIndex(0),
        m_iPopIndex(0),
        m_nQueueSize(0),
        m_nQueueCapacity(LIVE_VIDEO_DECODING_QUEUE_SIZE)
{
    m_pLiveVideoDecodingQueueMutex.reset(new CLockHandler);
}

LiveVideoDecodingQueue::~LiveVideoDecodingQueue()
{
    SHARED_PTR_DELETE(m_pLiveVideoDecodingQueueMutex);
}

void LiveVideoDecodingQueue::ResetBuffer()
{
    Locker lock(*m_pLiveVideoDecodingQueueMutex);

    m_iPushIndex = 0;
    m_iPopIndex = 0;
    m_nQueueSize = 0;
}

int LiveVideoDecodingQueue::Queue(unsigned char *saReceivedVideoFrameData, int nLength)
{
    Locker lock(*m_pLiveVideoDecodingQueueMutex);

    if(nLength > 30000)
    {
        CLogPrinter_WriteInstentTestLog(CLogPrinter::INFO, INSTENT_TEST_LOG_FF, "LiveVideoDecodingQueue::Queue   length : " + m_Tools.IntegertoStringConvert(nLength));
    }
    if( nLength < 0 || nLength > MAX_VIDEO_ENCODED_FRAME_SIZE )
    {
        CLogPrinter_WriteInstentTestLog(CLogPrinter::INFO, INSTENT_TEST_LOG_FF, "LiveVideoDecodingQueue::Queue   length : " + m_Tools.IntegertoStringConvert(nLength));
        //return 0;
    }

    memcpy(m_uchBuffer[m_iPushIndex], saReceivedVideoFrameData, nLength);

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

int LiveVideoDecodingQueue::DeQueue(unsigned char *saReceivedVideoFrameData)
{
    Locker lock(*m_pLiveVideoDecodingQueueMutex);

    if (m_nQueueSize == 0)
    {
        return -1;
    }
    else
    {
        int length = m_naBufferDataLength[m_iPopIndex];

        memcpy(saReceivedVideoFrameData, m_uchBuffer[m_iPopIndex], length);

        IncreamentIndex(m_iPopIndex);

        m_nQueueSize--;

        return length;
    }
}

void LiveVideoDecodingQueue::IncreamentIndex(int &irIndex)
{
    irIndex++;

    if (irIndex >= m_nQueueCapacity)
        irIndex = 0;
}

int LiveVideoDecodingQueue::GetQueueSize()
{
    Locker lock(*m_pLiveVideoDecodingQueueMutex);

    return m_nQueueSize;
}
