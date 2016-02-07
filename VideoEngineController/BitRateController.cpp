//
// Created by ipvision on 2/6/2016.
//

#include "BitRateController.h"
#include "PacketHeader.h"
#include "Size.h"
#include "LogPrinter.h"


BitRateController::BitRateController():
    m_ByteRecvInMegaSlotInterval(0),
    m_SlotIntervalCounter(0),
    m_bMegSlotCounterShouldStop(true),
    m_bsetBitrateCalled(false),
    m_iPreviousByterate(BITRATE_MAX/8),
    m_bGotOppBandwidth(0),
    m_ByteSendInSlotInverval(0),
    m_FrameCounterbeforeEncoding(0),
    m_LastSendingSlot(0)
{
    nFirstTimeDecrease = 100000;
    m_OppNotifiedByterate = 0;

    m_iGoodSlotCounter = 0;
    m_iNormalSlotCounter = 0;
    m_SlotCounter = 0;
    m_PrevMegaSlotStatus = 1;
}

BitRateController::~BitRateController(){

}

void BitRateController::SetEncoder(CVideoEncoder* pVideEnocder){
    m_pVideoEncoder = pVideEnocder;
}

bool BitRateController::HandleBitrateMiniPacket(CPacketHeader &tempHeader){
    CLogPrinter_WriteSpecific2(CLogPrinter::INFO, "BR~ <|> BITRATE");

    int packetNumber = tempHeader.getPacketNumber();

//    m_bGotOppBandwidth++;
    m_OppNotifiedByterate = tempHeader.getTimeStamp();

    if(m_BandWidthRatioHelper.find(tempHeader.getFrameNumber()) == m_BandWidthRatioHelper.end())
    {
        ////printf("TheKing--> Not Found SLOT = %d\n", tempHeader.getFrameNumber());
        if(m_LastSendingSlot<=tempHeader.getFrameNumber())
        {
            m_bMegSlotCounterShouldStop = false;
        }
        return false;
    }
    int iSlotNumber = tempHeader.getFrameNumber();
    m_ByteSendInMegaSlotInverval+=m_BandWidthRatioHelper.getElementAt(iSlotNumber);
    m_ByteRecvInMegaSlotInterval+=tempHeader.getTimeStamp();
    m_SlotIntervalCounter++;
    if(m_SlotIntervalCounter % MEGA_SLOT_INTERVAL == 0)
    {
        double MegaRatio =  (m_ByteRecvInMegaSlotInterval *1.0) / (1.0 * m_ByteSendInMegaSlotInverval) * 100.0;

        //printf("Theking--> &&&&&&&& MegaSlot = %d, TotalSend = %d, TotalRecv = %d, MegaRatio = %lf\n", m_SlotIntervalCounter, m_ByteSendInMegaSlotInverval, m_ByteRecvInMegaSlotInterval,MegaRatio);



        int iNeedToChange = NeedToChangeBitRate(MegaRatio);

        CLogPrinter_WriteSpecific4(CLogPrinter::DEBUGS, "BR~  BR: "+ Tools::IntegertoStringConvert(m_pVideoEncoder->GetBitrate())
                                                        +" MBR: "+ Tools::IntegertoStringConvert(m_pVideoEncoder->GetMaxBitrate())
                                                        +" Send: "+Tools::IntegertoStringConvert(m_ByteSendInMegaSlotInverval *8)
                                                        +" Rcv: "+Tools::IntegertoStringConvert(m_ByteRecvInMegaSlotInterval*8)
                                                        +" Change : "+Tools::IntegertoStringConvert(iNeedToChange)+ " Ratio: "+m_Tools.DoubleToString(MegaRatio)
                                                        + "  B-Cross: "+ Tools::IntegertoStringConvert(m_pVideoEncoder->GetBitrate() - m_ByteSendInMegaSlotInverval)
                                                        + "  M-Cross: "+ Tools::IntegertoStringConvert(m_pVideoEncoder->GetMaxBitrate() - m_ByteSendInMegaSlotInverval)
        +" SlotNo: "+Tools::IntegertoStringConvert(iSlotNumber));

        if(iNeedToChange == BITRATE_CHANGE_DOWN)
        {
            m_OppNotifiedByterate = BITRATE_DECREMENT_FACTOR * (m_ByteRecvInMegaSlotInterval/MEGA_SLOT_INTERVAL);

            //printf("@@@@@@@@@, BITRATE_CHANGE_DOWN --> %d\n", g_OppNotifiedByterate);

            m_bsetBitrateCalled = false;
            m_bMegSlotCounterShouldStop = true;
        }
        else if(iNeedToChange == BITRATE_CHANGE_UP)
        {

            m_OppNotifiedByterate = m_iPreviousByterate * BITRATE_INCREAMENT_FACTOR;

            //printf("@@@@@@@@@, BITRATE_CHANGE_UP --> %d\n", m_OppNotifiedByterate);

            m_bsetBitrateCalled = false;
            m_bMegSlotCounterShouldStop = true;
        }
        else
        {
            //printf("@@@@@@@@@, BITRATE_CHANGE_NO --> %d\n", m_OppNotifiedByterate);
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
        int iCurrentBitRate = m_OppNotifiedByterate* 8 - nFirstTimeDecrease;
        nFirstTimeDecrease = 0;

        //printf("VampireEngg--> iCurrentBitRate = %d, g_OppNotifiedByteRate = %d\n", iCurrentBitRate, m_OppNotifiedByterate);

        if(iCurrentBitRate < m_pVideoEncoder->GetBitrate())
        {
            iRet = m_pVideoEncoder->SetBitrate(iCurrentBitRate);

            if(iRet == 0) //First Initialization Successful
                iRet2 = m_pVideoEncoder->SetMaxBitrate(iCurrentBitRate);

        }
        else
        {
            iRet = m_pVideoEncoder->SetMaxBitrate(iCurrentBitRate);

            if(iRet == 0) //First Initialization Successful
                iRet2 = m_pVideoEncoder->SetBitrate(iCurrentBitRate);
        }

        if(iRet == 0 && iRet2 ==0) //We are intentionally skipping status of setbitrate operation success
        {
            m_iPreviousByterate = min(BITRATE_MAX ,iCurrentBitRate)/8;

            m_bMegSlotCounterShouldStop = false;
        }

        m_bsetBitrateCalled = true;
        return true;
    }
    return false;
}

void BitRateController::NotifyEncodedFrame(int nFrameSize){

    if(0 == nFrameSize)
    {
        CLogPrinter_WriteSpecific4(CLogPrinter::DEBUGS, "BR~  Failed :"+ Tools::IntegertoStringConvert(m_FrameCounterbeforeEncoding)
                                                        +" Mod : "+ Tools::IntegertoStringConvert(m_FrameCounterbeforeEncoding % 8));
    }
    m_ByteSendInSlotInverval+=nFrameSize;
    if(m_FrameCounterbeforeEncoding % FRAME_RATE == 0)
    {
        int ratioHelperIndex = (m_FrameCounterbeforeEncoding - FRAME_RATE) / FRAME_RATE;
        if(m_bMegSlotCounterShouldStop == false)
        {
            //printf("VampireEngg--> ***************m_ByteSendInSlotInverval = (%d, %d)\n", ratioHelperIndex, m_ByteSendInSlotInverval);
            m_LastSendingSlot = ratioHelperIndex;
            m_BandWidthRatioHelper.insert(ratioHelperIndex, m_ByteSendInSlotInverval);
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