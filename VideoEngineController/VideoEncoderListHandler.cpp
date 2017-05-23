
#include "VideoEncoderListHandler.h"
#include "LogPrinter.h"
#include "ThreadTools.h"

namespace MediaSDK
{

	CVideoEncoderListHandler::CVideoEncoderListHandler()
	{
		m_pVideoEncoderListMutex.reset(new CLockHandler);
	}

	CVideoEncoderListHandler::~CVideoEncoderListHandler()
	{
		SHARED_PTR_DELETE(m_pVideoEncoderListMutex);
	}

	void CVideoEncoderListHandler::AddToVideoEncoderList(long long llFriendID, CVideoEncoder* pcVideoEncoder)
	{
		Locker lock(*m_pVideoEncoderListMutex);

		CLogPrinter_Write(CLogPrinter::INFO, "CVideoEncoderListHandler::CVideoEncoderListHandler");

		m_mVideoEncoderList.insert(make_pair(llFriendID, pcVideoEncoder));

		CLogPrinter_Write(CLogPrinter::INFO, "CVideoEncoderListHandler::CVideoEncoderListHandler 2");
	}

	CVideoEncoder* CVideoEncoderListHandler::GetFromVideoEncoderList(long long llFriendID)
	{
		Locker lock(*m_pVideoEncoderListMutex);

		map<long long, CVideoEncoder*>::iterator mediaSearch = m_mVideoEncoderList.find(llFriendID);

		if (mediaSearch == m_mVideoEncoderList.end())
		{
			return NULL;
		}
		else
		{
			return mediaSearch->second;
		}
	}

	CVideoEncoder* CVideoEncoderListHandler::GetFromVideoEncoderListinIndex(int iIndex)
	{
		Locker lock(*m_pVideoEncoderListMutex);

		map<long long, CVideoEncoder*>::iterator mediaSearch = m_mVideoEncoderList.begin();

		for (int count = 0; mediaSearch != m_mVideoEncoderList.end(); ++mediaSearch, count++)
		{
			if (count == iIndex)
			{
				return mediaSearch->second;
			}
		}

		return NULL;
	}

	void CVideoEncoderListHandler::ClearAllFromVideoEncoderList()
	{
		Locker lock(*m_pVideoEncoderListMutex);

		if (false == m_mVideoEncoderList.empty())
			m_mVideoEncoderList.clear();
	}

	bool CVideoEncoderListHandler::RemoveFromVideoEncoderList(long long llFriendID)
	{
		Locker lock(*m_pVideoEncoderListMutex);

		map<long long, CVideoEncoder*>::iterator mediaSearch = m_mVideoEncoderList.find(llFriendID);

		if (mediaSearch == m_mVideoEncoderList.end())
		{
			return false;
		}
		else
		{
			CVideoEncoder *cVideoEncoder = mediaSearch->second;

			if (NULL == cVideoEncoder)
			{
				return false;
			}

			m_mVideoEncoderList.erase(llFriendID);

			/*delete cVideoEncoder;

			cVideoEncoder = NULL;*/

			return true;
		}
	}

	int CVideoEncoderListHandler::SizeOfVideoEncoderList()
	{
		Locker lock(*m_pVideoEncoderListMutex);

		return (int)m_mVideoEncoderList.size();
	}

	bool CVideoEncoderListHandler::IsVideoEncoderExist(long long llFriendID)
	{
		Locker lock(*m_pVideoEncoderListMutex);

		return !(m_mVideoEncoderList.find(llFriendID) == m_mVideoEncoderList.end());
	}

	bool CVideoEncoderListHandler::IsVideoEncoderExist(int nVideoHeight, int nVideoWidth)
	{
		Locker lock(*m_pVideoEncoderListMutex);

		/*	map<long long, CVideoEncoder*>::iterator mediaSearch = m_mVideoEncoderList.find(friendName);

			if (mediaSearch == m_mVideoEncoderList.end())
			return false;
			else
			{
			media = mediaSearch->second;

			return true;
			}*/

		return false;
	}

	void CVideoEncoderListHandler::ResetAllInVideoEncoderList()
	{
		Locker lock(*m_pVideoEncoderListMutex);

		map<long long, CVideoEncoder*>::iterator mediaSearch = m_mVideoEncoderList.begin();

		for (; mediaSearch != m_mVideoEncoderList.end(); ++mediaSearch)
		{
			CVideoEncoder *cVideoEncoder = mediaSearch->second;

			//		cVideoEncoder->ResetSocketOfThisVideoEncoder();
		}
	}

} //namespace MediaSDK
