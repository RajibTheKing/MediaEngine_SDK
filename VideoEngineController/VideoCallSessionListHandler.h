
#ifndef IPV_VIDEO_CALL_SESSION_LIST_HANDLER_H
#define IPV_VIDEO_CALL_SESSION_LIST_HANDLER_H

#include <stdio.h>
#include <string>
#include <map>

#include "VideoCallSession.h"
#include "ThreadTools.h"
#include "SmartPointer.h"
#include "CommonTypes.h"

namespace MediaSDK
{

	class CVideoCallSessionListHandler
	{

	public:

		CVideoCallSessionListHandler();
		~CVideoCallSessionListHandler();

		void AddToVideoSessionList(long long llFriendName, CVideoCallSession* pcVideoSession);
		CVideoCallSession* GetFromVideoSessionList(long long llFriendName);
		CVideoCallSession* GetFromVideoSessionListinIndex(int iIndex);
		bool RemoveFromVideoSessionList(long long llFriendName);
		int SizeOfVideoSessionList();
		bool IsVideoSessionExist(long long llFriendName, CVideoCallSession* &rpcVideoSession);
		bool IsVideoSessionExist(long long llFriendName);
		void ClearAllFromVideoSessionList();
		void ResetAllInVideoSessionList();
		int GetSessionListSize();

	private:

		std::map<long long, CVideoCallSession*> m_mVideoSessionList;

	protected:

		SharedPointer<CLockHandler> m_pVideoSessionListMutex;
	};

} //namespace MediaSDK

#endif