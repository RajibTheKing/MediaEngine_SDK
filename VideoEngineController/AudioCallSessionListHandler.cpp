#include "AudioCallSessionListHandler.h"
#include "LogPrinter.h"
#include "AudioCallSession.h"
#include "ThreadTools.h"



namespace MediaSDK
{

	CAudioCallSessionListHandler::CAudioCallSessionListHandler()
	{
		m_pAudioSessionListMutex.reset(new CLockHandler);
	}

	CAudioCallSessionListHandler::~CAudioCallSessionListHandler()
	{
		SHARED_PTR_DELETE(m_pAudioSessionListMutex);
	}

	void CAudioCallSessionListHandler::AddToAudioSessionList(long long friendName, CAudioCallSession* videoSession)
	{
		AudioListHandlerLock lock(*m_pAudioSessionListMutex);

		m_mAudioSessionList.insert(make_pair(friendName, videoSession));

		CLogPrinter_Write(CLogPrinter::INFO, "CAudioCallSessionListHandler::AddToAudioSessionList added video Session");
	}

	CAudioCallSession* CAudioCallSessionListHandler::GetFromAudioSessionList(long long friendName)
	{
		AudioListHandlerLock lock(*m_pAudioSessionListMutex);

		auto audioSessionSearch = m_mAudioSessionList.find(friendName);

		if (audioSessionSearch != m_mAudioSessionList.end())
		{
			return audioSessionSearch->second;
		}
		return NULL;
	}


	bool CAudioCallSessionListHandler::RemoveFromAudioSessionList(long long friendName)
	{
		AudioListHandlerLock lock(*m_pAudioSessionListMutex);
		auto audioSessionSearch = m_mAudioSessionList.find(friendName);

		if (audioSessionSearch != m_mAudioSessionList.end())
		{
			CAudioCallSession *audioSession = audioSessionSearch->second;
			if (NULL != audioSession)
			{
				delete audioSession;
				audioSession = NULL;
			}
			m_mAudioSessionList.erase(audioSessionSearch);
			return true;
		}
		return false;
	}

	void CAudioCallSessionListHandler::ClearAllFromAudioSessionList()
	{
		AudioListHandlerLock lock(*m_pAudioSessionListMutex);

		for (auto audioSessionIter = m_mAudioSessionList.begin(); audioSessionIter != m_mAudioSessionList.end(); ++audioSessionIter)
		{
			CAudioCallSession *AudioSession = audioSessionIter->second;

			if (NULL != AudioSession)
			{
				delete AudioSession;
				AudioSession = NULL;
			}
		}
		m_mAudioSessionList.clear();
	}

	int CAudioCallSessionListHandler::SizeOfAudioSessionList()
	{
		AudioListHandlerLock lock(*m_pAudioSessionListMutex);
		return (int)m_mAudioSessionList.size();
	}

	bool CAudioCallSessionListHandler::IsAudioSessionExist(long long lFriendName)
	{
		AudioListHandlerLock lock(*m_pAudioSessionListMutex);

		bool bReturnedValue = !(m_mAudioSessionList.find(lFriendName) == m_mAudioSessionList.end());

		return bReturnedValue;
	}


	bool CAudioCallSessionListHandler::IsAudioSessionExist(long long lFriendName, CAudioCallSession* &audioSession)
	{
		AudioListHandlerLock lock(*m_pAudioSessionListMutex);

		auto audioSessionSearch = m_mAudioSessionList.find(lFriendName);

		if (audioSessionSearch != m_mAudioSessionList.end())
		{
			audioSession = audioSessionSearch->second;
			return true;
		}
		return false;
	}

} //namespace MediaSDK

