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

	CLogPrinter::Write(CLogPrinter::INFO, "CAudioCallSessionListHandler::AddToAudioSessionList added video Session");
}

CAudioCallSession* CAudioCallSessionListHandler::GetFromAudioSessionList(LongLong friendName)
{
	Locker lock(*m_pAudioSessionListMutex);

	std::map<LongLong, CAudioCallSession*>::iterator videoSessionSearch = m_mAudioSessionList.find(friendName);

	if (videoSessionSearch == m_mAudioSessionList.end())
	{
		return NULL;
	}
	else
	{
		return videoSessionSearch->second;
	}
}

CAudioCallSession* CAudioCallSessionListHandler::GetFromAudioSessionListinIndex(int index)
{
	Locker lock(*m_pAudioSessionListMutex);

	std::map<LongLong, CAudioCallSession*>::iterator videoSessionSearch = m_mAudioSessionList.begin();

	for (int count = 0; videoSessionSearch != m_mAudioSessionList.end(); ++videoSessionSearch, count++)
		if (count == index)
		{
			return videoSessionSearch->second;
		}
	return NULL;
}

bool CAudioCallSessionListHandler::RemoveFromAudioSessionList(LongLong friendName)
{
	Locker lock(*m_pAudioSessionListMutex);

	std::map<LongLong, CAudioCallSession*>::iterator videoSessionSearch = m_mAudioSessionList.find(friendName);

	if (videoSessionSearch == m_mAudioSessionList.end())
	{
		return false;
	}
	else
	{
		CAudioCallSession *videoSession = videoSessionSearch->second;

		if (NULL == videoSession)
		{
			return false;
		}

		delete videoSession;
		videoSession = NULL;

        if( false == m_mAudioSessionList.empty())
            m_mAudioSessionList.erase(friendName);

		return true;
	}
}

void CAudioCallSessionListHandler::ClearAllFromAudioSessionList()
{
	Locker lock(*m_pAudioSessionListMutex);

	std::map<LongLong, CAudioCallSession*>::iterator videoSessionSearch = m_mAudioSessionList.begin();

	if (videoSessionSearch == m_mAudioSessionList.end())
	{
		return;
	}

	for (; videoSessionSearch != m_mAudioSessionList.end(); ++videoSessionSearch)
	{
		CAudioCallSession *AudioSession = videoSessionSearch->second;

		if (NULL != AudioSession)
		{
			delete AudioSession;
			AudioSession = NULL;
		}
	}

	if (false == m_mAudioSessionList.empty())
		m_mAudioSessionList.clear();
}

int CAudioCallSessionListHandler::SizeOfAudioSessionList()
{
	Locker lock(*m_pAudioSessionListMutex);

	int iSize = (int)m_mAudioSessionList.size();

	return iSize;
}

bool CAudioCallSessionListHandler::IsAudioSessionExist(LongLong lFriendName)
{
	Locker lock(*m_pAudioSessionListMutex);

	bool bReturnedValue = !(m_mAudioSessionList.find(lFriendName) == m_mAudioSessionList.end());

	return bReturnedValue;
}


bool CAudioCallSessionListHandler::IsAudioSessionExist(LongLong lFriendName, CAudioCallSession* &videoSession)
{
	Locker lock(*m_pAudioSessionListMutex);

	std::map<LongLong, CAudioCallSession*>::iterator videoSessionSearch;
	videoSessionSearch = m_mAudioSessionList.find(lFriendName);

	if (videoSessionSearch == m_mAudioSessionList.end())
	{
		return false;
	}
	else
	{
		videoSession = videoSessionSearch->second;
		return true;
	}
}

void CAudioCallSessionListHandler::ResetAllInAudioSessionList()
{
	Locker lock(*m_pAudioSessionListMutex);

	std::map<LongLong, CAudioCallSession*>::iterator videoSessionSearch = m_mAudioSessionList.begin();

	for (; videoSessionSearch != m_mAudioSessionList.end(); ++videoSessionSearch)
	{
		CAudioCallSession *videoSession = videoSessionSearch->second;

		//		videoSession->ResetAllInMediaList();
	}
}