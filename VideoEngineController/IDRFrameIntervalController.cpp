
#include "IDRFrameIntervalController.h"
#include "LogPrinter.h"
#include "CommonElementsBucket.h"

IDRFrameIntervalController::IDRFrameIntervalController()
{

}

IDRFrameIntervalController::~IDRFrameIntervalController()
{

}

void IDRFrameIntervalController::SetSharedObject(CCommonElementsBucket* pcSharedObject)
{
    m_pCommonElementsBucket = pcSharedObject;
}

void IDRFrameIntervalController::SetEncoder(CVideoEncoder* pcVideEnocder)
{
    m_pVideoEncoder = pcVideEnocder;
}

bool IDRFrameIntervalController::Handle_IDRFrame_Control_Packet(CVideoHeader &crTempHeader, int nServiceType)
{
    crTempHeader.ShowDetails("Handle_IDRFrame_Control_Packet: ");
    
	if (nServiceType == SERVICE_TYPE_LIVE_STREAM || nServiceType == SERVICE_TYPE_SELF_STREAM || nServiceType == SERVICE_TYPE_CHANNEL)
    {
      return false;
    }
    
    long long llMissingFrame = crTempHeader.getFrameNumber();
    if(m_llFirstMissedFrameNumber == -1)
    {
        m_llFirstMissedFrameNumber = llMissingFrame;
    }
    else
    {
        m_llLastMissedFrameNumber = llMissingFrame;
    }
    

    return true;
}

long long JustForCheck = -1;

void IDRFrameIntervalController::NotifyEncodedFrame(unsigned char *ucaEncodedFrame, int nEncodedFrameSize, long long nFrameNumber)
{
    if(nEncodedFrameSize <= 0)
    {
        return;
    }
    
    int type = m_Tools.GetEncodedFrameType(ucaEncodedFrame);
    if(type == SPS_DATA)
    {
        m_llLastSentIDRFrameNumber = nFrameNumber;
        m_llFirstMissedFrameNumber = -1;
        m_llLastMissedFrameNumber = -1;
        m_nRelativePframeCounter = 0;
        printf("TheKing--> Sending IDR FRAME = %lld, IDR_GAP = %lld\n", nFrameNumber, nFrameNumber - JustForCheck);
        JustForCheck = nFrameNumber;
    }
    else
    {
        m_llLastSentPFrameNumber = nFrameNumber;
        m_nRelativePframeCounter++;
    }
    
    m_llLastFrameNumber = nFrameNumber;
}

bool IDRFrameIntervalController::NeedToGenerateIFrame(int nServiceType)
{
    if (nServiceType == SERVICE_TYPE_LIVE_STREAM || nServiceType == SERVICE_TYPE_SELF_STREAM || nServiceType == SERVICE_TYPE_CHANNEL)
    {
        return false;
    }
    long long nextFrameNumber = m_llLastFrameNumber + 1;
    
    //if(nextFrameNumber % 15 == 0)
    if( (nextFrameNumber - m_llLastSentIDRFrameNumber) >= 15)
    {
        if(m_llFirstMissedFrameNumber > m_llLastSentIDRFrameNumber && m_llFirstMissedFrameNumber < nextFrameNumber)
        {
            return true;
        }
        
        if(m_llLastMissedFrameNumber > m_llLastSentIDRFrameNumber && m_llLastMissedFrameNumber < nextFrameNumber)
        {
            return true;
        }
    }
    
    
    return false;
}

