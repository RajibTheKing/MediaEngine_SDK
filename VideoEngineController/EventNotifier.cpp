#include "EventNotifier.h"
#include <string.h>
#include "LogPrinter.h"
#include "Tools.h"

void(*notifyClientWithPacketCallback)(LongLong, unsigned char*, int) = NULL;
void(*notifyClientWithVideoDataCallback)(LongLong, unsigned char*, int, int, int) = NULL;
void(*notifyClientWithAudioDataCallback)(LongLong, short*, int) = NULL;
void(*notifyClientWithAudioPacketDataCallback)(IPVLongType, unsigned char*, int) = NULL;

void CEventNotifier::firePacketEvent(int eventType, int frameNumber, int numberOfPackets, int packetNumber, int packetSize, int dataLenth, unsigned char data[])
{
	CLogPrinter_Write(CLogPrinter::INFO, "CEventNotifier::firePacketEvent 1");

	//notifyClientMethodWithPacketE(200, data, dataLenth);
    
    notifyClientWithPacketCallback(200, data, dataLenth);

	CLogPrinter_Write(CLogPrinter::INFO, "CEventNotifier::firePacketEvent 2");
}

void CEventNotifier::fireVideoEvent(int eventType, int frameNumber, int dataLenth, unsigned char data[], int iVideoHeight, int iVideoWidth)
{
	//notifyClientMethodWithVideoDataE(200, data, dataLenth, iVideoHeight, iVideoWidth);
    
    CLogPrinter_Write(CLogPrinter::INFO, "CEventNotifier::firePacketEvent eventType = " + Tools::IntegertoStringConvert(eventType) + ", FrameNumber = " + Tools::IntegertoStringConvert(frameNumber));
    
    notifyClientWithVideoDataCallback(eventType, data, dataLenth, iVideoHeight, iVideoWidth);
}

void CEventNotifier::fireVideoNotificationEvent(int callID, int eventType)
{
    CLogPrinter_Write(CLogPrinter::INFO, "CEventNotifier::firePacketEvent eventType = " + Tools::IntegertoStringConvert(eventType));
    
    if(eventType == VIDEO_QUALITY_LOW)
    {
		CLogPrinter_Write(CLogPrinter::INFO, "Video quality low\n");
    }
    else if(eventType == VIDEO_SHOULD_STOP)
    {
		CLogPrinter_Write(CLogPrinter::INFO, "Video must stop\n");
    }
}

void CEventNotifier::fireAudioPacketEvent(int eventType, int dataLenth, unsigned char data[])
{
    CLogPrinter_Write(CLogPrinter::INFO, "CEventNotifier::fireAudioPacketEvent 1");
    
//	notifyClientMethodWithAudioPacketDataE(200, data, dataLenth);
    
    notifyClientWithAudioPacketDataCallback(200, data, dataLenth);
    
    CLogPrinter_Write(CLogPrinter::INFO, "CEventNotifier::fireAudioPacketEvent 2");
}

void CEventNotifier::fireAudioEvent(int eventType, int dataLenth, short data[])
{
    CLogPrinter_Write(CLogPrinter::INFO, "CEventNotifier::fireAudioEvent " + Tools::IntegertoStringConvert(eventType));
    
//	notifyClientMethodWithAudioDataE(200, data, dataLenth);
    
    notifyClientWithAudioDataCallback(eventType, data, dataLenth);
    
    CLogPrinter_Write(CLogPrinter::INFO, "CEventNotifier::fireAudioEvent 2");
}

void CEventNotifier::SetNotifyClientWithPacketCallback(void(*callBackFunctionPointer)(LongLong, unsigned char*, int))
{
    notifyClientWithPacketCallback = callBackFunctionPointer;
}

void CEventNotifier::SetNotifyClientWithVideoDataCallback(void(*callBackFunctionPointer)(LongLong, unsigned char*, int, int, int))
{
    notifyClientWithVideoDataCallback = callBackFunctionPointer;
}

void CEventNotifier::SetNotifyClientWithAudioDataCallback(void(*callBackFunctionPointer)(LongLong, short*, int))
{
    notifyClientWithAudioDataCallback = callBackFunctionPointer;
}

void CEventNotifier::SetNotifyClientWithAudioPacketDataCallback(void(*callBackFunctionPointer)(IPVLongType, unsigned char*, int))
{
    notifyClientWithAudioPacketDataCallback = callBackFunctionPointer;
}






