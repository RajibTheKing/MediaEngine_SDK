
#ifndef IPV_COMMON_ELEMENTS_BUCKET_H
#define IPV_COMMON_ELEMENTS_BUCKET_H

#include <stdio.h>

#include "AudioTypes.h"
#include "VideoEncoderListHandler.h"
#include "VideoCallSessionListHandler.h"
#include "AudioCallSessionListHandler.h"

namespace MediaSDK
{

	class CEventNotifier;

	class CCommonElementsBucket
	{

	public:

		CCommonElementsBucket();
		~CCommonElementsBucket();

		SendFunctionPointerType SendFunctionPointer = NULL;

		void SetUserName(const long long& username);
		long long GetUsername();

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
		long long m_friendID;
		long long userName;
		CLockHandler* sharedMutex;
	};

} //namespace MediaSDK

#endif
