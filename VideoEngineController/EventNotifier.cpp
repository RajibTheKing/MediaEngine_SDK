#include "EventNotifier.h"
#include <string.h>
#include "LogPrinter.h"
#include "Tools.h"

void(*notifyClientWithPacketCallback)(LongLong, unsigned char*, int) = NULL;
void(*notifyClientWithVideoDataCallback)(LongLong, unsigned char*, int, int, int) = NULL;
void(*notifyClientWithVideoNotificationCallback)(LongLong, int) = NULL;
void(*notifyClientWithAudioDataCallback)(LongLong, short*, int) = NULL;
void(*notifyClientWithAudioPacketDataCallback)(IPVLongType, unsigned char*, int) = NULL;

void CEventNotifier::firePacketEvent(int eventType, int frameNumber, int numberOfPackets, int packetNumber, int packetSize, int dataLenth, unsigned char data[])
{
	CLogPrinter_Write(CLogPrinter::INFO, "CEventNotifier::firePacketEvent 1");

    notifyClientWithPacketCallback(200, data, dataLenth);

	CLogPrinter_Write(CLogPrinter::INFO, "CEventNotifier::firePacketEvent 2");
}

void CEventNotifier::fireVideoEvent(int eventType, int frameNumber, int dataLenth, unsigned char data[], int iVideoHeight, int iVideoWidth)
{
    CLogPrinter_Write(CLogPrinter::INFO, "CEventNotifier::firePacketEvent eventType = " + Tools::IntegertoStringConvert(eventType) + ", FrameNumber = " + Tools::IntegertoStringConvert(frameNumber));
    
    notifyClientWithVideoDataCallback(eventType, data, dataLenth, iVideoHeight, iVideoWidth);
}

void CEventNotifier::fireVideoNotificationEvent(long long callID, int eventType)
{
    CLogPrinter_Write(CLogPrinter::INFO, "CEventNotifier::firePacketEvent eventType = " + Tools::IntegertoStringConvert(eventType));

	notifyClientWithVideoNotificationCallback(callID, eventType);
    
    if(eventType == VIDEO_QUALITY_LOW)
    {
		CLogPrinter_WriteLog(CLogPrinter::INFO, VIDEO_NOTIFICATION_LOG, "Video quality low");
    }
	else if (eventType == VIDEO_QUALITY_HIGH)
	{
		CLogPrinter_WriteLog(CLogPrinter::INFO, VIDEO_NOTIFICATION_LOG, "Video quality high");
	}
    else if(eventType == VIDEO_SHOULD_STOP)
    {
		CLogPrinter_WriteLog(CLogPrinter::INFO, VIDEO_NOTIFICATION_LOG, "Video must stop");
    }
	else if (eventType == SET_CAMERA_RESOLUTION_640x480_25FPS)
	{
		CLogPrinter_WriteLog(CLogPrinter::INFO, VIDEO_NOTIFICATION_LOG, "SET_CAMERA_RESOLUTION_640x480_25FPS called");
	}
	else if (eventType == SET_CAMERA_RESOLUTION_352x288_25FPS)
	{
		CLogPrinter_WriteLog(CLogPrinter::INFO, VIDEO_NOTIFICATION_LOG, "SET_CAMERA_RESOLUTION_352x288_25FPS called");
	}
    else if (eventType == SET_CAMERA_RESOLUTION_352x288_15FPS)
    {
        CLogPrinter_WriteLog(CLogPrinter::INFO, VIDEO_NOTIFICATION_LOG, "SET_CAMERA_RESOLUTION_352x288_15FPS called");
    }
}

void CEventNotifier::fireAudioPacketEvent(int eventType, int dataLenth, unsigned char data[])
{
    CLogPrinter_Write(CLogPrinter::INFO, "CEventNotifier::fireAudioPacketEvent 1");

    notifyClientWithAudioPacketDataCallback(200, data, dataLenth);
    
    CLogPrinter_Write(CLogPrinter::INFO, "CEventNotifier::fireAudioPacketEvent 2");
}

void CEventNotifier::fireAudioEvent(int eventType, int dataLenth, short data[])
{
    CLogPrinter_Write(CLogPrinter::INFO, "CEventNotifier::fireAudioEvent " + Tools::IntegertoStringConvert(eventType));

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

void CEventNotifier::SetNotifyClientWithVideoNotificationCallback(void(*callBackFunctionPointer)(LongLong, int))
{
	notifyClientWithVideoNotificationCallback = callBackFunctionPointer;
}

void CEventNotifier::SetNotifyClientWithAudioDataCallback(void(*callBackFunctionPointer)(LongLong, short*, int))
{
    notifyClientWithAudioDataCallback = callBackFunctionPointer;
}

void CEventNotifier::SetNotifyClientWithAudioPacketDataCallback(void(*callBackFunctionPointer)(IPVLongType, unsigned char*, int))
{
    notifyClientWithAudioPacketDataCallback = callBackFunctionPointer;
}






