//
// Created by ipvision on 2/6/2016.
//

#ifndef ANDROIDTESTCLIENTVE_FTEST_BITRATECONTROLLER_H
#define ANDROIDTESTCLIENTVE_FTEST_BITRATECONTROLLER_H


#include "SmartPointer.h"
#include "LockHandler.h"
#include "Tools.h"
#include "VideoEncoder.h"
#include "SynchronizedMap.h"

//#include "CommonElementsBucket.h"

class CCommonElementsBucket;

class BitRateController {
public:
    BitRateController();
    ~BitRateController();

    void SetSharedObject(CCommonElementsBucket* sharedObject);
    void SetEncoder(CVideoEncoder* VideEnocder);
    bool HandleBitrateMiniPacket(CPacketHeader &tempHeader);
    bool HandleNetworkTypeMiniPacket(CPacketHeader &tempHeader);
    bool UpdateBitrate();
    void NotifyEncodedFrame(int &nFrameSize);
    int NeedToChangeBitRate(double dataReceivedRatio);
    int NeedToNotifyClient(int iCurrentByte);

    long long m_lTimeStampForFirstMiniPkt;
    //int m_iWaititngForFirstMiniPkt;
    map<int, long long>  m_TimeDiffMapHelper;
    int m_OppNotifiedByterate;

    int m_iOpponentNetworkType;
    int m_iOwnNetworkType;

    int m_iNetTypeMiniPktRcv;

private:
    Tools m_Tools;
    int m_iGoodSlotCounter;
    int m_iNormalSlotCounter;
    int m_SlotCounter;
    double m_PrevMegaSlotStatus;

    int m_FrameCounterbeforeEncoding;
    int m_LastSendingSlot;

    int m_iPreviousByterate;

    int m_bGotOppBandwidth;
    int m_SlotIntervalCounter;
    double m_fTotalDataInSlots;
    double m_fAverageData;

	bool m_bVideoQualityLowNotified;
	bool m_bVideoQualityHighNotified;
	bool m_bVideoShouldStopNotified;
    
    CCommonElementsBucket* m_pCommonElementsBucket;

    int m_ByteSendInSlotInverval;
    int m_ByteSendInMegaSlotInverval;
    int m_ByteRecvInMegaSlotInterval;
    double dFirstTimeDecrease;
    bool m_bMegSlotCounterShouldStop;
    bool m_bsetBitrateCalled;
    int m_iStopNotificationController;

    bool m_bIsFirstTime;
    CSynchronizedMap m_BandWidthRatioHelper;
    long long m_lTimeStampForMiniPkt;
    long long timeDiffForMiniPkt;
    bool m_bIsFirstMiniPktRcv;
    CVideoEncoder *m_pVideoEncoder;

    int m_lastState;
    int m_iSpiralCounter;
    int m_iUpCheckLimit;
    
    
};


#endif //ANDROIDTESTCLIENTVE_FTEST_BITRATECONTROLLER_H
