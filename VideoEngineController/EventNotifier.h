#ifndef _EVENT_NOTIFIER_H_
#define _EVENT_NOTIFIER_H_

#include <stdio.h>

#include "AudioVideoEngineDefinitions.h"
#include "LogPrinter.h"

class CController;
class CEventNotifier
{
	
public:
	CEventNotifier(CController *pController);

	void firePacketEvent(int eventType, int frameNumber, int numberOfPackets, int packetNumber, int packetSize, int dataLenth, unsigned char data[]);
	void fireVideoEvent(long long friendID, int eventType, int frameNumber, int dataLenth, unsigned char data[], int iVideoHeight, int iVideoWidth, int iOrientation);
	void fireAudioPacketEvent(int eventType, int dataLenth, unsigned char data[]);
	void fireAudioEvent(long long friendID, int eventType, int dataLenth, short data[]);
	void fireAudioAlarm(int eventType, int dataLenth, short data[]);
    void fireVideoNotificationEvent(long long callID, int eventType);
	void fireNetworkStrengthNotificationEvent(long long callID, int eventType);
    
    void SetNotifyClientWithPacketCallback(void(*callBackFunctionPointer)(LongLong, unsigned char*, int));
	void SetNotifyClientWithVideoDataCallback(void(*callBackFunctionPointer)(LongLong, int, unsigned char*, int, int, int, int));
	void SetNotifyClientWithVideoNotificationCallback(void(*callBackFunctionPointer)(LongLong, int));
	void SetNotifyClientWithNetworkStrengthNotificationCallback(void(*callBackFunctionPointer)(LongLong, int));
    void SetNotifyClientWithAudioDataCallback(void(*callBackFunctionPointer)(LongLong, int, short*, int));
	void SetNotifyClientWithAudioPacketDataCallback(void(*callBackFunctionPointer)(IPVLongType, unsigned char*, int));
	void SetNotifyClientWithAudioAlarmCallback(void(*callBackFunctionPointer)(LongLong, short*, int));
	bool IsVideoCallRunning();

	
    static const int NETWORK_STRENTH_GOOD = 202;
	static const int NETWORK_STRENTH_EXCELLENT = 204;
    static const int NETWORK_STRENTH_BAD = 203;
	static const int VIDEO_SHOULD_STOP = 201;

	static const int SET_CAMERA_RESOLUTION_640x480_25FPS = 205;
    static const int SET_CAMERA_RESOLUTION_640x480_25FPS_NOT_SUPPORTED = 206;
    static const int SET_CAMERA_RESOLUTION_352x288_25FPS = 207;
	static const int SET_CAMERA_RESOLUTION_352x288_25FPS_NOT_SUPPORTED = 208;
    
    static const int SET_CAMERA_RESOLUTION_352x288_15FPS = 209;

	static const int SET_CAMERA_RESOLUTION_352x288 = 210;
	static const int SET_CAMERA_RESOLUTION_640x480 = 211;

private:
	CController *m_pController;
};

#endif
