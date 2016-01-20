//
// Created by ipvision on 1/2/2016.
//

#include "FPSController.h"
#include "Size.h"
#include <math.h>


CFPSController::CFPSController(){
    m_pMutex.reset(new CLockHandler);
    m_LastIntervalStartingTime = m_Tools.CurrentTimestamp();
    m_ClientFPS = m_nOwnFPS = m_nOpponentFPS = FPS_BEGINNING;
    m_iFrameDropIntervalCounter=0;
    m_EncodingFrameCounter = 0;
    m_DropSum = 0;

}

CFPSController::~CFPSController(){
    while(!m_SignalQue.empty())
        m_SignalQue.pop();
    SHARED_PTR_DELETE(m_pMutex);
}

void CFPSController::Reset(){
    m_LastIntervalStartingTime = m_Tools.CurrentTimestamp();
    m_ClientFPS = m_nOwnFPS = m_nOpponentFPS = FPS_BEGINNING;
    m_iFrameDropIntervalCounter=0;
    m_EncodingFrameCounter = 0;
    m_DropSum = 0;
}

int CFPSController::GetOpponentFPS() const {
    return m_nOpponentFPS;
}

void CFPSController::SetOpponentFPS(int OpponentFPS) {
    Locker lock(*m_pMutex);
    m_nOpponentFPS = OpponentFPS;
}

int CFPSController::GetOwnFPS() const {
    return m_nOwnFPS;
}

void CFPSController::SetOwnFPS(int nOwnFPS){
    Locker lock(*m_pMutex);
    m_nOwnFPS = nOwnFPS;
}

void CFPSController::SetClientFPS(double fps){
    Locker lock(*m_pMutex);
    m_ClientFPS = fps;
}

unsigned char CFPSController::GetFPSSignalByte()
{
    unsigned char ret = m_nOwnFPS;
    unsigned char changeSignal = 0;

    {
        Locker lock(*m_pMutex);
        int tmp = floor(m_ClientFPS+0.5);
        if(m_nOwnFPS > tmp)
            ret = tmp;
    }

    if(!m_SignalQue.empty()) {
        changeSignal = m_SignalQue.front();
        m_SignalQue.pop();
    }
    ret |= (changeSignal << 6);

//    CLogPrinter_WriteSpecific(CLogPrinter::DEBUGS, "# SIGNAL: ------------------------------------------------------------->   "+ m_Tools.IntegertoStringConvert((int)changeSignal)+"~"+ m_Tools.IntegertoStringConvert((int)ret));
    return ret;
}


void CFPSController::SetFPSSignalByte(unsigned char signalByte)
{
    signalByte &= 0xCF;
    if(0==signalByte)   return;

    int FPSChangeSignal = (signalByte>>6);
    int opponentFPS = 15&signalByte;

    if(opponentFPS != m_nOpponentFPS)
    {
        Locker lock(*m_pMutex);
        m_nOpponentFPS = opponentFPS;
    }

//    if(FPSChangeSignal)
//        CLogPrinter_WriteSpecific(CLogPrinter::DEBUGS, "# SIGNAL: -------------------------------------------------------SET------>   "+ m_Tools.IntegertoStringConvert(FPSChangeSignal));

    if(FPSChangeSignal == 1)
    {
        if(m_nOpponentFPS<MAX_DIFF_TO_DROP_FPS+m_nOwnFPS &&  m_nOwnFPS>FPS_MINIMUM)
        {
            Locker lock(*m_pMutex);
            m_nOwnFPS--;
        }
    }
    else if(FPSChangeSignal == 2) {
        if(m_nOwnFPS < FPS_MAXIMUM && m_nOwnFPS+1 < m_ClientFPS+0.1) {
            Locker lock(*m_pMutex);
            m_nOwnFPS++;
        }
    }
}

int CFPSController::NotifyFrameComplete(int framNumber)
{
//    m_iFrameCompletedIntervalCounter++;
    LongLong diffTimeStamp = m_Tools.CurrentTimestamp() - m_LastIntervalStartingTime;

    bool bIsIntervalOver = diffTimeStamp >= 3*1000;

    if( bIsIntervalOver && 0==m_iFrameDropIntervalCounter)
    {
        {
            Locker lock(*m_pMutex);
            m_nFPSChangeSignal = 2;
            m_SignalQue.push(2);
        }
        m_LastIntervalStartingTime = m_Tools.CurrentTimestamp();
//        CLogPrinter_WriteSpecific(CLogPrinter::DEBUGS, "PushPacketForDecoding:: @@@@@@@@@@@@@@@@@@@@@@@@@   FPS INCREASE  @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@");
    }
    else if(bIsIntervalOver)
        m_LastIntervalStartingTime = m_Tools.CurrentTimestamp();
    
    return 1;
}

int CFPSController::NotifyFrameDropped(int framNumber)
{
    m_iFrameDropIntervalCounter++;

//    CLogPrinter_WriteSpecific(CLogPrinter::DEBUGS, "PushPacketForDecoding:: FRAME DROP------------> "+m_Tools.IntegertoStringConvert(framNumber)+" CNT: "+m_Tools.IntegertoStringConvert(m_iFrameDropIntervalCounter));

    LongLong diffTimeStamp = m_Tools.CurrentTimestamp() - m_LastIntervalStartingTime;
    bool bIsIntervalOver = diffTimeStamp <= 3*1000;

    if(bIsIntervalOver && m_iFrameDropIntervalCounter>3)
    {
        {
            Locker lock(*m_pMutex);
            m_nFPSChangeSignal = 1;
            m_SignalQue.push(1);
        }
        m_LastIntervalStartingTime = m_Tools.CurrentTimestamp();

        m_iFrameDropIntervalCounter=0;
//        CLogPrinter_WriteSpecific(CLogPrinter::DEBUGS, "PushPacketForDecoding:: *************************   FPS DECREASING  ***********************************************************");
    }
    else if(!bIsIntervalOver)
    {
        m_LastIntervalStartingTime = m_Tools.CurrentTimestamp();
        m_iFrameDropIntervalCounter=0;
    }
    
    return 1;
}

bool CFPSController::IsProcessableFrame()
{
    Tools tools;

//	CLogPrinter_WriteSpecific(CLogPrinter::DEBUGS, "PushPacketForDecoding:: ClientFPS "+tools.DoubleToString(m_ClientFPS));

    if(m_nOwnFPS+FPS_COMPARISON_EPS > m_ClientFPS) return true;

    double diff = m_ClientFPS - m_nOwnFPS;

    double ratio = (double)(m_ClientFPS*1.0)/(diff*1.0);

    if(m_EncodingFrameCounter == 0)
    {
        m_DropSum+=ratio;
    }

    int indx = floor(m_DropSum + 0.5);
    m_EncodingFrameCounter++;

    if(m_EncodingFrameCounter == indx)
    {
        m_DropSum+=ratio;
//		CLogPrinter_WriteSpecific(CLogPrinter::INFO, "PushPacketForDecoding -> Indx = "+m_Tools.IntegertoStringConvert(indx) + "  ClientFPS: " +m_Tools.IntegertoStringConvert((int)m_ClientFPS)
//                                                      +"m_nOwnFPS = " + m_Tools.IntegertoStringConvert(m_nOwnFPS)+ ",  m_DropSum =" + m_Tools.DoubleToString(m_DropSum));
        return false;
    }

    return true;
}
