#include "VideoCallSessionListHandler.h"
#include "LogPrinter.h"

CVideoCallSessionListHandler::CVideoCallSessionListHandler()
{
	m_pVideoSessionListMutex.reset(new CLockHandler);
}

CVideoCallSessionListHandler::~CVideoCallSessionListHandler()
{
	SHARED_PTR_DELETE(m_pVideoSessionListMutex);
}

void CVideoCallSessionListHandler::AddToVideoSessionList(LongLong friendName, CVideoCallSession* videoSession)
{
	Locker lock(*m_pVideoSessionListMutex);

	m_mVideoSessionList.insert(make_pair(friendName, videoSession));

	CLogPrinter_Write(CLogPrinter::INFO, "CVideoCallSessionListHandler::AddToVideoSessionList added video Session");
}

CVideoCallSession* CVideoCallSessionListHandler::GetFromVideoSessionList(LongLong friendName)
{
	Locker lock(*m_pVideoSessionListMutex);

	std::map<LongLong, CVideoCallSession*>::iterator videoSessionSearch = m_mVideoSessionList.find(friendName);

	if (videoSessionSearch == m_mVideoSessionList.end())
	{
		return NULL;
	}
	else
	{
		return videoSessionSearch->second;
	}
}

CVideoCallSession* CVideoCallSessionListHandler::GetFromVideoSessionListinIndex(int index)
{
	Locker lock(*m_pVideoSessionListMutex);

	std::map<LongLong, CVideoCallSession*>::iterator videoSessionSearch = m_mVideoSessionList.begin();

	for (int count = 0; videoSessionSearch != m_mVideoSessionList.end(); ++videoSessionSearch, count++)
		if (count == index)
		{
			return videoSessionSearch->second;
		}
	return NULL;
}

bool CVideoCallSessionListHandler::RemoveFromVideoSessionList(LongLong friendName)
{
	Locker lock(*m_pVideoSessionListMutex);

	std::map<LongLong, CVideoCallSession*>::iterator videoSessionSearch = m_mVideoSessionList.find(friendName);

	if (videoSessionSearch == m_mVideoSessionList.end())
	{
		return false;
	}
	else
	{
		CVideoCallSession *videoSession = videoSessionSearch->second;

		if (NULL == videoSession)
		{
			return false;
		}

		delete videoSession;
		videoSession = NULL;

        if(false == m_mVideoSessionList.empty())
            m_mVideoSessionList.erase(friendName);

		return true;
	}
}

void CVideoCallSessionListHandler::ClearAllFromVideoSessionList()
{
	Locker lock(*m_pVideoSessionListMutex);

	std::map<LongLong, CVideoCallSession*>::iterator videoSessionSearch = m_mVideoSessionList.begin();

	if (videoSessionSearch == m_mVideoSessionList.end())
	{
		return;
	}

	for (; videoSessionSearch != m_mVideoSessionList.end(); ++videoSessionSearch)
	{
		CVideoCallSession *VideoSession = videoSessionSearch->second;

		if (NULL != VideoSession)
		{
			delete VideoSession;
			VideoSession = NULL;
		}	
	}

	if (false == m_mVideoSessionList.empty())
		m_mVideoSessionList.clear();
}

int CVideoCallSessionListHandler::SizeOfVideoSessionList()
{
	Locker lock(*m_pVideoSessionListMutex);

	int iSize = (int)m_mVideoSessionList.size();

	return iSize;
}

bool CVideoCallSessionListHandler::IsVideoSessionExist(LongLong lFriendName)
{
	Locker lock(*m_pVideoSessionListMutex);

	bool bReturnedValue = !(m_mVideoSessionList.find(lFriendName) == m_mVideoSessionList.end());

	return bReturnedValue;
}


bool CVideoCallSessionListHandler::IsVideoSessionExist(LongLong lFriendName, CVideoCallSession* &videoSession)
{
	Locker lock(*m_pVideoSessionListMutex);

	std::map<LongLong, CVideoCallSession*>::iterator videoSessionSearch;
	videoSessionSearch = m_mVideoSessionList.find(lFriendName);

	if (videoSessionSearch == m_mVideoSessionList.end())
	{
		return false;
	}
	else
	{
		videoSession = videoSessionSearch->second;
		return true;
	}
}

void CVideoCallSessionListHandler::ResetAllInVideoSessionList()
{
	Locker lock(*m_pVideoSessionListMutex);

	std::map<LongLong, CVideoCallSession*>::iterator videoSessionSearch = m_mVideoSessionList.begin();

	for (; videoSessionSearch != m_mVideoSessionList.end(); ++videoSessionSearch)
	{
		CVideoCallSession *videoSession = videoSessionSearch->second;

//		videoSession->ResetAllInMediaList();
	}
}