//
// Created by ipvision on 10/23/2016.
//

#ifndef LIVESTREAMING_LIVEAUDIODECODINGQUEUE_H
#define LIVESTREAMING_LIVEAUDIODECODINGQUEUE_H

#include "SmartPointer.h"
#include "LockHandler.h"
#include "Tools.h"

#define LIVE_AUDIO_DECODING_QUEUE_SIZE 5
#define MAX_AUDIO_ENCODED_FRAME_LEN 4096


class LiveAudioDecodingQueue
{
public:

    LiveAudioDecodingQueue();
    ~LiveAudioDecodingQueue();

    int Queue(unsigned char *saReceivedAudioFrameData, int nLength);
    int DeQueue(unsigned char *saReceivedAudioFrameData);
    void IncreamentIndex(int &irIndex);
    int GetQueueSize();
    void ResetBuffer();

private:

    int m_iPushIndex;
    int m_iPopIndex;
    int m_nQueueCapacity;
    int m_nQueueSize;
    Tools m_Tools;

    unsigned char m_uchBuffer[LIVE_AUDIO_DECODING_QUEUE_SIZE][MAX_AUDIO_ENCODED_FRAME_LEN];
    int m_naBufferDataLength[LIVE_AUDIO_DECODING_QUEUE_SIZE];

    SmartPointer<CLockHandler> m_pLiveAudioDecodingQueueMutex;
};


#endif //LIVESTREAMING_LIVEAUDIODECODINGQUEUE_H
