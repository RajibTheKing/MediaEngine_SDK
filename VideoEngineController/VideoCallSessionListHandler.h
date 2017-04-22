
#ifndef IPV_VIDEO_CALL_SESSION_LIST_HANDLER_H
#define IPV_VIDEO_CALL_SESSION_LIST_HANDLER_H

#include <stdio.h>
#include <string>
#include <map>
#include "VideoCallSession.h"
#include "ThreadTools.h"
#include "SmartPointer.h"
#include "LockHandler.h"

class CVideoCallSessionListHandler
{

public:

	CVideoCallSessionListHandler();
	~CVideoCallSessionListHandler();

	void AddToVideoSessionList(LongLong friendName, CVideoCallSession* VideoSession);
	CVideoCallSession* GetFromVideoSessionList(LongLong friendName);
	CVideoCallSession* GetFromVideoSessionListinIndex(int index);
	bool RemoveFromVideoSessionList(LongLong friendName);
	int SizeOfVideoSessionList();
	bool IsVideoSessionExist(LongLong lFriendName, CVideoCallSession* &VideoSession);
	bool IsVideoSessionExist(LongLong lFriendName);
	void ClearAllFromVideoSessionList();
	void ResetAllInVideoSessionList();

private:

	std::map<LongLong, CVideoCallSession*> m_mVideoSessionList;

protected:

	SmartPointer<CLockHandler> m_pVideoSessionListMutex;
};

#endif