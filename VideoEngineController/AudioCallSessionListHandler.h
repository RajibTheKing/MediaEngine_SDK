#ifndef _AUDIO_CALL_SESSION_LIST_HANDLER_H_
#define _AUDIO_CALL_SESSION_LIST_HANDLER_H_


#include <map>
#include "SmartPointer.h"
#include "CommonTypes.h"

namespace MediaSDK
{
	class CAudioCallSession;

	class CAudioCallSessionListHandler
	{

	public:

		CAudioCallSessionListHandler();
		~CAudioCallSessionListHandler();

		void AddToAudioSessionList(long long friendName, CAudioCallSession* AudioSession);
		CAudioCallSession* GetFromAudioSessionList(long long friendName);
		bool RemoveFromAudioSessionList(long long friendName);
		int SizeOfAudioSessionList();
		bool IsAudioSessionExist(long long lFriendName, CAudioCallSession* &AudioSession);
		bool IsAudioSessionExist(long long lFriendName);
		void ClearAllFromAudioSessionList();

	private:

		std::map<long long, CAudioCallSession*> m_mAudioSessionList;

	protected:

		SmartPointer<CLockHandler> m_pAudioSessionListMutex;
	};

} //namespace MediaSDK

#endif