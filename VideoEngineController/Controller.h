
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

using namespace std;


class CController
{

public:

	CController();
	CController(const char* sLoggerPath, int iLoggerPrintLevel);
	~CController();

	bool SetUserName(const LongLong& lUserName);
	bool StartAudioCall(const LongLong& lFriendID);
	bool StartVideoCall(const LongLong& lFriendID, int iVideoHeight, int iVideoWidth);
	int EncodeAndTransfer(const LongLong& lFriendID, unsigned char *in_data, unsigned int in_size);
	int PushPacketForDecoding(const LongLong& lFriendID, unsigned char *in_data, unsigned int in_size);
	int PushAudioForDecoding(const LongLong& lFriendID, unsigned char *in_data, unsigned int in_size);
	int SendAudioData(const LongLong& lFriendID, short *in_data, unsigned int in_size);
//	int SendVideoData(const LongLong& lFriendID, unsigned char *in_data, unsigned int in_size);
	int SendVideoData(const LongLong& lFriendID, unsigned char *in_data, unsigned int in_size, unsigned int orientation_type);
	int SetHeightWidth(const LongLong& lFriendID, int width, int height); 
	int SetBitRate(const LongLong& lFriendID, int bitRate);
	bool StopAudioCall(const LongLong& lFriendID);
	bool StopVideoCall(const LongLong& lFriendID);
	void TempChange();
	void initializeEventHandler();
	void SetLoggerPath(std::string);
    bool SetLoggingState(bool loggingState, int logLevel);
	void UninitializeLibrary();

	void SetNotifyClientWithPacketCallback(void(*callBackFunctionPointer)(LongLong, unsigned char*, int));
	void SetNotifyClientWithVideoDataCallback(void(*callBackFunctionPointer)(LongLong, unsigned char*, int, int, int));
	void SetNotifyClientWithVideoNotificationCallback(void(*callBackFunctionPointer)(LongLong, int));
	void SetNotifyClientWithAudioDataCallback(void(*callBackFunctionPointer)(LongLong, short*, int));
    void SetNotifyClientWithAudioPacketDataCallback(void(*callBackFunctionPointer)(IPVLongType, unsigned char*, int));

    void SetSendFunctionPointer(void(*callBackFunctionPointer)(IPVLongType, int, unsigned char*, int));
	
private:

	CEventNotifier m_EventNotifier;

	int iLoggerPrintLevel;
	std::string logFilePath;
	Tools m_Tools;

	CCommonElementsBucket *m_pCommonElementsBucket;
    
    SmartPointer<CLockHandler> m_pVideoSendMutex;
    SmartPointer<CLockHandler> m_pVideoReceiveMutex;
    SmartPointer<CLockHandler> m_pAudioSendMutex;
    SmartPointer<CLockHandler> m_pAudioReceiveMutex;
};

#endif
