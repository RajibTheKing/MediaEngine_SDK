//
// Created by ipvision on 2/6/2016.
//

#include "BitRateController.h"
#include "PacketHeader.h"
#include "Size.h"
#include "LogPrinter.h"
#include "CommonElementsBucket.h"


double m_fTotalDataInSlots;
double m_fAverageData;

BitRateController::BitRateController():
    m_ByteRecvInMegaSlotInterval(0),
    m_SlotIntervalCounter(0),
    m_bMegSlotCounterShouldStop(false),
    m_bsetBitrateCalled(false),
    m_iPreviousByterate(BITRATE_MAX/8),
    m_bGotOppBandwidth(0),
    m_ByteSendInSlotInverval(0),
    m_FrameCounterbeforeEncoding(0),
    m_LastSendingSlot(0),
    m_fTotalDataInSlots(0.0),
    m_fAverageData(0.0),
    m_iStopNotificationController(0),
    m_iOpponentNetworkType(NETWORK_TYPE_NOT_2G),
    m_iNetTypeMiniPktRcv(0)
{
    dFirstTimeDecrease = BITRATE_DECREMENT_FACTOR;
    m_OppNotifiedByterate = 0;

    m_iGoodSlotCounter = 0;
    m_iNormalSlotCounter = 0;
    m_SlotCounter = 0;
    m_PrevMegaSlotStatus = 1;

    m_bIsFirstTime = true;
    timeDiffForMiniPkt = 0;
    m_bIsFirstMiniPktRcv = false;
    m_lTimeStampForFirstMiniPkt = 0;
    m_iOwnNetworkType = NETWORK_TYPE_NOT_2G;
    //m_iWaititngForFirstMiniPkt = -1;
}

BitRateController::~BitRateController(){

}

void BitRateController::SetSharedObject(CCommonElementsBucket* sharedObject)
{
    m_pCommonElementsBucket = sharedObject;
}

void BitRateController::SetEncoder(CVideoEncoder* pVideEnocder){
    m_pVideoEncoder = pVideEnocder;
}

bool BitRateController::HandleNetworkTypeMiniPacket(CPacketHeader &tempHeader)
{
    m_iOpponentNetworkType = tempHeader.getTimeStamp();
    CLogPrinter_WriteSpecific5(CLogPrinter::INFO, "m_iNetworkType = " + m_Tools.IntegertoStringConvert(
            m_iOpponentNetworkType));
    m_iNetTypeMiniPktRcv = 1;
}


bool BitRateController::HandleBitrateMiniPacket(CPacketHeader &tempHeader)
{
    CLogPrinter_WriteSpecific5(CLogPrinter::INFO, " mini pkt found setting zero:////******");


    int packetNumber = tempHeader.getPacketNumber();
    //m_iWaititngForFirstMiniPkt = 0;
//    m_bGotOppBandwidth++;
    m_OppNotifiedByterate = tempHeader.getTimeStamp();

    if(m_BandWidthRatioHelper.find(tempHeader.getFrameNumber()) == m_BandWidthRatioHelper.end())
    {
        CLogPrinter_WriteSpecific5(CLogPrinter::INFO, "TheKing--> Not Found SLOT = " + m_Tools.IntegertoStringConvert(tempHeader.getFrameNumber()));
        if(m_LastSendingSlot<=tempHeader.getFrameNumber())
        {
            m_bMegSlotCounterShouldStop = false;
        }
        return false;
    }
    int iSlotNumber = tempHeader.getFrameNumber();
    m_ByteSendInMegaSlotInverval+=m_BandWidthRatioHelper.getElementAt(iSlotNumber);
    m_ByteRecvInMegaSlotInterval+=tempHeader.getTimeStamp();

    NeedToNotifyClient(m_OppNotifiedByterate);



    if(m_SlotIntervalCounter % MEGA_SLOT_INTERVAL == 0)
    {
        double MegaRatio =  (m_ByteRecvInMegaSlotInterval *1.0) / (1.0 * m_ByteSendInMegaSlotInverval) * 100.0;

        //printf("Theking--> &&&&&&&& MegaSlot = %d, TotalSend = %d, TotalRecv = %d, MegaRatio = %lf\n", m_SlotIntervalCounter, m_ByteSendInMegaSlotInverval, m_ByteRecvInMegaSlotInterval,MegaRatio);

        m_lTimeStampForMiniPkt = m_TimeDiffMapHelper[iSlotNumber];

        int iNeedToChange = NeedToChangeBitRate(MegaRatio);
        int timeDiffForMinPkt = (int)(m_Tools.CurrentTimestamp() - m_lTimeStampForMiniPkt);




        string sMsg = " Minipacket -->  BR: "+ Tools::IntegertoStringConvert(m_pVideoEncoder->GetBitrate())
                      +" MBR: "+ Tools::IntegertoStringConvert(m_pVideoEncoder->GetMaxBitrate())
                      +" Send: "+Tools::IntegertoStringConvert(m_ByteSendInMegaSlotInverval *8)
                      +" Rcv: "+Tools::IntegertoStringConvert(m_ByteRecvInMegaSlotInterval*8)
                      +" Change : "+Tools::IntegertoStringConvert(iNeedToChange)+ " Ratio: "+m_Tools.DoubleToString(MegaRatio)
                      /*+ "  B-Cross: "+ Tools::IntegertoStringConvert(m_pVideoEncoder->GetBitrate() - m_ByteSendInMegaSlotInverval)
                      + "  M-Cross: "+ Tools::IntegertoStringConvert(m_pVideoEncoder->GetMaxBitrate() - m_ByteSendInMegaSlotInverval)*/
                      +" SlotNo: " + Tools::IntegertoStringConvert(iSlotNumber) + " MiniPkt time delley: "+ m_Tools.IntegertoStringConvert(timeDiffForMinPkt);




        CLogPrinter_WriteSpecific5(CLogPrinter::DEBUGS, sMsg );

        if(iNeedToChange == BITRATE_CHANGE_DOWN)
        {
            m_OppNotifiedByterate = BITRATE_DECREMENT_FACTOR * (m_ByteRecvInMegaSlotInterval/MEGA_SLOT_INTERVAL);

            //printf("@@@@@@@@@, BITRATE_CHANGE_DOWN --> %d\n", m_OppNotifiedByterate);

            m_bsetBitrateCalled = false;
            m_bMegSlotCounterShouldStop = true;
        }
        else if(iNeedToChange == BITRATE_CHANGE_UP)
        {

            //m_OppNotifiedByterate = m_iPreviousByterate * BITRATE_INCREAMENT_FACTOR;
            m_OppNotifiedByterate = m_iPreviousByterate + BITRATE_INCREAMENT_DIFFERENCE/8;

            //printf("@@@@@@@@@, BITRATE_CHANGE_UP --> %d\n", m_OppNotifiedByterate);

            m_bsetBitrateCalled = false;
            m_bMegSlotCounterShouldStop = true;
        }
        else
        {
            //printf("@@@@@@@@@, BITRATE_CHANGE_NO --> %d\n", m_OppNotifiedByterate);
        }

        if(timeDiffForMinPkt > MAX_MINIPACKET_WAIT_TIME)
        {
            m_OppNotifiedByterate = BITRATE_MIN / 8;
            CLogPrinter_WriteSpecific5(CLogPrinter::DEBUGS, " @@@@@@@@@, Minipacket BITRATE_CHANGE_DOWN --> "+ m_Tools.IntegertoStringConvert(timeDiffForMinPkt) + " SlotNo: " + Tools::IntegertoStringConvert(iSlotNumber));

            m_bsetBitrateCalled = false;
            m_bMegSlotCounterShouldStop = true;

        }
        /*
        if(timeDiffForMinPkt < MAX_1ST_MINIPACKET_WAIT_TIME)
        {
            if(m_bIsFirstTime)
            {
                m_OppNotifiedByterate = BITRATE_MID / 8;
                CLogPrinter_WriteSpecific5(CLogPrinter::DEBUGS, "1st Minipacket arrived timely --> "+ m_Tools.IntegertoStringConvert(timeDiffForMinPkt)+"  SlotNo: " + Tools::IntegertoStringConvert(iSlotNumber));

                m_bsetBitrateCalled = false;
                m_bMegSlotCounterShouldStop = true;
            }
        }
        else
        {
            if(m_bIsFirstTime)
            {
                CLogPrinter_WriteSpecific5(CLogPrinter::DEBUGS, "1st Minipacket arrived LATE --> "+ m_Tools.IntegertoStringConvert(timeDiffForMinPkt)+"  SlotNo: " + Tools::IntegertoStringConvert(iSlotNumber));
            }
        }
        */

        if(m_bIsFirstTime)
        {
            m_bIsFirstTime = false;
        }
        m_ByteRecvInMegaSlotInterval = 0;
        m_ByteSendInMegaSlotInverval = 0;

    }

    //printf("TheKing--> g_OppNotifiedByteRate = (%d, %d)\n", tempHeader.getFrameNumber(), tempHeader.getTimeStamp());

    double ratio =  (tempHeader.getTimeStamp() *1.0) / (1.0 * m_BandWidthRatioHelper.getElementAt(tempHeader.getFrameNumber())) * 100.0;

    //printf("Theking--> &&&&&&&& Loss Ratio = %lf\n", ratio);

    m_BandWidthRatioHelper.erase(tempHeader.getFrameNumber());

    return true;
}

bool BitRateController::UpdateBitrate()
{
    ++m_FrameCounterbeforeEncoding;

    if(m_FrameCounterbeforeEncoding%FRAME_RATE == 0 && m_OppNotifiedByterate>0 && m_bsetBitrateCalled == false)
    {
        int iRet = -1, iRet2 = -1;
        int iCurrentBitRate = m_OppNotifiedByterate * 8  * dFirstTimeDecrease;
        dFirstTimeDecrease = 1;

        //printf("VampireEngg--> iCurrentBitRate = %d, g_OppNotifiedByteRate = %d\n", iCurrentBitRate, m_OppNotifiedByterate);

        if(iCurrentBitRate < m_pVideoEncoder->GetBitrate())
        {
            iRet = m_pVideoEncoder->SetBitrate(iCurrentBitRate);

            if(iRet == 0) //First Initialization Successful
                iRet2 = m_pVideoEncoder->SetMaxBitrate(m_pVideoEncoder->GetBitrate());
        }
        else
        {
            iRet = m_pVideoEncoder->SetMaxBitrate(iCurrentBitRate);

            if(iRet == 0) //First Initialization Successful
                iRet2 = m_pVideoEncoder->SetBitrate(iCurrentBitRate);
        }

        if(iRet == 0 && iRet2 ==0) //We are intentionally skipping status of setbitrate operation success
        {
            m_iPreviousByterate = m_pVideoEncoder->GetBitrate()/8;

            m_bMegSlotCounterShouldStop = false;
        }

        m_bsetBitrateCalled = true;
        return true;
    }
    return false;
}

void BitRateController::NotifyEncodedFrame(int &nFrameSize){

    if(0 == nFrameSize)
    {
        CLogPrinter_WriteSpecific4(CLogPrinter::DEBUGS, "BR~  Failed :"+ Tools::IntegertoStringConvert(m_FrameCounterbeforeEncoding)
                                                        +" Mod : "+ Tools::IntegertoStringConvert(m_FrameCounterbeforeEncoding % 8));
    }
    m_ByteSendInSlotInverval+=nFrameSize;
    if(m_FrameCounterbeforeEncoding % FRAME_RATE == 0)
    {
#ifdef USE_FIXED_BITRATE_PER_SLOT
        int addsize = 0;
        if(m_ByteSendInSlotInverval < m_pVideoEncoder->GetBitrate() * FIXED_BITRATE_TOLERANCE / 8)
        {
            addsize = m_pVideoEncoder->GetBitrate() * FIXED_BITRATE_TOLERANCE / 8- m_ByteSendInSlotInverval;
            m_ByteSendInSlotInverval += addsize;
            nFrameSize += addsize;
        }
        printf("VampireEngg--> m_ByteSendInSlotIn = %d, encodedSize = %d, addsize = %d\n", m_ByteSendInSlotInverval, nFrameSize, addsize);
#endif
        
        int ratioHelperIndex = (m_FrameCounterbeforeEncoding - FRAME_RATE) / FRAME_RATE;
        if(m_bMegSlotCounterShouldStop == false)
        {

            CLogPrinter_WriteSpecific5(CLogPrinter::DEBUGS, "VampireEngg--> *************** m_ByteSendInSlotInverval = ("+m_Tools.IntegertoStringConvert(ratioHelperIndex)+
                                                            ",   "+m_Tools.IntegertoStringConvert(m_ByteSendInSlotInverval)+")" );
            m_LastSendingSlot = ratioHelperIndex;
            m_BandWidthRatioHelper.insert(ratioHelperIndex,  m_ByteSendInSlotInverval);
            m_TimeDiffMapHelper[ratioHelperIndex] =  m_Tools.CurrentTimestamp();

            /*if(m_iWaititngForFirstMiniPkt == -1)
            {
                CLogPrinter_WriteSpecific5(CLogPrinter::DEBUGS, "VampireEngg--> <><><><><><><> m_ByteSendInSlotInverval = ("+m_Tools.IntegertoStringConvert(ratioHelperIndex)+
                        ",   "+m_Tools.IntegertoStringConvert(m_ByteSendInSlotInverval)+")" );

                m_iWaititngForFirstMiniPkt = 1;
            }*/

        }

        m_ByteSendInSlotInverval = 0;
    }
}

int BitRateController::NeedToChangeBitRate(double dataReceivedRatio)
{
    m_SlotCounter++;

    if(dataReceivedRatio < NORMAL_BITRATE_RATIO_IN_MEGA_SLOT)
    {
        m_iGoodSlotCounter = 0;
        m_iNormalSlotCounter = 0;
        m_SlotCounter = 0;

        //m_iConsecutiveGoodMegaSlot = 0;
        m_PrevMegaSlotStatus = dataReceivedRatio;
        return BITRATE_CHANGE_DOWN;

    }
    else if(dataReceivedRatio>=NORMAL_BITRATE_RATIO_IN_MEGA_SLOT && dataReceivedRatio<=GOOD_BITRATE_RATIO_IN_MEGA_SLOT)
    {
        m_iNormalSlotCounter++;
    }
    else if(dataReceivedRatio > GOOD_BITRATE_RATIO_IN_MEGA_SLOT)
    {
        m_iGoodSlotCounter++;
        /*m_iConsecutiveGoodMegaSlot++;
        if(m_iConsecutiveGoodMegaSlot == GOOD_MEGASLOT_TO_UP)
        {
            m_iConsecutiveGoodMegaSlot = 0;
            return BITRATE_CHANGE_UP;
        }*/


    }
    else
    {
        //      m_iConsecutiveGoodMegaSlot = 0;
    }


    if(m_SlotCounter >= GOOD_MEGASLOT_TO_UP)
    {
        int temp = GOOD_MEGASLOT_TO_UP * 0.9;

        if(m_iGoodSlotCounter>=temp && m_PrevMegaSlotStatus>GOOD_BITRATE_RATIO_IN_MEGA_SLOT)
        {
            m_iGoodSlotCounter = 0;
            m_iNormalSlotCounter = 0;
            m_SlotCounter = 0;

            m_PrevMegaSlotStatus = dataReceivedRatio;

            return BITRATE_CHANGE_UP;
        }

    }

    m_PrevMegaSlotStatus = dataReceivedRatio;
    return BITRATE_CHANGE_NO;
}

int BitRateController::NeedToNotifyClient(int iCurrentByte)
{
    m_SlotIntervalCounter++;
    m_fTotalDataInSlots+=m_OppNotifiedByterate;
    m_fAverageData = (m_fTotalDataInSlots * 1.0) / (m_SlotIntervalCounter * 1.0);
   // printf("TheKing--> m_fAverageByteData = %lf\n", m_fAverageData);
    
    iCurrentByte*=8;
    printf("TheKing--> iCurrentBits = %d\n", iCurrentByte);
    if(iCurrentByte<BITRATE_LOW && iCurrentByte >= BITRATE_MIN)
    {
        m_iStopNotificationController = 0;

		if (false == m_bVideoQualityLowNotified)
		{
			m_pCommonElementsBucket->m_pEventNotifier->fireVideoNotificationEvent(200, CEventNotifier::VIDEO_QUALITY_LOW);

			m_bVideoQualityLowNotified = true;
			m_bVideoQualityHighNotified = false;
			m_bVideoShouldStopNotified = false;
		}
    }
    else if(iCurrentByte<BITRATE_MIN)
    {
        m_iStopNotificationController++;

		if (false == m_bVideoShouldStopNotified && m_iStopNotificationController >= STOP_NOTIFICATION_SENDING_COUNTER)
        {
			m_pCommonElementsBucket->m_pEventNotifier->fireVideoNotificationEvent(200, CEventNotifier::VIDEO_SHOULD_STOP);

			m_bVideoShouldStopNotified = true;
			m_bVideoQualityHighNotified = false;
			m_bVideoQualityLowNotified = false;
        }
    }
	else if (iCurrentByte >= BITRATE_LOW)
	{
		m_iStopNotificationController = 0;

		if (false == m_bVideoQualityHighNotified)
		{
			m_pCommonElementsBucket->m_pEventNotifier->fireVideoNotificationEvent(200, CEventNotifier::VIDEO_QUALITY_HIGH);

			m_bVideoQualityHighNotified = true;
			m_bVideoQualityLowNotified = false;
			m_bVideoShouldStopNotified = false;
		}
	}  
    
    if(m_SlotIntervalCounter%BITRATE_AVERAGE_TIME == 0)
    {
        m_SlotIntervalCounter = 0;
        m_fTotalDataInSlots=0;
        m_fAverageData = 0;
    }
    
    
    return 0;
}