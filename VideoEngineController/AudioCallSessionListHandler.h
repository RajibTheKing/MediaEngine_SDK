#ifndef _AUDIO_CALL_SESSION_LIST_HANDLER_H_
#define _AUDIO_CALL_SESSION_LIST_HANDLER_H_

#include <stdio.h>
#include <string>
#include <map>

#include "AudioCallSession.h"
#include "ThreadTools.h"
#include "SmartPointer.h"
#include "LockHandler.h"

class CAudioCallSessionListHandler
{

public:

	CAudioCallSessionListHandler();
	~CAudioCallSessionListHandler();

	void AddToAudioSessionList(LongLong friendName, CAudioCallSession* AudioSession);
	CAudioCallSession* GetFromAudioSessionList(LongLong friendName);
	CAudioCallSession* GetFromAudioSessionListinIndex(int index);
	bool RemoveFromAudioSessionList(LongLong friendName);
	int SizeOfAudioSessionList();
	bool IsAudioSessionExist(LongLong lFriendName, CAudioCallSession* &AudioSession);
	bool IsAudioSessionExist(LongLong lFriendName);
	void ClearAllFromAudioSessionList();
	void ResetAllInAudioSessionList();

private:

	std::map<LongLong, CAudioCallSession*> m_mAudioSessionList;

protected:

	SmartPointer<CLockHandler> m_pAudioSessionListMutex;
};

#endif