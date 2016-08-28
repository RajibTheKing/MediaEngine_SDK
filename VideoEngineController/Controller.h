
#ifndef _CONTROLLER_H_
#define _CONTROLLER_H_

#include <stdio.h>
#include <string>
#include <vector>

#include "CommonElementsBucket.h"
#include "SmartPointer.h"
#include "EventNotifier.h"
#include "ThreadTools.h"
#include "LockHandler.h"
#include "DeviceCapabilityCheckThread.h"
#include "DeviceCapabilityCheckBuffer.h"
#include "AudioFileEncodeDecodeSession.h"

using namespace std;

typedef struct 
{
    int iHeight;
    int iWidth;
    
} VideoQuality;

class CController
{

public:

	CController();
	CController(const char* sLoggerPath, int iLoggerPrintLevel);
	~CController();

	bool SetUserName(const LongLong& lUserName);
	bool StartAudioCall(const LongLong& lFriendID);
	bool StartVideoCall(const LongLong& lFriendID, int iVideoHeight, int iVideoWidth, int iNetworkType);
	bool StartTestAudioCall(const LongLong& lFriendID);
	CVideoCallSession* StartTestVideoCall(const LongLong& lFriendID, int iVideoHeight, int iVideoWidth, int iNetworkType);
	int EncodeVideoFrame(const LongLong& lFriendID, unsigned char *in_data, unsigned int in_size);
	int PushPacketForDecoding(const LongLong& lFriendID, unsigned char *in_data, unsigned int in_size);
	int PushAudioForDecoding(const LongLong& lFriendID, unsigned char *in_data, unsigned int in_size);
	int SendAudioData(const LongLong& lFriendID, short *in_data, unsigned int in_size);
	int SendVideoData(const LongLong& lFriendID, unsigned char *in_data, unsigned int in_size, unsigned int orientation_type, int device_orientation);
	int SetHeightWidth(const LongLong& lFriendID, int width, int height); 
	int SetBitRate(const LongLong& lFriendID, int bitRate);

    int CheckDeviceCapability(const LongLong& lFriendID, int iHeightHigh, int iWidthHigh, int iHeightLow, int iWidthLow);
    int SetDeviceCapabilityResults(int iNotification, int iHeightHigh, int iWidthHigh, int iHeightLow, int iWidthLow);

	bool StopAudioCall(const LongLong& lFriendID);
	bool StopVideoCall(const LongLong& lFriendID);
	bool StopTestAudioCall(const LongLong& lFriendID);
	bool StopTestVideoCall(const LongLong& lFriendID);
	void initializeEventHandler();
	void SetLoggerPath(std::string);
    bool SetLoggingState(bool loggingState, int logLevel);
	void UninitializeLibrary();

	int StartAudioEncodeDecodeSession();
	int EncodeAudioFrame(short *psaEncodingDataBuffer, int nAudioFrameSize, unsigned char *ucaEncodedDataBuffer);
	int DecodeAudioFrame(unsigned char *ucaDecodedDataBuffer, int nAudioFrameSize, short *psaDecodingDataBuffer);
	int StopAudioEncodeDecodeSession();

	void SetNotifyClientWithPacketCallback(void(*callBackFunctionPointer)(LongLong, unsigned char*, int));
	void SetNotifyClientWithVideoDataCallback(void(*callBackFunctionPointer)(LongLong, unsigned char*, int, int, int, int));
	void SetNotifyClientWithVideoNotificationCallback(void(*callBackFunctionPointer)(LongLong, int));
	void SetNotifyClientWithNetworkStrengthNotificationCallback(void(*callBackFunctionPointer)(IPVLongType, int));
	void SetNotifyClientWithAudioDataCallback(void(*callBackFunctionPointer)(LongLong, short*, int));
	void SetNotifyClientWithAudioPacketDataCallback(void(*callBackFunctionPointer)(IPVLongType, unsigned char*, int));
	void SetNotifyClientWithAudioAlarmCallback(void(*callBackFunctionPointer)(LongLong, short*, int));


    void SetSendFunctionPointer(void(*callBackFunctionPointer)(IPVLongType, int, unsigned char*, int));

	int m_nDeviceStrongness;
	int m_nMemoryEnoughness;
	int m_nEDVideoSupportablity;
	int m_nHighFPSVideoSupportablity;

	unsigned long long m_ullTotalDeviceMemory;

	int m_nSupportedResolutionFPSLevel;
    
    VideoQuality m_Quality[2];
	bool m_bLiveCallRunning;
	
private:

	CEventNotifier m_EventNotifier;

	int iLoggerPrintLevel;
	std::string logFilePath;
	Tools m_Tools;

	CAudioFileEncodeDecodeSession *m_pAudioEncodeDecodeSession;

	CCommonElementsBucket *m_pCommonElementsBucket;
	CDeviceCapabilityCheckThread *m_pDeviceCapabilityCheckThread;
	CDeviceCapabilityCheckBuffer *m_pDeviceCapabilityCheckBuffer;
    
    bool m_bDeviceCapabilityRunning;
    
    

	int m_nDeviceSupportedCallFPS;

    SmartPointer<CLockHandler> m_pVideoSendMutex;
    SmartPointer<CLockHandler> m_pVideoReceiveMutex;
    SmartPointer<CLockHandler> m_pAudioSendMutex;
    SmartPointer<CLockHandler> m_pAudioReceiveMutex;
};

#endif
