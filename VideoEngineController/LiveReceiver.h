//
// Created by ipvision on 10/23/2016.
//

#ifndef LIVESTREAMING_LIVERECEIVER_H
#define LIVESTREAMING_LIVERECEIVER_H

#include "SmartPointer.h"
#include "LockHandler.h"
#include "AudioDecoderBuffer.h"
#include "LiveVideoDecodingQueue.h"
#include "LiveAudioDecodingQueue.h"

class LiveReceiver {
public:
    LiveReceiver(CAudioDecoderBuffer *pAudioDecoderBuffer, LiveVideoDecodingQueue *pLiveVideoDecodingQueue);
    ~LiveReceiver();
    void SetVideoDecodingQueue(LiveVideoDecodingQueue *pQueue);
    void SetAudioDecodingQueue(LiveAudioDecodingQueue *pQueue);
    
	void PushAudioData(unsigned char* uchAudioData, int iLen, int numberOfFrames = 0, int *frameSizes = NULL, int numberOfMissingFrames = 0, int *missingFrames = NULL);
	void PushVideoData(unsigned char* uchVideoData, int iLen, int numberOfFrames = 0, int *frameSizes = NULL, int numberOfMissingFrames = 0, int *missingFrames = NULL);
    bool GetVideoFrame(unsigned char* uchVideoFrame,int iLen);
    void ProcessAudioStream(int nOffset, unsigned char* uchAudioData,int nDataLenght, int *pAudioFramsStartingByte, int nNumberOfAudioFrames, int *pMissingBlocks, int nNumberOfMissingBlocks);

private:
    SmartPointer<CLockHandler> m_pLiveReceiverMutex;
    CAudioDecoderBuffer *m_pAudioDecoderBuffer;
    LiveVideoDecodingQueue *m_pLiveVideoDecodingQueue;
    LiveAudioDecodingQueue *m_pLiveAudioDecodingQueue;
};


#endif //LIVESTREAMING_LIVERECEIVER_H
