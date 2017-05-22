
#ifndef IPV_BITRATE_CONTROLLER_H
#define IPV_BITRATE_CONTROLLER_H

#include "SmartPointer.h"
#include "Tools.h"
#include "VideoEncoder.h"
#include "SynchronizedMap.h"
//#include "PacketHeader.h"
#include "VideoHeader.h"

#define STOP_VIDEO_FOR_BITRATE_COUNTER 10

namespace MediaSDK
{

	class CCommonElementsBucket;

	class BitRateController
	{

	public:

		BitRateController(int nFPS, LongLong llfriendID);
		~BitRateController();

		void SetSharedObject(CCommonElementsBucket* pcSharedObject);
		void SetEncoder(CVideoEncoder* pcVideEnocder);

		//bool HandleBitrateMiniPacket(CPacketHeader &crTempHeader, int nServiceType);
		//bool HandleNetworkTypeMiniPacket(CPacketHeader &crTempHeader);

		bool HandleBitrateMiniPacket(CVideoHeader &crTempHeader, int nServiceType);
		bool HandleNetworkTypeMiniPacket(CVideoHeader &crTempHeader);


		bool UpdateBitrate();
		void NotifyEncodedFrame(int &nrFrameSize);
		void SetInitialBitrate();

		void SetCallFPS(int nFPS);

		int GetOpponentNetworkType();
		void SetOpponentNetworkType(int nNetworkType);

		int GetOwnNetworkType();
		void SetOwnNetworkType(int nNetworkType);

		bool IsNetworkTypeMiniPacketReceived();
		void ResetVideoController();

	private:

		int NeedToChangeBitRate(double dDataReceivedRatio);
		int NeedToNotifyClient(int nCurrentByte);

		Tools m_Tools;

		LongLong m_FriendID;

		int m_nOppNotifiedByterate;
		int m_nOpponentNetworkType;
		int m_nOwnNetworkType;
		bool m_bNetworkTypeMiniPacketReceived;

		int m_nCallFPS;

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
		int m_nVideoShouldStopCounter;

		CSynchronizedMap m_BandWidthRatioHelper;
		CVideoEncoder *m_pVideoEncoder;

		int m_nLastState;
		int m_nSpiralCounter;
		int m_nContinuousUpCounter;
		int m_nContinuousUpCounterLimitToJump;
		int m_nUpCheckLimit;
		int m_nMostRecentRespondedSlotNumber;
	};

} //namespace MediaSDK

#endif
