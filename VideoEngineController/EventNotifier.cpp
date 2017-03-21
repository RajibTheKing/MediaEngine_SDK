#include "EventNotifier.h"
#include <string.h>
#include "LogPrinter.h"
#include "Tools.h"
#include "Size.h"
#include "Controller.h"

void(*notifyClientWithPacketCallback)(LongLong, unsigned char*, int) = NULL;
void(*notifyClientWithVideoDataCallback)(LongLong, int, unsigned char*, int, int, int, int) = NULL;
void(*notifyClientWithVideoNotificationCallback)(LongLong, int) = NULL;
void(*notifyClientWithNetworkStrengthNotificationCallback)(LongLong, int) = NULL;
void(*notifyClientWithAudioDataCallback)(LongLong, int, short*, int) = NULL;
void(*notifyClientWithAudioPacketDataCallback)(IPVLongType, unsigned char*, int) = NULL;
void(*notifyClientWithAudioAlarmCallback)(LongLong, short*, int) = NULL;


CEventNotifier::CEventNotifier(CController *pController)
{
	m_pController = pController;
}

void CEventNotifier::firePacketEvent(int eventType, int frameNumber, int numberOfPackets, int packetNumber, int packetSize, int dataLenth, unsigned char data[])
{
	CLogPrinter_Write(CLogPrinter::INFO, "CEventNotifier::firePacketEvent 1");

    notifyClientWithPacketCallback(200, data, dataLenth);

	CLogPrinter_Write(CLogPrinter::INFO, "CEventNotifier::firePacketEvent 2");
}

void CEventNotifier::fireVideoEvent(long long friendID, int eventType, int frameNumber, int dataLenth, unsigned char data[], int iVideoHeight, int iVideoWidth, int iDeviceOrientation)
{
//    CLogPrinter_WriteLog(CLogPrinter::INFO, INSTENT_TEST_LOG,"CEventNotifier::firePacketEvent eventType = " + Tools::IntegertoStringConvert(eventType) + ", FrameNumber = " + Tools::IntegertoStringConvert(frameNumber) + " iOrientation --> " + Tools::IntegertoStringConvert(iDeviceOrientation));
    
	notifyClientWithVideoDataCallback(friendID, eventType, data, dataLenth, iVideoHeight, iVideoWidth, iDeviceOrientation);
}

void CEventNotifier::fireVideoNotificationEvent(long long callID, int eventType)
{
    CLogPrinter_Write(CLogPrinter::INFO, "CEventNotifier::firePacketEvent eventType = " + Tools::IntegertoStringConvert(eventType));

	notifyClientWithVideoNotificationCallback(callID, eventType);
    
	if (eventType == SET_CAMERA_RESOLUTION_640x480_25FPS)
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

void CEventNotifier::fireNetworkStrengthNotificationEvent(long long callID, int eventType)
{
	CLogPrinter_Write(CLogPrinter::INFO, "CEventNotifier::firePacketEvent eventType = " + Tools::IntegertoStringConvert(eventType));

	notifyClientWithNetworkStrengthNotificationCallback(callID, eventType);

	if (eventType == NETWORK_STRENTH_GOOD)
	{
		CLogPrinter_WriteLog(CLogPrinter::INFO, VIDEO_NOTIFICATION_LOG, "Video quality low");
	}
	else if (eventType == NETWORK_STRENTH_EXCELLENT)
	{
		CLogPrinter_WriteLog(CLogPrinter::INFO, VIDEO_NOTIFICATION_LOG, "Video quality high");
	}
	else if (eventType == NETWORK_STRENTH_BAD)
	{
		CLogPrinter_WriteLog(CLogPrinter::INFO, VIDEO_NOTIFICATION_LOG, "Video must stop");
	}
}

void CEventNotifier::fireAudioPacketEvent(int eventType, int dataLenth, unsigned char data[])
{
	CLogPrinter_Write(CLogPrinter::INFO, "CEventNotifier::fireAudioPacketEvent 1");

	notifyClientWithAudioPacketDataCallback(200, data, dataLenth);

	CLogPrinter_Write(CLogPrinter::INFO, "CEventNotifier::fireAudioPacketEvent 2");
}

void CEventNotifier::fireAudioEvent(long long friendID, int eventType, int dataLenth, short data[])
{
	CLogPrinter_Write(CLogPrinter::INFO, "CEventNotifier::fireAudioEvent " + Tools::IntegertoStringConvert(friendID));

	notifyClientWithAudioDataCallback(friendID, eventType, data, dataLenth);
    
    CLogPrinter_Write(CLogPrinter::INFO, "CEventNotifier::fireAudioEvent 2");
}

void CEventNotifier::fireAudioAlarm(int eventType, int dataLenth, short data[])
{
	CLogPrinter_Write(CLogPrinter::INFO, "CEventNotifier::fireAudioAlarm " + Tools::IntegertoStringConvert(eventType));

	if (((eventType == AUDIO_EVENT_PEER_TOLD_TO_STOP_VIDEO || eventType == AUDIO_EVENT_I_TOLD_TO_STOP_VIDEO) && m_pController->m_bLiveCallRunning)
		||
		(eventType != AUDIO_EVENT_PEER_TOLD_TO_STOP_VIDEO && eventType != AUDIO_EVENT_I_TOLD_TO_STOP_VIDEO)
		)
	{
		notifyClientWithAudioAlarmCallback(eventType, data, dataLenth);
	}
	
	CLogPrinter_Write(CLogPrinter::INFO, "CEventNotifier::fireAudioAlarm 2");
}

void CEventNotifier::SetNotifyClientWithPacketCallback(void(*callBackFunctionPointer)(LongLong, unsigned char*, int))
{
    notifyClientWithPacketCallback = callBackFunctionPointer;
}

void CEventNotifier::SetNotifyClientWithVideoDataCallback(void(*callBackFunctionPointer)(LongLong, int, unsigned char*, int, int, int, int))
{
    notifyClientWithVideoDataCallback = callBackFunctionPointer;
}

void CEventNotifier::SetNotifyClientWithVideoNotificationCallback(void(*callBackFunctionPointer)(LongLong, int))
{
	notifyClientWithVideoNotificationCallback = callBackFunctionPointer;
}

void CEventNotifier::SetNotifyClientWithNetworkStrengthNotificationCallback(void(*callBackFunctionPointer)(LongLong, int))
{
	notifyClientWithNetworkStrengthNotificationCallback = callBackFunctionPointer;
}

void CEventNotifier::SetNotifyClientWithAudioDataCallback(void(*callBackFunctionPointer)(LongLong, int, short*, int))
{
    notifyClientWithAudioDataCallback = callBackFunctionPointer;
}

void CEventNotifier::SetNotifyClientWithAudioPacketDataCallback(void(*callBackFunctionPointer)(IPVLongType, unsigned char*, int))
{
	notifyClientWithAudioPacketDataCallback = callBackFunctionPointer;
}

void CEventNotifier::SetNotifyClientWithAudioAlarmCallback(void(*callBackFunctionPointer)(LongLong, short*, int))
{
	notifyClientWithAudioAlarmCallback = callBackFunctionPointer;
}

bool CEventNotifier::IsVideoCallRunning(){
	return m_pController->m_bLiveCallRunning;
}





