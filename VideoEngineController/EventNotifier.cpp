
#include "EventNotifier.h"
#include <string.h>
#include "LogPrinter.h"
#include "Tools.h"
#include "Size.h"
#include "Controller.h"

namespace MediaSDK
{

	void(*notifyClientWithPacketCallback)(long long, unsigned char*, int) = NULL;

#if defined(DESKTOP_C_SHARP)

	void(*notifyClientWithVideoDataCallback)(long long, int, unsigned char*, int, int, int, int, int, int) = NULL;

#else

	void(*notifyClientWithVideoDataCallback)(long long, int, unsigned char*, int, int, int, int) = NULL;

#endif

	void(*notifyClientWithVideoNotificationCallback)(long long, int) = NULL;
	void(*notifyClientWithNetworkStrengthNotificationCallback)(long long, int) = NULL;
	void(*notifyClientWithAudioDataCallback)(long long, int, short*, int) = NULL;
	void(*notifyClientWithAudioPacketDataCallback)(long long, unsigned char*, int) = NULL;
	void(*notifyClientWithAudioAlarmCallback)(long long, short*, int) = NULL;


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

#if defined(DESKTOP_C_SHARP)

	void CEventNotifier::fireVideoEvent(long long friendID, int eventType, int frameNumber, int dataLenth, unsigned char data[], int iVideoHeight, int iVideoWidth, int nInsetHeight, int nInsetWidth, int iDeviceOrientation)
	{
		//    CLogPrinter_WriteLog(CLogPrinter::INFO, INSTENT_TEST_LOG,"CEventNotifier::firePacketEvent eventType = " + Tools::IntegertoStringConvert(eventType) + ", FrameNumber = " + Tools::IntegertoStringConvert(frameNumber) + " iOrientation --> " + Tools::IntegertoStringConvert(iDeviceOrientation));

		notifyClientWithVideoDataCallback(friendID, eventType, data, dataLenth, iVideoHeight, iVideoWidth, nInsetHeight, nInsetWidth, iDeviceOrientation);
	}

#else

	void CEventNotifier::fireVideoEvent(long long friendID, int eventType, int frameNumber, int dataLenth, unsigned char data[], int iVideoHeight, int iVideoWidth, int iDeviceOrientation)
	{
		//    CLogPrinter_WriteLog(CLogPrinter::INFO, INSTENT_TEST_LOG,"CEventNotifier::firePacketEvent eventType = " + Tools::IntegertoStringConvert(eventType) + ", FrameNumber = " + Tools::IntegertoStringConvert(frameNumber) + " iOrientation --> " + Tools::IntegertoStringConvert(iDeviceOrientation));

		notifyClientWithVideoDataCallback(friendID, eventType, data, dataLenth, iVideoHeight, iVideoWidth, iDeviceOrientation);
	}

#endif

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

	void CEventNotifier::SetNotifyClientWithPacketCallback(void(*callBackFunctionPointer)(long long, unsigned char*, int))
	{
		notifyClientWithPacketCallback = callBackFunctionPointer;
	}

#if defined(DESKTOP_C_SHARP)

	void CEventNotifier::SetNotifyClientWithVideoDataCallback(void(*callBackFunctionPointer)(long long, int, unsigned char*, int, int, int, int, int, int))
	{
		notifyClientWithVideoDataCallback = callBackFunctionPointer;
	}

#else

	void CEventNotifier::SetNotifyClientWithVideoDataCallback(void(*callBackFunctionPointer)(long long, int, unsigned char*, int, int, int, int))
	{
		notifyClientWithVideoDataCallback = callBackFunctionPointer;
	}


#endif

	void CEventNotifier::SetNotifyClientWithVideoNotificationCallback(void(*callBackFunctionPointer)(long long, int))
	{
		notifyClientWithVideoNotificationCallback = callBackFunctionPointer;
	}

	void CEventNotifier::SetNotifyClientWithNetworkStrengthNotificationCallback(void(*callBackFunctionPointer)(long long, int))
	{
		notifyClientWithNetworkStrengthNotificationCallback = callBackFunctionPointer;
	}

	void CEventNotifier::SetNotifyClientWithAudioDataCallback(void(*callBackFunctionPointer)(long long, int, short*, int))
	{
		notifyClientWithAudioDataCallback = callBackFunctionPointer;
	}

	void CEventNotifier::SetNotifyClientWithAudioPacketDataCallback(void(*callBackFunctionPointer)(long long, unsigned char*, int))
	{
		notifyClientWithAudioPacketDataCallback = callBackFunctionPointer;
	}

	void CEventNotifier::SetNotifyClientWithAudioAlarmCallback(void(*callBackFunctionPointer)(long long, short*, int))
	{
		notifyClientWithAudioAlarmCallback = callBackFunctionPointer;
	}

	bool CEventNotifier::IsVideoCallRunning()
	{
		return m_pController->m_bLiveCallRunning;
	}



} //namespace MediaSDK


