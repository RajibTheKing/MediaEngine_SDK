#include "AudioCallSessionListHandler.h"
#include "LogPrinter.h"

CAudioCallSessionListHandler::CAudioCallSessionListHandler()
{
	m_pAudioSessionListMutex.reset(new CLockHandler);
}

CAudioCallSessionListHandler::~CAudioCallSessionListHandler()
{
	SHARED_PTR_DELETE(m_pAudioSessionListMutex);
}

void CAudioCallSessionListHandler::AddToAudioSessionList(LongLong friendName, CAudioCallSession* videoSession)
{
	Locker lock(*m_pAudioSessionListMutex);

	m_mAudioSessionList.insert(make_pair(friendName, videoSession));

	CLogPrinter_Write(CLogPrinter::INFO, "CAudioCallSessionListHandler::AddToAudioSessionList added video Session");
}

CAudioCallSession* CAudioCallSessionListHandler::GetFromAudioSessionList(LongLong friendName)
{
	Locker lock(*m_pAudioSessionListMutex);

	auto audioSessionSearch = m_mAudioSessionList.find(friendName);

	if (audioSessionSearch != m_mAudioSessionList.end())
	{
		return audioSessionSearch->second;
	}
	return NULL;
}

CAudioCallSession* CAudioCallSessionListHandler::GetFromAudioSessionListinIndex(int index)
{
	Locker lock(*m_pAudioSessionListMutex);

	auto audioSessionSearch = m_mAudioSessionList.begin();

	for (int count = 0; audioSessionSearch != m_mAudioSessionList.end(); ++audioSessionSearch, count++) 
	{
		if (count == index)
		{
			return audioSessionSearch->second;
		}
	}
	return NULL;
}

bool CAudioCallSessionListHandler::RemoveFromAudioSessionList(LongLong friendName)
{
	Locker lock(*m_pAudioSessionListMutex);
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
	Locker lock(*m_pAudioSessionListMutex);

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
	Locker lock(*m_pAudioSessionListMutex);
	return (int)m_mAudioSessionList.size();
}

bool CAudioCallSessionListHandler::IsAudioSessionExist(LongLong lFriendName)
{
	Locker lock(*m_pAudioSessionListMutex);

	bool bReturnedValue = !(m_mAudioSessionList.find(lFriendName) == m_mAudioSessionList.end());

	return bReturnedValue;
}


bool CAudioCallSessionListHandler::IsAudioSessionExist(LongLong lFriendName, CAudioCallSession* &audioSession)
{
	Locker lock(*m_pAudioSessionListMutex);

	auto audioSessionSearch = m_mAudioSessionList.find(lFriendName);

	if (audioSessionSearch != m_mAudioSessionList.end())
	{
		audioSession = audioSessionSearch->second;
		return true;
	}
	return false;
}
