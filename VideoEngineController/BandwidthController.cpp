//
// Created by ipvision on 2/2/2016.
//

#include "BandwidthController.h"
#include "Tools.h"
#include "LogPrinter.h"

BandwidthController::BandwidthController(){
    m_LastTimeStamp = 0;
    m_nDataToSendInByte = 0;
    m_BandWidth = BAND_WIDTH_IN_BYTE;
    m_TimePeriodListSize = 0;
    m_NextIntervalStartTime = (1LL<<30);
}

BandwidthController::~BandwidthController(){

}


void BandwidthController::SetTimeInterval(std::vector<int>&BandWidthList, std::vector<int>&TimePeriod)
{
    m_BandWidthList = BandWidthList;
    m_TimePeriod = TimePeriod;
    m_TimePeriodListSize = BandWidthList.size();
    if(m_TimePeriodListSize > TimePeriod.size())
        m_TimePeriodListSize = TimePeriod.size();

//    CLogPrinter_WriteSpecific2(CLogPrinter::DEBUGS, "$$$!@# BandWidth Controller: BandWidthSwitching# Bandwidth: "+Tools::IntegertoStringConvert(m_TimePeriodListSize));
    if(m_TimePeriodListSize) {
        return;
    }

    m_iNextTimePeriodIndex = 0;

    m_BandWidth = m_BandWidthList[m_iNextTimePeriodIndex];
    m_NextIntervalStartTime= m_Tools.CurrentTimestamp() + m_TimePeriod[m_iNextTimePeriodIndex];

    ++m_iNextTimePeriodIndex;
    if(m_iNextTimePeriodIndex == m_TimePeriodListSize)
        m_iNextTimePeriodIndex = 0;
}

bool BandwidthController::IsSendeablePacket(int nPacketLen){
    nPacketLen += UDP_HEADER_SIZE;

    long long CurrentTime = m_Tools.CurrentTimestamp();
    long long timeDiff = CurrentTime - m_LastTimeStamp;
    m_LastTimeStamp=CurrentTime;
    int decrease = (int)(timeDiff * m_BandWidth / 1000.0);
    m_nDataToSendInByte -=  decrease;

//    CLogPrinter_WriteSpecific2(CLogPrinter::DEBUGS, "$$$!@# DataToSend: "+Tools::IntegertoStringConvert(m_nDataToSendInByte)+ " Tdif: "+Tools::IntegertoStringConvert(timeDiff)
//                                                    + " Decr : "+Tools::IntegertoStringConvert(decrease)
//                                                                 + " BandWidth : "+Tools::IntegertoStringConvert(m_BandWidth));
    if(m_nDataToSendInByte < 0)
        m_nDataToSendInByte = 0;

    if(m_TimePeriodListSize>0 && CurrentTime >= m_NextIntervalStartTime) {                    //Interval Switching
        m_BandWidth = m_BandWidthList[m_iNextTimePeriodIndex];
        m_NextIntervalStartTime = CurrentTime + m_TimePeriod[m_iNextTimePeriodIndex];
        ++m_iNextTimePeriodIndex;
        if (m_iNextTimePeriodIndex == m_TimePeriodListSize)
            m_iNextTimePeriodIndex = 0;
        m_nDataToSendInByte = 0;
//        CLogPrinter_WriteSpecific2(CLogPrinter::DEBUGS, "$$$!@# ##########################BandWidth Controller: BandWidthSwitching# Bandwidth: "+Tools::IntegertoStringConvert(m_BandWidth));
    }

    if( m_nDataToSendInByte + nPacketLen <= ROUTER_BUFFER_SIZE)
    {
        m_nDataToSendInByte += nPacketLen;
        return true;
    }
//    CLogPrinter_WriteSpecific2(CLogPrinter::DEBUGS, "$$$!@# Failed");
    return false;
}