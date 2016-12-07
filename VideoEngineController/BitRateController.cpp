//
// Created by ipvision on 2/6/2016.
//

#include "BitRateController.h"
#include "PacketHeader.h"
#include "Size.h"
#include "LogPrinter.h"
#include "CommonElementsBucket.h"

BitRateController::BitRateController(int nFPS, LongLong llfriendID) :

    m_nBytesReceivedInMegaSlotInterval(0),
    m_nSlotIntervalCounter(0),
    m_bMegSlotCounterShouldStop(false),
    m_bSetBitRateCalled(false),
    m_nPreviousByteRate(BITRATE_MAX/8),
    m_nBytesSendInSlotInverval(0),
    m_nFrameCounterBeforeEncoding(nFPS),
    m_nLastSendingSlot(0),
    m_dTotalDataByteInSlots(0.0),
    m_dAverageDataByteInSlots(0.0),
    m_nStopNotificationCounter(0),
    m_nOpponentNetworkType(NETWORK_TYPE_NOT_2G),
    m_bNetworkTypeMiniPacketReceived(false),
    m_nLastState(BITRATE_CHANGE_DOWN),
    m_nSpiralCounter(0),
    m_nUpCheckLimit(GOOD_MEGASLOT_TO_UP),
	m_nContinuousUpCounter(0),
	m_nContinuousUpCounterLimitToJump(1),
	m_bInMaxBitrate(false),
	m_dFirstTimeBitRateChangeFactor(BITRATE_DECREMENT_FACTOR),
	m_nOppNotifiedByterate(0),
	m_nMostRecentRespondedSlotNumber(-1),
	m_nGoodSlotCounter(0),
	m_nNormalSlotCounter(0),
	m_nSlotCounterToUp(0),
	m_dPreviousMegaSlotStatus(1),
	m_nOwnNetworkType(NETWORK_TYPE_NOT_2G),
	m_nGoodSlotCounterToUp(GOOD_MEGASLOT_TO_UP * GOOD_MEGASLOT_TO_UP_TOLERANCE),
	m_nCallFPS(nFPS),
	m_bVideoQualityLowNotified(false),
	m_bVideoQualityHighNotified(false),
	m_bVideoShouldStopNotified(false),
	m_FriendID(llfriendID)

{

}

BitRateController::~BitRateController()
{

}

void BitRateController::SetCallFPS(int nFPS)
{
	m_nCallFPS = nFPS;
    m_nFrameCounterBeforeEncoding = nFPS;
}

void BitRateController::ResetVideoController()
{
	/*m_nBytesReceivedInMegaSlotInterval
	m_nSlotIntervalCounter(0),
	m_bMegSlotCounterShouldStop(false),
	m_bSetBitRateCalled(false),
	m_nPreviousByteRate(BITRATE_MAX / 8),
	m_nBytesSendInSlotInverval(0),
	m_nFrameCounterBeforeEncoding(0),
	m_nLastSendingSlot(0),
	m_dTotalDataByteInSlots(0.0),
	m_dAverageDataByteInSlots(0.0),
	m_nStopNotificationCounter(0),
	m_nOpponentNetworkType(NETWORK_TYPE_NOT_2G),
	m_bNetworkTypeMiniPacketReceived(false),
	m_nLastState(BITRATE_CHANGE_DOWN),
	m_nSpiralCounter(0),
	m_nUpCheckLimit(GOOD_MEGASLOT_TO_UP),
	m_nContinuousUpCounter(0),
	m_nContinuousUpCounterLimitToJump(1),
	m_bInMaxBitrate(false),
	m_dFirstTimeBitRateChangeFactor(BITRATE_DECREMENT_FACTOR),
	m_nOppNotifiedByterate(0),
	m_nMostRecentRespondedSlotNumber(-1),
	m_nGoodSlotCounter(0),
	m_nNormalSlotCounter(0),
	m_nSlotCounterToUp(0),
	m_dPreviousMegaSlotStatus(1),
	m_nOwnNetworkType(NETWORK_TYPE_NOT_2G),
	m_nGoodSlotCounterToUp(GOOD_MEGASLOT_TO_UP * GOOD_MEGASLOT_TO_UP_TOLERANCE)*/
}

void BitRateController::SetSharedObject(CCommonElementsBucket* pcSharedObject)
{
	m_pCommonElementsBucket = pcSharedObject;
}

void BitRateController::SetEncoder(CVideoEncoder* pcVideEnocder)
{
	m_pVideoEncoder = pcVideEnocder;
}

bool BitRateController::HandleNetworkTypeMiniPacket(CPacketHeader &crTempHeader)
{
	m_nOpponentNetworkType = crTempHeader.getTimeStamp();

    CLogPrinter_WriteSpecific5(CLogPrinter::INFO, "m_iNetworkType = " + m_Tools.IntegertoStringConvert(m_nOpponentNetworkType));

    m_bNetworkTypeMiniPacketReceived = true;

    m_pVideoEncoder->SetNetworkType(m_nOwnNetworkType || m_nOpponentNetworkType);
    SetOpponentNetworkType(m_nOpponentNetworkType);

	return true;
}


bool BitRateController::HandleBitrateMiniPacket(CPacketHeader &crTempHeader)
{
    CVideoCallSession* pVideoSession;
	bool bExist = m_pCommonElementsBucket->m_pVideoCallSessionList->IsVideoSessionExist(m_FriendID, pVideoSession);
    
    if(bExist && (pVideoSession->GetServiceType() == SERVICE_TYPE_LIVE_STREAM || pVideoSession->GetServiceType() == SERVICE_TYPE_SELF_STREAM))
    {
//    double __ratio = 100.00 * crTempHeader.getTimeStamp() * 8.00 /  m_pVideoEncoder->GetBitrate();
//    string __Message = "------------------------> Video BitRate: "+Tools::IntegertoStringConvert(m_pVideoEncoder->GetBitrate())
//    +"  Ratio: "+Tools::DoubleToString(__ratio)
//    +"  Size in Bit: "+Tools::IntegertoStringConvert(crTempHeader.getTimeStamp()*8);
//    LOGE( "%s", __Message.c_str());
      return false;
    }

    //printf("TheKing--> Bitrate MiniPacket Found\n");
    CLogPrinter_WriteSpecific5(CLogPrinter::INFO, " mini pkt found setting zero:////******");

	m_nOppNotifiedByterate = crTempHeader.getTimeStamp();

	if (m_BandWidthRatioHelper.find(crTempHeader.getFrameNumber()) == -1)
    {
		CLogPrinter_WriteSpecific5(CLogPrinter::INFO, "TheKing--> Not Found SLOT = " + m_Tools.IntegertoStringConvert(crTempHeader.getFrameNumber()));
        
        //printf("TheKing--> Bitrate MiniPacket slot not found\n");

		if (m_nLastSendingSlot <= crTempHeader.getFrameNumber())
        {
            m_bMegSlotCounterShouldStop = false;
        }

        return false;
    }

	int iSlotNumber = crTempHeader.getFrameNumber();

    m_nMostRecentRespondedSlotNumber = iSlotNumber;
    m_nBytesSendInMegaSlotInverval+=m_BandWidthRatioHelper.getElementAt(iSlotNumber);
	m_nBytesReceivedInMegaSlotInterval += crTempHeader.getTimeStamp();

    NeedToNotifyClient(m_nOppNotifiedByterate);

    if(m_nSlotIntervalCounter % MEGA_SLOT_INTERVAL == 0)
    {
        double dMegaRatio =  (m_nBytesReceivedInMegaSlotInterval *1.0) / (1.0 * m_nBytesSendInMegaSlotInverval) * 100.0;

       // printf("Theking--> &&&&&&&& Slot = %d, TotalSend = %d, TotalRecv = %d, MegaRatio = %lf\n", m_nSlotIntervalCounter, m_nBytesSendInMegaSlotInverval, m_nBytesReceivedInMegaSlotInterval,dMegaRatio);

        long long llTimeStampForMiniPkt = m_TimeDiffMapHelper[iSlotNumber];

		int nNeedToChange = NeedToChangeBitRate(dMegaRatio);
		int nTimeDifferenceBetweenMiniPackets = (int)(m_Tools.CurrentTimestamp() - llTimeStampForMiniPkt);

        string sMsg = " Minipacket -->  BR: "+ Tools::IntegertoStringConvert(m_pVideoEncoder->GetBitrate())
                      +" MBR: "+ Tools::IntegertoStringConvert(m_pVideoEncoder->GetMaxBitrate())
                      +" Send: "+Tools::IntegertoStringConvert(m_nBytesSendInMegaSlotInverval *8)
                      +" Rcv: "+Tools::IntegertoStringConvert(m_nBytesReceivedInMegaSlotInterval*8)
					  + " Change : " + Tools::IntegertoStringConvert(nNeedToChange) + " Ratio: " + m_Tools.DoubleToString(dMegaRatio)
                      /*+ "  B-Cross: "+ Tools::IntegertoStringConvert(m_pVideoEncoder->GetBitrate() - m_nBytesSendInMegaSlotInverval)
                      + "  M-Cross: "+ Tools::IntegertoStringConvert(m_pVideoEncoder->GetMaxBitrate() - m_nBytesSendInMegaSlotInverval)*/
                      +" SlotNo: " + Tools::IntegertoStringConvert(iSlotNumber) + " MiniPkt time delley: "+ m_Tools.IntegertoStringConvert(nTimeDifferenceBetweenMiniPackets);

        //CLogPrinter_WriteLog(CLogPrinter::DEBUGS, INSTENT_TEST_LOG, sMsg);
        //LOGE( "%s", sMsg.c_str());

        if(nNeedToChange == BITRATE_CHANGE_DOWN)
        {
            m_nOppNotifiedByterate = BITRATE_DECREMENT_FACTOR * (m_nBytesReceivedInMegaSlotInterval/MEGA_SLOT_INTERVAL);

            //printf("@@@@@@@@@, BITRATE_CHANGE_DOWN --> %d\n", m_nOppNotifiedByterate);

            m_bSetBitRateCalled = false;
            m_bMegSlotCounterShouldStop = true;
        }
        else if(nNeedToChange == BITRATE_CHANGE_UP)
        {
            m_nOppNotifiedByterate = m_nPreviousByteRate + BITRATE_INCREAMENT_DIFFERENCE/8;

            //printf("@@@@@@@@@, BITRATE_CHANGE_UP --> %d\n", m_nOppNotifiedByterate);

            m_bSetBitRateCalled = false;
            m_bMegSlotCounterShouldStop = true;
        }
		else if (nNeedToChange == BITRATE_CHANGE_UP_JUMP)
		{
			m_nOppNotifiedByterate = BITRATE_MAX / 8;

			m_bSetBitRateCalled = false;
			m_bMegSlotCounterShouldStop = true;
		}
        else
        {
            //printf("@@@@@@@@@, BITRATE_CHANGE_NO --> %d\n", m_nOppNotifiedByterate);
        }

        if(nTimeDifferenceBetweenMiniPackets > MAX_MINIPACKET_WAIT_TIME)
        {
            m_nOppNotifiedByterate = BITRATE_MIN / 8;
            CLogPrinter_WriteSpecific5(CLogPrinter::DEBUGS, " @@@@@@@@@, Minipacket BITRATE_CHANGE_DOWN --> "+ m_Tools.IntegertoStringConvert(nTimeDifferenceBetweenMiniPackets) + " SlotNo: " + Tools::IntegertoStringConvert(iSlotNumber));

            m_bSetBitRateCalled = false;
            m_bMegSlotCounterShouldStop = true;
        }
   
        m_nBytesReceivedInMegaSlotInterval = 0;
        m_nBytesSendInMegaSlotInverval = 0;
    }

    //printf("TheKing--> g_OppNotifiedByteRate = (%d, %d)\n", tempHeader.getFrameNumber(), tempHeader.getTimeStamp());

	double dRatio = (crTempHeader.getTimeStamp() *1.0) / (1.0 * m_BandWidthRatioHelper.getElementAt(crTempHeader.getFrameNumber())) * 100.0;

    //printf("Theking--> &&&&&&&& Loss Ratio = %lf\n", dRatio);

	m_BandWidthRatioHelper.eraseAllSmallerEqual(crTempHeader.getFrameNumber());

    return true;
}

bool BitRateController::UpdateBitrate()
{
    if(m_nFrameCounterBeforeEncoding<30)
        printf("Frame %d --> Encode Dequeue Time = %lld\n", m_nFrameCounterBeforeEncoding, m_Tools.CurrentTimestamp());
    
    ++m_nFrameCounterBeforeEncoding;
    
    
    
	if (m_nFrameCounterBeforeEncoding % m_nCallFPS == 0 && m_nOppNotifiedByterate>0 && m_bSetBitRateCalled == false)
    {
        int nRet = -1, nRet2 = -1;
        int nCurrentBitRate = m_nOppNotifiedByterate * 8  * m_dFirstTimeBitRateChangeFactor;

        m_dFirstTimeBitRateChangeFactor = 1;

        //printf("VampireEngg--> nCurrentBitRate = %d, g_OppNotifiedByteRate = %d\n", nCurrentBitRate, m_nOppNotifiedByterate);

        if(nCurrentBitRate < m_pVideoEncoder->GetBitrate())
        {
            nRet = m_pVideoEncoder->SetBitrate(nCurrentBitRate);

            if(nRet == 0) //First Initialization Successful
                nRet2 = m_pVideoEncoder->SetMaxBitrate(m_pVideoEncoder->GetBitrate());
        }
        else
        {
            nRet = m_pVideoEncoder->SetMaxBitrate(nCurrentBitRate);

            if(nRet == 0) //First Initialization Successful
                nRet2 = m_pVideoEncoder->SetBitrate(nCurrentBitRate);
        }

        if(nRet == 0 && nRet2 ==0) //We are intentionally skipping status of setbitrate operation success
        {
            m_nPreviousByteRate = m_pVideoEncoder->GetBitrate()/8;

            m_bMegSlotCounterShouldStop = false;
        }

        m_bSetBitRateCalled = true;

        return true;
    }

    return false;
}

void BitRateController::NotifyEncodedFrame(int &nrFrameSize)
{
	if (0 == nrFrameSize)
    {
        CLogPrinter_WriteSpecific4(CLogPrinter::DEBUGS, "BR~  Failed :"+ Tools::IntegertoStringConvert(m_nFrameCounterBeforeEncoding) +" Mod : "+ Tools::IntegertoStringConvert(m_nFrameCounterBeforeEncoding % 8));
    }

	m_nBytesSendInSlotInverval += nrFrameSize;

	if (m_nFrameCounterBeforeEncoding % m_nCallFPS == 0)
    {        
		int iRatioHelperIndex = (m_nFrameCounterBeforeEncoding - m_nCallFPS) / m_nCallFPS;

        if(m_bMegSlotCounterShouldStop == false)
        {
            CLogPrinter_WriteSpecific5(CLogPrinter::DEBUGS, "VampireEngg--> *************** m_nBytesSendInSlotInverval = ("+m_Tools.IntegertoStringConvert(iRatioHelperIndex)+
                                                            ",   "+m_Tools.IntegertoStringConvert(m_nBytesSendInSlotInverval)+")" );
            m_nLastSendingSlot = iRatioHelperIndex;
            m_BandWidthRatioHelper.insert(iRatioHelperIndex,  m_nBytesSendInSlotInverval);
            
            
            CLogPrinter_WriteLog(CLogPrinter::INFO, INSTENT_TEST_LOG, "SendingSide: SlotIndex = " + m_Tools.IntegertoStringConvert(iRatioHelperIndex) + ", SendBytes = " + m_Tools.IntegertoStringConvert(m_nBytesSendInSlotInverval));

            
            m_TimeDiffMapHelper[iRatioHelperIndex] =  m_Tools.CurrentTimestamp();
        }

        m_nBytesSendInSlotInverval = 0;
    }
}

void BitRateController::SetInitialBitrate()
{
	if (GetOpponentNetworkType() == NETWORK_TYPE_2G || GetOwnNetworkType() == NETWORK_TYPE_2G)
	{

		m_pVideoEncoder->SetBitrate(BITRATE_BEGIN_FOR_2G);
		m_pVideoEncoder->SetMaxBitrate(BITRATE_MAX_FOR_2G);

	}
	else
	{
		m_pVideoEncoder->SetBitrate(BITRATE_BEGIN);
		m_pVideoEncoder->SetMaxBitrate(BITRATE_BEGIN);

	}
}

int BitRateController::NeedToChangeBitRate(double dDataReceivedRatio)
{
    m_nSlotCounterToUp++;

	CLogPrinter_WriteLog(CLogPrinter::INFO, BITRATE_CHNANGE_LOG, "#BR~  SLOT: " + Tools::IntegertoStringConvert(m_nMostRecentRespondedSlotNumber) + "  BitRate :" + Tools::IntegertoStringConvert(m_pVideoEncoder->GetBitrate()) + "  Ratio: " + Tools::DoubleToString(dDataReceivedRatio));
    
	if(m_nSlotCounterToUp >= GOOD_MEGASLOT_TO_UP_SAFE)
        m_nUpCheckLimit = GOOD_MEGASLOT_TO_UP;

	if (dDataReceivedRatio < NORMAL_BITRATE_RATIO_IN_MEGA_SLOT)
    {
        m_nGoodSlotCounter = 0;
        m_nNormalSlotCounter = 0;
        m_nSlotCounterToUp = 0;					// should be placed down

		m_dPreviousMegaSlotStatus = dDataReceivedRatio;

		if (m_nLastState == BITRATE_CHANGE_UP && m_nSlotCounterToUp <= NUMBER_OF_WAIT_SLOT_TO_DETECT_UP_FAIL)
			m_nSpiralCounter++;
		else if (m_nLastState == BITRATE_CHANGE_DOWN && m_nSlotCounterToUp > NUMBER_OF_WAIT_SLOT_TO_DETECT_UP_FAIL)
		{
			m_nSpiralCounter = 0;
			m_nUpCheckLimit = GOOD_MEGASLOT_TO_UP;
		}

        if(m_nSpiralCounter >= NUMBER_OF_SPIRAL_LIMIT)
            m_nUpCheckLimit = GOOD_MEGASLOT_TO_UP_SAFE;

        m_nLastState = BITRATE_CHANGE_DOWN;
		m_bInMaxBitrate = false;

        return BITRATE_CHANGE_DOWN;
    }
	else if (dDataReceivedRatio >= NORMAL_BITRATE_RATIO_IN_MEGA_SLOT && dDataReceivedRatio <= GOOD_BITRATE_RATIO_IN_MEGA_SLOT)
    {
        m_nNormalSlotCounter++;
    }
	else if (dDataReceivedRatio > GOOD_BITRATE_RATIO_IN_MEGA_SLOT)
    {
        m_nGoodSlotCounter++;
    }
    else
    {

    }

    if(m_nSlotCounterToUp >= m_nUpCheckLimit)
    {
		CLogPrinter_WriteLog(CLogPrinter::INFO, BITRATE_CHNANGE_LOG ,"BITRATE_CHANGE_UP called");

		if (m_nGoodSlotCounter >= m_nGoodSlotCounterToUp && m_dPreviousMegaSlotStatus>GOOD_BITRATE_RATIO_IN_MEGA_SLOT)
        {
            m_nGoodSlotCounter = 0;
            m_nNormalSlotCounter = 0;
            m_nSlotCounterToUp = 0;

			m_dPreviousMegaSlotStatus = dDataReceivedRatio;

			if (m_nLastState == BITRATE_CHANGE_UP)
			{
				m_nSpiralCounter = 0;
				m_nContinuousUpCounter++;

				CLogPrinter_WriteLog(CLogPrinter::INFO, BITRATE_CHNANGE_LOG ,"previous was BITRATE_CHANGE_UP");
			}
			else
				m_nContinuousUpCounter = 0;

			m_nLastState = BITRATE_CHANGE_UP;

			if (m_nContinuousUpCounterLimitToJump <= m_nContinuousUpCounter && m_bInMaxBitrate == false)
			{
				CLogPrinter_WriteLog(CLogPrinter::INFO, BITRATE_CHNANGE_LOG ,"Time to jump bitrate");

				m_nContinuousUpCounterLimitToJump = GOOD_MEGASLOT_TO_UP_LIMIT_TO_BITRATE_JUMP;
				m_nContinuousUpCounter = 0;
				m_bInMaxBitrate = true;

				return BITRATE_CHANGE_UP_JUMP;
			}
			else if (m_bInMaxBitrate == false)
				return BITRATE_CHANGE_UP;
			else
				return BITRATE_CHANGE_NO;
        }
    }

	m_dPreviousMegaSlotStatus = dDataReceivedRatio;

    return BITRATE_CHANGE_NO;
}

int BitRateController::NeedToNotifyClient(int nCurrentByte)
{
    m_nSlotIntervalCounter++;
    m_dTotalDataByteInSlots+=m_nOppNotifiedByterate;
    m_dAverageDataByteInSlots = (m_dTotalDataByteInSlots * 1.0) / (m_nSlotIntervalCounter * 1.0);
   // printf("TheKing--> m_fAverageByteData = %lf\n", m_dAverageDataByteInSlots);
    
	nCurrentByte *= 8;

    printf("TheKing--> iCurrentBits = %d\n", nCurrentByte);

	if (nCurrentByte<BITRATE_LOW && nCurrentByte >= BITRATE_MIN)
    {
        m_nStopNotificationCounter = 0;

		if (false == m_bVideoQualityLowNotified)
		{
			m_pCommonElementsBucket->m_pEventNotifier->fireNetworkStrengthNotificationEvent(m_FriendID, CEventNotifier::NETWORK_STRENTH_GOOD);

			m_bVideoQualityLowNotified = true;
			m_bVideoQualityHighNotified = false;
			m_bVideoShouldStopNotified = false;
		}
    }
	else if (nCurrentByte<BITRATE_MIN)
    {
        m_nStopNotificationCounter++;

		if (false == m_bVideoShouldStopNotified && m_nStopNotificationCounter >= STOP_NOTIFICATION_SENDING_COUNTER)
        {
			m_pCommonElementsBucket->m_pEventNotifier->fireNetworkStrengthNotificationEvent(m_FriendID, CEventNotifier::NETWORK_STRENTH_BAD);
			//m_pCommonElementsBucket->m_pEventNotifier->fireVideoNotificationEvent(m_FriendID, CEventNotifier::VIDEO_SHOULD_STOP);

			m_bVideoShouldStopNotified = true;
			m_bVideoQualityHighNotified = false;
			m_bVideoQualityLowNotified = false;
        }
    }
	else if (nCurrentByte >= BITRATE_LOW)
	{
		m_nStopNotificationCounter = 0;

		if (false == m_bVideoQualityHighNotified)
		{
			m_pCommonElementsBucket->m_pEventNotifier->fireNetworkStrengthNotificationEvent(m_FriendID, CEventNotifier::NETWORK_STRENTH_EXCELLENT);

			m_bVideoQualityHighNotified = true;
			m_bVideoQualityLowNotified = false;
			m_bVideoShouldStopNotified = false;
		}
	}  
    
    if(m_nSlotIntervalCounter%BITRATE_AVERAGE_TIME == 0)
    {
        m_nSlotIntervalCounter = 0;
        m_dTotalDataByteInSlots=0;
        m_dAverageDataByteInSlots = 0;
    }
    
    return 0;
}

int BitRateController::GetOpponentNetworkType()
{
	return m_nOpponentNetworkType;
}

void BitRateController::SetOpponentNetworkType(int nNetworkType){
	m_nOpponentNetworkType = nNetworkType;
}

int BitRateController::GetOwnNetworkType()
{
	return m_nOwnNetworkType;
}

void BitRateController::SetOwnNetworkType(int nNetworkType)
{
	m_nOwnNetworkType = nNetworkType;
}

bool BitRateController::IsNetworkTypeMiniPacketReceived()
{
	return m_bNetworkTypeMiniPacketReceived;
}
