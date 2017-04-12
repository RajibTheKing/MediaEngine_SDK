//
// Created by ipvision on 10/23/2016.
//

#ifndef LIVESTREAMING_LIVERECEIVER_H
#define LIVESTREAMING_LIVERECEIVER_H

#include "SmartPointer.h"
#include "LockHandler.h"
#include "LiveVideoDecodingQueue.h"

#include<vector>
class CCommonElementsBucket;

class LiveReceiver {
public:
	LiveReceiver(CCommonElementsBucket* sharedObject);
    ~LiveReceiver();

    void SetVideoDecodingQueue(LiveVideoDecodingQueue *pQueue);    
	void PushVideoData(unsigned char* uchVideoData, int iLen, int numberOfFrames = 0, int *frameSizes = NULL, int numberOfMissingFrames = 0, int *missingFrames = NULL);
	void PushVideoDataVector(int offset, unsigned char* uchVideoData, int iLen, int numberOfFrames, int *frameSizes, std::vector< std::pair<int, int> > vMissingFrames);
    bool GetVideoFrame(unsigned char* uchVideoFrame,int iLen);	
	
private:

	Tools m_Tools;
	
    SmartPointer<CLockHandler> m_pLiveReceiverMutex;
    LiveVideoDecodingQueue *m_pLiveVideoDecodingQueue;    
	CCommonElementsBucket* m_pCommonElementsBucket;
	// FILE* logFile;
};


#endif //LIVESTREAMING_LIVERECEIVER_H
