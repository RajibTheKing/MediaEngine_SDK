//
// Created by Rajib Chandra Das on 26/2/2017.
//

#ifndef IPV_IDR_FRAME_INTERVAL_CONTROLLER_H
#define IPV_IDR_FRAME_INTERVAL_CONTROLLER_H

#include "SmartPointer.h"
#include "CommonTypes.h"
#include "Tools.h"
#include "VideoEncoder.h"
#include "VideoHeader.h"

class CCommonElementsBucket;

class IDRFrameIntervalController
{
public:

	IDRFrameIntervalController();
    ~IDRFrameIntervalController();

    void SetSharedObject(CCommonElementsBucket* pcSharedObject);
	void SetEncoder(CVideoEncoder* pcVideEnocder);
	bool Handle_IDRFrame_Control_Packet(CVideoHeader &crTempHeader, int nServiceType);
    void NotifyEncodedFrame(unsigned char *ucaEncodedFrame, int nEncodedFrameSize, long long nFrameNumber);
    bool NeedToGenerateIFrame(int nServiceType);
	
private:
	int NeedToCalculate();
    CCommonElementsBucket *m_pCommonElementsBucket;
    CVideoEncoder *m_pVideoEncoder;
    
    Tools m_Tools;
	LongLong m_FriendID;
    
    long long m_llLastSentIDRFrameNumber;
    long long m_llLastSentPFrameNumber;
    long long m_llLastFrameNumber;
    
    long long m_llFirstMissedFrameNumber;
    long long m_llLastMissedFrameNumber;
    
    int m_nRelativePframeCounter;
};


#endif
