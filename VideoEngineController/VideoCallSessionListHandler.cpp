
#include "VideoCallSessionListHandler.h"
#include "LogPrinter.h"

namespace MediaSDK
{

	CVideoCallSessionListHandler::CVideoCallSessionListHandler()
	{
		m_pVideoSessionListMutex.reset(new CLockHandler);
	}

	CVideoCallSessionListHandler::~CVideoCallSessionListHandler()
	{
		SHARED_PTR_DELETE(m_pVideoSessionListMutex);
	}

	void CVideoCallSessionListHandler::AddToVideoSessionList(long long llFriendName, CVideoCallSession* pcVideoSession)
	{
		SessionListLocker lock(*m_pVideoSessionListMutex);

		m_mVideoSessionList.insert(make_pair(llFriendName, pcVideoSession));

		CLogPrinter_Write(CLogPrinter::INFO, "CVideoCallSessionListHandler::AddToVideoSessionList added video Session");
	}

	CVideoCallSession* CVideoCallSessionListHandler::GetFromVideoSessionList(long long llFriendName)
	{
		SessionListLocker lock(*m_pVideoSessionListMutex);

		std::map<long long, CVideoCallSession*>::iterator videoSessionSearch = m_mVideoSessionList.find(llFriendName);

		if (videoSessionSearch == m_mVideoSessionList.end())
		{
			return NULL;
		}
		else
		{
			return videoSessionSearch->second;
		}
	}

	CVideoCallSession* CVideoCallSessionListHandler::GetFromVideoSessionListinIndex(int iIndex)
	{
		SessionListLocker lock(*m_pVideoSessionListMutex);

		std::map<long long, CVideoCallSession*>::iterator videoSessionSearch = m_mVideoSessionList.begin();

		for (int count = 0; videoSessionSearch != m_mVideoSessionList.end(); ++videoSessionSearch, count++)
		{
			if (count == iIndex)
			{
				return videoSessionSearch->second;
			}
		}

		return NULL;
	}

	bool CVideoCallSessionListHandler::RemoveFromVideoSessionList(long long llFriendName)
	{
		CLogPrinter_WriteLog(CLogPrinter::INFO, CHECK_CAPABILITY_LOG, "CVideoCallSessionListHandler::RemoveFromVideoSessionList() called");

		SessionListLocker lock(*m_pVideoSessionListMutex);

		CLogPrinter_WriteLog(CLogPrinter::INFO, CHECK_CAPABILITY_LOG, "CVideoCallSessionListHandler::RemoveFromVideoSessionList() checking session key");

		std::map<long long, CVideoCallSession*>::iterator videoSessionSearch = m_mVideoSessionList.find(llFriendName);

		if (videoSessionSearch == m_mVideoSessionList.end())
		{
			CLogPrinter_WriteLog(CLogPrinter::INFO, CHECK_CAPABILITY_LOG, "CVideoCallSessionListHandler::RemoveFromVideoSessionList() session key not found");

			return false;
		}
		else
		{
			CVideoCallSession *videoSession = videoSessionSearch->second;

			if (NULL == videoSession)
			{
				CLogPrinter_WriteLog(CLogPrinter::INFO, CHECK_CAPABILITY_LOG, "CVideoCallSessionListHandler::RemoveFromVideoSessionList() session is NULL");
				return false;
			}

			delete videoSession;
			videoSession = NULL;

			CLogPrinter_WriteLog(CLogPrinter::INFO, CHECK_CAPABILITY_LOG, "CVideoCallSessionListHandler::RemoveFromVideoSessionList() session deleted");

			if (false == m_mVideoSessionList.empty())
				m_mVideoSessionList.erase(llFriendName);

			CLogPrinter_WriteLog(CLogPrinter::INFO, CHECK_CAPABILITY_LOG, "CVideoCallSessionListHandler::RemoveFromVideoSessionList() session removed from session list");

			return true;
		}
	}

	void CVideoCallSessionListHandler::ClearAllFromVideoSessionList()
	{
		SessionListLocker lock(*m_pVideoSessionListMutex);

		std::map<long long, CVideoCallSession*>::iterator videoSessionSearch = m_mVideoSessionList.begin();

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
		SessionListLocker lock(*m_pVideoSessionListMutex);

		int iSize = (int)m_mVideoSessionList.size();

		return iSize;
	}

	bool CVideoCallSessionListHandler::IsVideoSessionExist(long long llFriendName)
	{
		SessionListLocker lock(*m_pVideoSessionListMutex);

		bool bReturnedValue = !(m_mVideoSessionList.find(llFriendName) == m_mVideoSessionList.end());

		return bReturnedValue;
	}


	bool CVideoCallSessionListHandler::IsVideoSessionExist(long long llFriendName, CVideoCallSession* &rpcvideoSession)
	{
		SessionListLocker lock(*m_pVideoSessionListMutex);

		std::map<long long, CVideoCallSession*>::iterator videoSessionSearch;
		videoSessionSearch = m_mVideoSessionList.find(llFriendName);

		if (videoSessionSearch == m_mVideoSessionList.end())
		{
			return false;
		}
		else
		{
			rpcvideoSession = videoSessionSearch->second;
			return true;
		}
	}

	int CVideoCallSessionListHandler::GetSessionListSize()
	{
		SessionListLocker lock(*m_pVideoSessionListMutex);

		return m_mVideoSessionList.size();
	}

	void CVideoCallSessionListHandler::ResetAllInVideoSessionList()
	{
		SessionListLocker lock(*m_pVideoSessionListMutex);

		std::map<long long, CVideoCallSession*>::iterator videoSessionSearch = m_mVideoSessionList.begin();

		for (; videoSessionSearch != m_mVideoSessionList.end(); ++videoSessionSearch)
		{
			CVideoCallSession *videoSession = videoSessionSearch->second;

			//		videoSession->ResetAllInMediaList();
		}
	}

} //namespace MediaSDK
