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


class CCommonElementsBucket;

class LiveReceiver {
public:
	LiveReceiver(CCommonElementsBucket* sharedObject);
    ~LiveReceiver();
    void SetVideoDecodingQueue(LiveVideoDecodingQueue *pQueue);
    void SetAudioDecodingQueue(LiveAudioDecodingQueue *pQueue);

	void PushVideoData(unsigned char* uchVideoData, int iLen, int numberOfFrames = 0, int *frameSizes = NULL, int numberOfMissingFrames = 0, int *missingFrames = NULL);
    bool GetVideoFrame(unsigned char* uchVideoFrame,int iLen);
    void ProcessAudioStream(int nOffset, unsigned char* uchAudioData,int nDataLenght, int *pAudioFramsStartingByte, int nNumberOfAudioFrames, int *pMissingBlocks, int nNumberOfMissingBlocks);
	void ProcessAudioStream(int nOffset, unsigned char* uchAudioData,int nDataLenght, int *pAudioFramsStartingByte, int nNumberOfAudioFrames, std::vector< std::pair<int,int> > vMissingBlocks);

private:
    SmartPointer<CLockHandler> m_pLiveReceiverMutex;
    CAudioDecoderBuffer *m_pAudioDecoderBuffer;
    LiveVideoDecodingQueue *m_pLiveVideoDecodingQueue;
    LiveAudioDecodingQueue *m_pLiveAudioDecodingQueue;
	CCommonElementsBucket* m_pCommonElementsBucket;
};


#endif //LIVESTREAMING_LIVERECEIVER_H
