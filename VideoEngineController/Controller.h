
#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <stdio.h>
#include <string>
#include <vector>

#include "CommonElementsBucket.h"
#include "SmartPointer.h"
#include "EventNotifier.h"
#include "ThreadTools.h"
#include "CommonTypes.h"
#include "DeviceCapabilityCheckThread.h"
#include "DeviceCapabilityCheckBuffer.h"
#include "AudioFileEncodeDecodeSession.h"
#include "VideoMuxingAndEncodeSession.h"

namespace MediaSDK
{

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

		bool SetUserName(const long long& lUserName);
		bool StartAudioCall(const long long& lFriendID, int nServiceType, int nEntityType, int nAudioSpeakerType);
		bool SetVolume(const long long& lFriendID, int iVolume, bool bRecorder);		
		bool SetSpeakerType(const long long& lFriendID, int iSpeakerType);
		bool SetEchoCanceller(const long long& lFriendID, bool bOn);
		bool StartVideoCall(const long long& lFriendID, int iVideoHeight, int iVideoWidth, int nServiceType, int nEntityType, int iNetworkType, bool bAudioOnlyLive, bool bSelfViewOnly);
		bool StartTestAudioCall(const long long& lFriendID);
		CVideoCallSession* StartTestVideoCall(const long long& lFriendID, int iVideoHeight, int iVideoWidth, int iNetworkType);
		int EncodeVideoFrame(const long long& lFriendID, unsigned char *in_data, unsigned int in_size);
		int PushPacketForDecoding(const long long& lFriendID, unsigned char *in_data, unsigned int in_size, int numberOfFrames = 0, int *frameSizes = NULL, int numberOfMissingFrames = 0, int *missingFrames = NULL);
		int PushPacketForDecodingVector(const long long& lFriendID, int offset, unsigned char *in_data, unsigned int in_size, int numberOfFrames, int *frameSizes, std::vector< std::pair<int, int> > vMissingFrames);
		int PushAudioForDecoding(const long long& lFriendID, int nOffset, unsigned char *in_data, unsigned int in_size, int numberOfFrames, int *frameSizes, std::vector< std::pair<int, int> > vMissingFrames);
		int SendAudioData(const long long& lFriendID, short *in_data, unsigned int in_size);
		int CancelAudioData(const long long& lFriendID, short *in_data, unsigned int in_size);
		int SendVideoData(const long long& lFriendID, unsigned char *in_data, unsigned int in_size, unsigned int orientation_type, int device_orientation);
		int SetEncoderHeightWidth(const long long& lFriendID, int height, int width, int nDataType);
		int SetDeviceDisplayHeightWidth(int height, int width);
		int SetBitRate(const long long& lFriendID, int bitRate);

		int CheckDeviceCapability(const long long& lFriendID, int iHeightHigh, int iWidthHigh, int iHeightLow, int iWidthLow);
		int SetDeviceCapabilityResults(int iNotification, int iHeightHigh, int iWidthHigh, int iHeightLow, int iWidthLow);

        int SetBeautification(const IPVLongType llFriendID, bool bIsEnable);
		int SetVideoEffect(const long long llFriendID, int nEffectStatus);

		void SetCallInLiveType(const long long llFriendID, int nCallInLiveType);

		int TestVideoEffect(const long long llFriendID, int *param, int size);

		void InterruptOccured(const long long lFriendID);
		void InterruptOver(const long long lFriendID);

		bool StopAudioCall(const long long& lFriendID);
		bool StopVideoCall(const long long& lFriendID);
		bool StopTestAudioCall(const long long& lFriendID);
		bool StopTestVideoCall(const long long& lFriendID);
		void initializeEventHandler();
		void SetLoggerPath(std::string);
		bool SetLoggingState(bool loggingState, int logLevel);
		void UninitializeLibrary();

		int StartAudioEncodeDecodeSession();
		int EncodeAudioFrame(short *psaEncodingDataBuffer, int nAudioFrameSize, unsigned char *ucaEncodedDataBuffer);
		int DecodeAudioFrame(unsigned char *ucaDecodedDataBuffer, int nAudioFrameSize, short *psaDecodingDataBuffer);
		int StopAudioEncodeDecodeSession();

		int StartVideoMuxingAndEncodeSession(unsigned char *pBMP32Data, int iLen, int nVideoHeight, int nVideoWidth);
		int FrameMuxAndEncode(unsigned char *pVideoYuv, int iHeight, int iWidth);
		int StopVideoMuxingAndEncodeSession(unsigned char *finalData);

		void SetNotifyClientWithPacketCallback(void(*callBackFunctionPointer)(long long, unsigned char*, int));
		void SetNotifyClientWithVideoDataCallback(void(*callBackFunctionPointer)(long long, int, unsigned char*, int, int, int, int, int, int));

		void TraverseReceivedVideoData(int offset, unsigned char *in_data, unsigned int in_size, int numberOfFrames, int *frameSizes, std::vector< std::pair<int, int> > vMissingFrames);

		void SetNotifyClientWithVideoNotificationCallback(void(*callBackFunctionPointer)(long long, int));
		void SetNotifyClientWithNetworkStrengthNotificationCallback(void(*callBackFunctionPointer)(long long, int));
		void SetNotifyClientWithAudioDataCallback(void(*callBackFunctionPointer)(long long, int, short*, int));
		void SetNotifyClientWithAudioPacketDataCallback(void(*callBackFunctionPointer)(long long, unsigned char*, int));
		void SetNotifyClientWithAudioAlarmCallback(void(*callBackFunctionPointer)(long long, short*, int));


		void SetSendFunctionPointer(SendFunctionPointerType callBackFunctionPointer);

		bool StartAudioCallInLive(const long long& lFriendID, int iRole, int nCallInLiveType);
		bool EndAudioCallInLive(const long long& lFriendID);
		bool StartVideoCallInLive(const long long& lFriendID, int nCallInLiveType, int nCalleeID);
		bool EndVideoCallInLive(const long long& lFriendID, int nCalleeID);
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
		long long m_llLastChunkDuration;

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
		SmartPointer<CLockHandler> m_pAudioLockMutex;
	};

} //namespace MediaSDK

#endif
