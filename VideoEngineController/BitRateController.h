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

class BitRateController {
public:
    BitRateController();
    ~BitRateController();

    void SetEncoder(CVideoEncoder* VideEnocder);
    bool HandleBitrateMiniPacket(CPacketHeader &tempHeader);
    bool UpdateBitrate();
    void NotifyEncodedFrame(int nFrameSize);
    int NeedToChangeBitRate(double dataReceivedRatio);

private:
    Tools m_Tools;
    int m_iGoodSlotCounter;
    int m_iNormalSlotCounter;
    int m_SlotCounter;
    double m_PrevMegaSlotStatus;

    int m_FrameCounterbeforeEncoding;
    int m_LastSendingSlot;

    int m_OppNotifiedByterate;
    int m_iPreviousByterate;

    int m_bGotOppBandwidth;
    int m_SlotIntervalCounter;

    int m_ByteSendInSlotInverval;
    int m_ByteSendInMegaSlotInverval;
    int m_ByteRecvInMegaSlotInterval;
    double dFirstTimeDecrease;
    bool m_bMegSlotCounterShouldStop;
    bool m_bsetBitrateCalled;
    CSynchronizedMap m_BandWidthRatioHelper;
    CVideoEncoder *m_pVideoEncoder;
};


#endif //ANDROIDTESTCLIENTVE_FTEST_BITRATECONTROLLER_H
