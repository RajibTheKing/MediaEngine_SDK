//
// Created by ipvision on 10/23/2016.
//

#ifndef LIVESTREAMING_LIVEVIDEODECODINGQUEUE_H
#define LIVESTREAMING_LIVEVIDEODECODINGQUEUE_H

#include "SmartPointer.h"
#include "LockHandler.h"
#include "Tools.h"

#define LIVE_VIDEO_DECODING_QUEUE_SIZE 75
#define MAX_VIDEO_ENCODED_FRAME_SIZE 25000


class LiveVideoDecodingQueue {
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
    Tools m_Tools;

    unsigned char m_uchBuffer[LIVE_VIDEO_DECODING_QUEUE_SIZE][MAX_VIDEO_ENCODED_FRAME_SIZE];
    int m_naBufferDataLength[LIVE_VIDEO_DECODING_QUEUE_SIZE];

    SmartPointer<CLockHandler> m_pLiveVideoDecodingQueueMutex;
};


#endif //LIVESTREAMING_LIVEVIDEODECODINGQUEUE_H
