#include "CommonElementsBucket.h"
#include "CommonTypes.h"
#include "LogPrinter.h"
#include "EventNotifier.h"
#include "AudioCallSessionListHandler.h"

namespace MediaSDK
{

	CCommonElementsBucket::CCommonElementsBucket() :
		userName(-1),
		sharedMutex(NULL)

	{
		InstantiateSharedMutex();

		m_pVideoCallSessionList = new CVideoCallSessionListHandler();
		m_pAudioCallSessionList = new CAudioCallSessionListHandler();
		m_pVideoEncoderList = new CVideoEncoderListHandler();

		CLogPrinter_Write(CLogPrinter::DEBUGS, "CCommonElementsBucket::CCommonElementsBucket() common bucket created");
	}

	CCommonElementsBucket::~CCommonElementsBucket()
	{
		if (NULL != m_pVideoCallSessionList)
		{
			delete m_pVideoCallSessionList;
			m_pVideoCallSessionList = NULL;
		}

		if (NULL != m_pAudioCallSessionList)
		{
			delete m_pAudioCallSessionList;
			m_pAudioCallSessionList = NULL;
		}

		if (NULL != m_pVideoEncoderList)
		{
			delete m_pVideoEncoderList;
			m_pVideoEncoderList = NULL;
		}

		if (NULL != sharedMutex)
		{
			delete sharedMutex;
			sharedMutex = NULL;
		}
	}

	void CCommonElementsBucket::InstantiateSharedMutex()
	{
		if (NULL == sharedMutex)
		{
			sharedMutex = new CLockHandler();
		}
	}

	CLockHandler* CCommonElementsBucket::GetSharedMutex()
	{
		return sharedMutex;
	}

	void CCommonElementsBucket::SetUserName(const long long& username)
	{
		userName = username;
	}

	long long CCommonElementsBucket::GetUsername()
	{
		return userName;
	}

} //namespace MediaSDK
