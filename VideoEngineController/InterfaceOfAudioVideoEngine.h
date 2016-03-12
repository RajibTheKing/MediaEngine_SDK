#ifndef _INTERFACE_OF_AUDIO_VIDEO_ENGINEE_H_
#define _INTERFACE_OF_AUDIO_VIDEO_ENGINEE_H_

#include <stdio.h>
#include <string>

void abc();

class CController;

class CInterfaceOfAudioVideoEngine
{
    
public:
    
    CInterfaceOfAudioVideoEngine();
    CInterfaceOfAudioVideoEngine(const char* sLoggerPath, int iLoggerPrintLevel);
     ~CInterfaceOfAudioVideoEngine();
    
     bool Init(const IPVLongType& lUserID, const char* sLoggerPath, int iLoggerPrintLevel);
     bool InitializeLibrary(const IPVLongType& lUserID);
     bool SetUserName(const IPVLongType lUserName);
     bool StartAudioCall(const IPVLongType lFriendID);
     bool StartVideoCall(const IPVLongType lFriendID, int iVideoHeight, int iVideoWidth, int iNetworkType);
     int EncodeAndTransfer(const IPVLongType lFriendID, unsigned char *in_data, unsigned int in_size);
     int PushPacketForDecoding(const IPVLongType lFriendID, unsigned char *in_data, unsigned int in_size);
     int PushAudioForDecoding(const IPVLongType lFriendID, unsigned char *in_data, unsigned int in_size);
     int SendAudioData(const IPVLongType lFriendID, short *in_data, unsigned int in_size);
//     int SendVideoData(const IPVLongType lFriendID, unsigned char *in_data, unsigned int in_size);
     int SendVideoData(const IPVLongType lFriendID, unsigned char *in_data, unsigned int in_size, unsigned int orientation_type);
     int SetHeightWidth(const IPVLongType lFriendID, int width, int height);
     int SetBitRate(const IPVLongType lFriendID, int bitRate);
     bool StopAudioCall(const IPVLongType lFriendID);
     bool StopVideoCall(const IPVLongType lFriendID);
     void SetLoggerPath(std::string sLoggerPath);
     bool SetLoggingState(bool loggingState, int logLevel);
     void UninitializeLibrary();
    
     void SetNotifyClientWithPacketCallback(void(*callBackFunctionPointer)(IPVLongType, unsigned char*, int));
     void SetNotifyClientWithVideoDataCallback(void(*callBackFunctionPointer)(IPVLongType, unsigned char*, int, int, int));
	 void SetNotifyClientWithVideoNotificationCallback(void(*callBackFunctionPointer)(IPVLongType, int));
     void SetNotifyClientWithAudioDataCallback(void(*callBackFunctionPointer)(IPVLongType, short*, int));
     void SetNotifyClientWithAudioPacketDataCallback(void(*callBackFunctionPointer)(IPVLongType, unsigned char*, int));

     void SetSendFunctionPointer(void(*callBackFunctionPointer)(IPVLongType, int, unsigned char*, int));
    
private:
    
    CController* m_pController;
};

#endif