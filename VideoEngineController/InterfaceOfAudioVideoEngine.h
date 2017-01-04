
#ifndef _INTERFACE_OF_AUDIO_VIDEO_ENGINEE_H_
#define _INTERFACE_OF_AUDIO_VIDEO_ENGINEE_H_

#include <string>
#include <vector>

#include "Tools.h"

#define MEDIA_TYPE_AUDIO 1
#define MEDIA_TYPE_VIDEO 2
#define MEDIA_TYPE_LIVE_STREAM 3
#define MEDIA_TYPE_LIVE_CALL_AUDIO 4
#define MEDIA_TYPE_LIVE_CALL_VIDEO 5

#define SERVICE_TYPE_CALL 11
#define SERVICE_TYPE_SELF_CALL 13

#define ENTITY_TYPE_CALLER 31

#define SERVICE_TYPE_LIVE_STREAM 12
#define SERVICE_TYPE_SELF_STREAM 14

#define ENTITY_TYPE_PUBLISHER 31
#define ENTITY_TYPE_VIEWER 32
#define ENTITY_TYPE_VIEWER_CALLEE 2
#define ENTITY_TYPE_PUBLISHER_CALLER 1

typedef long long IPVLongType;

//#define NO_CONNECTIVITY

class CController;

class CInterfaceOfAudioVideoEngine
{

public:

	CInterfaceOfAudioVideoEngine();
	CInterfaceOfAudioVideoEngine(const char* szLoggerPath, int nLoggerPrintLevel);
	~CInterfaceOfAudioVideoEngine();

	bool Init(const IPVLongType& llUserID, const char* szLoggerPath, int nLoggerPrintLevel);
	bool InitializeLibrary(const IPVLongType& llUserID);
	bool SetUserName(const IPVLongType llUserName);
	bool StartAudioCall(const IPVLongType llFriendID, int nServiceType);

	bool SetVolume(const LongLong lFriendID, int iVolume, bool bRecorder);
	bool SetLoudSpeaker(const LongLong lFriendID, bool bOn);
	bool SetEchoCanceller(const IPVLongType llFriendID, bool bOn);
	int CancelAudioData(const IPVLongType llFriendID, short *in_data, unsigned int unLength);

	bool StartCallInLive(const IPVLongType llFriendID, int iRole);
	bool EndCallInLive(const IPVLongType llFriendID);

	bool StartVideoCall(const IPVLongType llFriendID, int nVideoHeight, int nVideoWidth, int nServiceType, int nEntityType, int packetSizeOfNetwork = 0, int nNetworkType = 0);
	int EncodeAndTransfer(const IPVLongType llFriendID, unsigned char *in_data, unsigned int unLength);
	int PushPacketForDecoding(const IPVLongType llFriendID, int mediaType, int nEntityType, unsigned char *in_data, unsigned int unLength);
	int PushAudioForDecodingVector(const IPVLongType llFriendID, int mediaType, int nEntityType, unsigned char *in_data, unsigned int unLength, std::vector< std::pair<int, int> > vMissingFrames);
	int SendAudioData(const IPVLongType llFriendID, short *in_data, unsigned int unLength);
	int SendVideoData(const IPVLongType llFriendID, unsigned char *in_data, unsigned int unLength, unsigned int nOrientationType = 0, int device_orientation = 0);
	int SetEncoderHeightWidth(const IPVLongType llFriendID, int nVideoHeight, int nVideoWidth);
	int SetBitRate(const IPVLongType llFriendID, int nBitRate);

	int CheckDeviceCapability(const LongLong& lFriendID, int iHeightHigh, int iWidthHigh, int iHeightLow, int iWidthLow);
	int SetDeviceCapabilityResults(int iNotification, int iHeightHigh, int iWidthHigh, int iHeightLow, int iWidthLow);

	bool StopAudioCall(const IPVLongType llFriendID);
	bool StopVideoCall(const IPVLongType llFriendID);
	void SetLoggerPath(std::string strLoggerPath);
	bool SetLoggingState(bool bLoggingState, int nLogLevel);
	void UninitializeLibrary();

	int StartAudioEncodeDecodeSession();
	int EncodeAudioFrame(short *psaEncodingDataBuffer, int nAudioFrameSize, unsigned char *ucaEncodedDataBuffer);
	int DecodeAudioFrame(unsigned char *ucaDecodedDataBuffer, int nAudioFrameSize, short *psaDecodingDataBuffer);
	int StopAudioEncodeDecodeSession();

	void SetNotifyClientWithPacketCallback(void(*callBackFunctionPointer)(long long, unsigned char*, int));
	void SetNotifyClientWithVideoDataCallback(void(*callBackFunctionPointer)(long long, int, unsigned char*, int, int, int, int));
	void SetNotifyClientWithVideoNotificationCallback(void(*callBackFunctionPointer)(long long, int));
	void SetNotifyClientWithNetworkStrengthNotificationCallback(void(*callBackFunctionPointer)(IPVLongType, int));
	void SetNotifyClientWithAudioDataCallback(void(*callBackFunctionPointer)(long long, int, short*, int));
	void SetNotifyClientWithAudioPacketDataCallback(void(*callBackFunctionPointer)(IPVLongType, unsigned char*, int));
	void SetNotifyClientWithAudioAlarmCallback(void(*callBackFunctionPointer)(long long, short*, int));

	void SetSendFunctionPointer(void(*callBackFunctionPointer)(IPVLongType, int, unsigned char*, int, int));

	int StartVideoMuxingAndEncodeSession(unsigned char *pBMP32Data, int iLen, int nVideoHeight, int nVideoWidth);
	int FrameMuxAndEncode(unsigned char *pVideoYuv, int iHeight, int iWidth);
	int StopVideoMuxingAndEncodeSession(unsigned char *finalData);
	void InterruptOccured(const LongLong lFriendID);
	void InterruptOver(const LongLong lFriendID);

private:

	Tools m_Tools;

	long long m_llTimeOffset;

	CController* m_pcController;
};

#endif
