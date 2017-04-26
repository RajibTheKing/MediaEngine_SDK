#ifndef _SHARED_BUCKET_H_
#define _SHARED_BUCKET_H_

#include <stdio.h>

#include "AudioTypes.h"
#include "VideoEncoderListHandler.h"
#include "VideoCallSessionListHandler.h"
#include "AudioCallSessionListHandler.h"

class CEventNotifier;

class CCommonElementsBucket
{

public:

	CCommonElementsBucket();
	~CCommonElementsBucket();
    
	SendFunctionPointerType SendFunctionPointer = NULL;
    
    void SetUserName(const LongLong& username);
    LongLong GetUsername();
   
	CEventNotifier *m_pEventNotifier;
	CVideoCallSessionListHandler *m_pVideoCallSessionList;
	CAudioCallSessionListHandler *m_pAudioCallSessionList;
	CVideoEncoderListHandler *m_pVideoEncoderList;
	CLockHandler* GetSharedMutex();
    
	void SetSendFunctionPointer(SendFunctionPointerType sendFunc)
	{
		SendFunctionPointer = sendFunc;
	}

	SendFunctionPointerType GetSendFunctionPointer()
	{		
		return SendFunctionPointer;
	}

	void SetPacketSizeOfNetwork(int packetSizeOfNetwork);
	int GetPacketSizeOfNetwork();
    
private:

    void InstantiateSharedMutex();
	
	int m_nPacketSizeOfNetwork;
    LongLong m_friendID;
	LongLong userName;
	CLockHandler* sharedMutex;
};

#endif
