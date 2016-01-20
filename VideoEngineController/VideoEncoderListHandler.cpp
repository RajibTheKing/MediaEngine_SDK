#include "VideoEncoderListHandler.h"
#include "LogPrinter.h"

CVideoEncoderListHandler::CVideoEncoderListHandler()
{
	m_pVideoEncoderListMutex.reset(new CLockHandler);
}

CVideoEncoderListHandler::~CVideoEncoderListHandler()
{
	SHARED_PTR_DELETE(m_pVideoEncoderListMutex);
}

void CVideoEncoderListHandler::AddToVideoEncoderList(LongLong lFriendID, CVideoEncoder* cVideoEncoder)
{
	Locker lock(*m_pVideoEncoderListMutex);

	CLogPrinter_Write(CLogPrinter::INFO, "CVideoEncoderListHandler::CVideoEncoderListHandler");

	VideoEncoderList.insert(make_pair(lFriendID, cVideoEncoder));

	CLogPrinter_Write(CLogPrinter::INFO, "CVideoEncoderListHandler::CVideoEncoderListHandler 2");
}

CVideoEncoder* CVideoEncoderListHandler::GetFromVideoEncoderList(LongLong lFriendID)
{
	Locker lock(*m_pVideoEncoderListMutex);

	map<LongLong, CVideoEncoder*>::iterator mediaSearch = VideoEncoderList.find(lFriendID);

	if (mediaSearch == VideoEncoderList.end())
	{
		return NULL;
	}
	else
	{
		return mediaSearch->second;
	}
}

CVideoEncoder* CVideoEncoderListHandler::GetFromVideoEncoderListinIndex(int index)
{
	Locker lock(*m_pVideoEncoderListMutex);

	map<LongLong, CVideoEncoder*>::iterator mediaSearch = VideoEncoderList.begin();
	
	for (int count = 0; mediaSearch != VideoEncoderList.end(); ++mediaSearch, count++)
	{
		if (count == index)
		{
			return mediaSearch->second;
		}
	}

	return NULL;
}

void CVideoEncoderListHandler::ClearAllFromVideoEncoderList()
{
	Locker lock(*m_pVideoEncoderListMutex);

	if (false == VideoEncoderList.empty())
		VideoEncoderList.clear();
}

bool CVideoEncoderListHandler::RemoveFromVideoEncoderList(LongLong lFriendID)
{
	Locker lock(*m_pVideoEncoderListMutex);

	map<LongLong, CVideoEncoder*>::iterator mediaSearch = VideoEncoderList.find(lFriendID);

	if (mediaSearch == VideoEncoderList.end())
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

		VideoEncoderList.erase(lFriendID);

		/*delete cVideoEncoder;

		cVideoEncoder = NULL;*/

		return true;
	}	
}

int CVideoEncoderListHandler::SizeOfVideoEncoderList()
{
	Locker lock(*m_pVideoEncoderListMutex);

	return (int)VideoEncoderList.size();
}

bool CVideoEncoderListHandler::IsVideoEncoderExist(LongLong lFriendID)
{
	Locker lock(*m_pVideoEncoderListMutex);

	return !(VideoEncoderList.find(lFriendID) == VideoEncoderList.end());
}

bool CVideoEncoderListHandler::IsVideoEncoderExist(int iVideoHeight, int iVideoWidth)
{
	Locker lock(*m_pVideoEncoderListMutex);

/*	map<LongLong, CVideoEncoder*>::iterator mediaSearch = VideoEncoderList.find(friendName);

	if (mediaSearch == VideoEncoderList.end())
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

	map<LongLong, CVideoEncoder*>::iterator mediaSearch = VideoEncoderList.begin();

	for (; mediaSearch != VideoEncoderList.end(); ++mediaSearch)
	{
		CVideoEncoder *cVideoEncoder = mediaSearch->second;

//		cVideoEncoder->ResetSocketOfThisVideoEncoder();
	}
}
