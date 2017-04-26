
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
#include "VideoMuxingAndEncodeSession.h"

using namespace std;
//#define DISABLE_VIDEO_FOR_LIVE

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
	bool StartAudioCall(const LongLong& lFriendID, int nServiceType, int nEntityType);
	bool SetVolume(const LongLong& lFriendID, int iVolume, bool bRecorder);
	bool SetLoudSpeaker(const LongLong& lFriendID, bool bOn);
	bool SetEchoCanceller(const LongLong& lFriendID, bool bOn);
	bool StartVideoCall(const LongLong& lFriendID, int iVideoHeight, int iVideoWidth, int nServiceType, int nEntityType, int iNetworkType, bool bAudioOnlyLive, bool bSelfViewOnly);
	bool StartTestAudioCall(const LongLong& lFriendID);
	CVideoCallSession* StartTestVideoCall(const LongLong& lFriendID, int iVideoHeight, int iVideoWidth, int iNetworkType);
	int EncodeVideoFrame(const LongLong& lFriendID, unsigned char *in_data, unsigned int in_size);
	int PushPacketForDecoding(const LongLong& lFriendID, unsigned char *in_data, unsigned int in_size, int numberOfFrames = 0, int *frameSizes = NULL, int numberOfMissingFrames = 0, int *missingFrames = NULL);
	int PushPacketForDecodingVector(const LongLong& lFriendID, int offset, unsigned char *in_data, unsigned int in_size, int numberOfFrames, int *frameSizes, std::vector< std::pair<int, int> > vMissingFrames);
	int PushAudioForDecoding(const LongLong& lFriendID, int nOffset, unsigned char *in_data, unsigned int in_size, int numberOfFrames, int *frameSizes, std::vector< std::pair<int, int> > vMissingFrames);
	int SendAudioData(const LongLong& lFriendID, short *in_data, unsigned int in_size);
	int CancelAudioData(const LongLong& lFriendID, short *in_data, unsigned int in_size);
	int SendVideoData(const LongLong& lFriendID, unsigned char *in_data, unsigned int in_size, unsigned int orientation_type, int device_orientation);
	int SetEncoderHeightWidth(const LongLong& lFriendID, int height, int width);
	int SetDeviceDisplayHeightWidth(int height, int width);
	int SetBitRate(const LongLong& lFriendID, int bitRate);

    int CheckDeviceCapability(const LongLong& lFriendID, int iHeightHigh, int iWidthHigh, int iHeightLow, int iWidthLow);
    int SetDeviceCapabilityResults(int iNotification, int iHeightHigh, int iWidthHigh, int iHeightLow, int iWidthLow);

	int SetVideoEffect(const IPVLongType llFriendID, int nEffectStatus);

	void SetCallInLiveType(const IPVLongType llFriendID, int nCallInLiveType);

	int TestVideoEffect(const IPVLongType llFriendID, int *param, int size);

	void InterruptOccured(const LongLong lFriendID);
	void InterruptOver(const LongLong lFriendID);

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

	int StartVideoMuxingAndEncodeSession(unsigned char *pBMP32Data,int iLen, int nVideoHeight, int nVideoWidth);
	int FrameMuxAndEncode( unsigned char *pVideoYuv, int iHeight, int iWidth);
	int StopVideoMuxingAndEncodeSession(unsigned char *finalData);

	void SetNotifyClientWithPacketCallback(void(*callBackFunctionPointer)(LongLong, unsigned char*, int));

#if defined(DESKTOP_C_SHARP)

	void SetNotifyClientWithVideoDataCallback(void(*callBackFunctionPointer)(LongLong, int, unsigned char*, int, int, int, int, int, int));

#else

	void SetNotifyClientWithVideoDataCallback(void(*callBackFunctionPointer)(LongLong, int, unsigned char*, int, int, int, int));

#endif

	void SetNotifyClientWithVideoNotificationCallback(void(*callBackFunctionPointer)(LongLong, int));
	void SetNotifyClientWithNetworkStrengthNotificationCallback(void(*callBackFunctionPointer)(IPVLongType, int));
	void SetNotifyClientWithAudioDataCallback(void(*callBackFunctionPointer)(LongLong, int, short*, int));
	void SetNotifyClientWithAudioPacketDataCallback(void(*callBackFunctionPointer)(IPVLongType, unsigned char*, int));
	void SetNotifyClientWithAudioAlarmCallback(void(*callBackFunctionPointer)(LongLong, short*, int));


	void SetSendFunctionPointer(SendFunctionPointerType callBackFunctionPointer);

	bool StartAudioCallInLive(const LongLong& lFriendID, int iRole, int nCallInLiveType);
	bool EndAudioCallInLive(const LongLong& lFriendID);
	bool StartVideoCallInLive(const LongLong& lFriendID, int nCallInLiveType);
	bool EndVideoCallInLive(const LongLong& lFriendID);
	bool IsCallInLiveEnabled();
	void SetCallInLiveEnabled(bool value);

	int GetDeviceDisplayHeight();
	int GetDeviceDisplayWidth();

	int m_nDeviceStrongness;
	int m_nMemoryEnoughness;
	int m_nEDVideoSupportablity;
	int m_nHighFPSVideoSupportablity;

	unsigned long long m_ullTotalDeviceMemory;

	int m_nSupportedResolutionFPSLevel;
    
    VideoQuality m_Quality[2];
	bool m_bLiveCallRunning;

	CCommonElementsBucket *m_pCommonElementsBucket;
	long long m_llLastTimeStamp;

private:

	CEventNotifier m_EventNotifier;

	int m_nDeviceDisplayHeight;
	int m_nDeviceDisplayWidth;

	int iLoggerPrintLevel;
	std::string logFilePath;
	Tools m_Tools;

	CAudioFileEncodeDecodeSession *m_pAudioEncodeDecodeSession;

	CVideoMuxingAndEncodeSession *m_pVideoMuxingAndEncodeSession;

	
	CDeviceCapabilityCheckThread *m_pDeviceCapabilityCheckThread;
	CDeviceCapabilityCheckBuffer *m_pDeviceCapabilityCheckBuffer;
    
    bool m_bDeviceCapabilityRunning;
	bool m_bCallInLiveEnabled;
    

	int m_nDeviceSupportedCallFPS;

	SmartPointer<CLockHandler> m_pVideoStartMutex;
    SmartPointer<CLockHandler> m_pVideoSendMutex;
    SmartPointer<CLockHandler> m_pVideoReceiveMutex;
    SmartPointer<CLockHandler> m_pAudioSendMutex;
    SmartPointer<CLockHandler> m_pAudioReceiveMutex;
};

#endif
