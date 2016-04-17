//
// Created by ipvision on 2/6/2016.
//

#ifndef _BITRATE_CONTROLLER_H_
#define _BITRATE_CONTROLLER_H_

#include "SmartPointer.h"
#include "LockHandler.h"
#include "Tools.h"
#include "VideoEncoder.h"
#include "SynchronizedMap.h"
#include "PacketHeader.h"

class CCommonElementsBucket;

class BitRateController 
{

public:

    BitRateController();
    ~BitRateController();

    void SetSharedObject(CCommonElementsBucket* pcSharedObject);
	void SetEncoder(CVideoEncoder* pcVideEnocder);

    bool HandleBitrateMiniPacket(CPacketHeader &crTempHeader);
	bool HandleNetworkTypeMiniPacket(CPacketHeader &crTempHeader);
    bool UpdateBitrate();
    void NotifyEncodedFrame(int &nrFrameSize);
	void SetInitialBitrate();
	int GetOpponentNetworkType();
	int GetOwnNetworkType();
	void SetOwnNetworkType(int nNetworkType);
	bool IsNetworkTypeMiniPacketReceived();
	void ResetVideoController();

private:

	int NeedToChangeBitRate(double dDataReceivedRatio);
	int NeedToNotifyClient(int nCurrentByte);

    Tools m_Tools;

	int m_nOppNotifiedByterate;
	int m_nOpponentNetworkType;
	int m_nOwnNetworkType;
	bool m_bNetworkTypeMiniPacketReceived;

	map<int, long long>  m_TimeDiffMapHelper;

    int m_nGoodSlotCounter;
    int m_nNormalSlotCounter;
    int m_nSlotCounterToUp;
	int m_nGoodSlotCounterToUp;
    double m_dPreviousMegaSlotStatus;

    int m_nFrameCounterBeforeEncoding;
    int m_nLastSendingSlot;
    int m_nPreviousByteRate;				// Will be changed based on side

    int m_nSlotIntervalCounter;				// Will be changed based on side
    double m_dTotalDataByteInSlots;			// Will be changed based on side
    double m_dAverageDataByteInSlots;		// Will be changed based on side

	bool m_bVideoQualityLowNotified;
	bool m_bVideoQualityHighNotified;
	bool m_bVideoShouldStopNotified;
    
    CCommonElementsBucket* m_pCommonElementsBucket;

    int m_nBytesSendInSlotInverval;
    int m_nBytesSendInMegaSlotInverval;
    int m_nBytesReceivedInMegaSlotInterval;
    double m_dFirstTimeBitRateChangeFactor;
    bool m_bMegSlotCounterShouldStop;
    bool m_bSetBitRateCalled;
    int m_nStopNotificationCounter;

    CSynchronizedMap m_BandWidthRatioHelper;
    CVideoEncoder *m_pVideoEncoder;

    int m_nLastState;
    int m_nSpiralCounter;
	int m_nContinuousUpCounter;
	int m_nContinuousUpCounterLimitToJump;
    int m_nUpCheckLimit;
	bool m_bInMaxBitrate;
    int m_nMostRecentRespondedSlotNumber;
};


#endif
