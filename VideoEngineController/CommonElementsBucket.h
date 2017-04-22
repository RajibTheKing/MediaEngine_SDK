
#ifndef IPV_COMMON_ELEMENTS_BUCKET_H
#define IPV_COMMON_ELEMENTS_BUCKET_H

#include <stdio.h>

#include "EventNotifier.h"
#include "VideoEncoderListHandler.h"
#include "VideoCallSessionListHandler.h"
#include "AudioCallSessionListHandler.h"

class CCommonElementsBucket
{

public:

	CCommonElementsBucket();
	~CCommonElementsBucket();
    
	void(*SendFunctionPointer)(long long, int, unsigned char*, int, int, std::vector< std::pair<int, int> >) = NULL;
    
    void SetUserName(const long long& username);
    long long GetUsername();
   
	CEventNotifier *m_pEventNotifier;
	CVideoCallSessionListHandler *m_pVideoCallSessionList;
	CAudioCallSessionListHandler *m_pAudioCallSessionList;
	CVideoEncoderListHandler *m_pVideoEncoderList;
	CLockHandler* GetSharedMutex();
    
	void SetSendFunctionPointer(void(*callBackFunctionPointer)(long long, int, unsigned char*, int, int, std::vector< std::pair<int, int> > vAudioBlocks));

	void SetPacketSizeOfNetwork(int packetSizeOfNetwork);
	int GetPacketSizeOfNetwork();
    
private:

    void InstantiateSharedMutex();
	
	int m_nPacketSizeOfNetwork;
    long long m_friendID;
	long long userName;
	CLockHandler* sharedMutex;
};

#endif
