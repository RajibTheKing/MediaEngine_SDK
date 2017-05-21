
#ifndef IPV_LIVE_VIDEO_DECODING_QUEUE_H
#define IPV_LIVE_VIDEO_DECODING_QUEUE_H

#include "SmartPointer.h"
#include "CommonTypes.h"
#include "Tools.h"

#define LIVE_VIDEO_DECODING_QUEUE_SIZE 75

#if defined(DESKTOP_C_SHARP)

#define MAX_VIDEO_ENCODED_FRAME_SIZE 200000

#else

#define MAX_VIDEO_ENCODED_FRAME_SIZE 100000

#endif


class LiveVideoDecodingQueue 
{

public:

    LiveVideoDecodingQueue();
    ~LiveVideoDecodingQueue();

    int Queue(unsigned char *saReceivedVideoFrameData, int nLength);
    int DeQueue(unsigned char *saReceivedVideoFrameData);
    void IncreamentIndex(int &irIndex);
    int GetQueueSize();
    void ResetBuffer();

private:

    int m_iPushIndex;
    int m_iPopIndex;
    int m_nQueueCapacity;
    int m_nQueueSize;

    unsigned char m_uchBuffer[LIVE_VIDEO_DECODING_QUEUE_SIZE][MAX_VIDEO_ENCODED_FRAME_SIZE];
    int m_naBufferDataLength[LIVE_VIDEO_DECODING_QUEUE_SIZE];

	Tools m_Tools;
    SmartPointer<CLockHandler> m_pLiveVideoDecodingQueueMutex;
};

#endif 
