#include "CommonElementsBucket.h"
#include "LockHandler.h"
#include "LogPrinter.h"

CCommonElementsBucket::CCommonElementsBucket() :
userName(-1),
m_nPacketSizeOfNetwork(-1),
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

void CCommonElementsBucket::SetPacketSizeOfNetwork(int packetSizeOfNetwork)
{
	m_nPacketSizeOfNetwork = packetSizeOfNetwork;
}

int CCommonElementsBucket::GetPacketSizeOfNetwork()
{
	return m_nPacketSizeOfNetwork;
}

CLockHandler* CCommonElementsBucket::GetSharedMutex()
{
    return sharedMutex;
}

void CCommonElementsBucket::SetUserName(const LongLong& username)
{
    userName = username;
}

LongLong CCommonElementsBucket::GetUsername()
{
    return userName;
}

void CCommonElementsBucket::SetSendFunctionPointer(void(*callBackFunctionPointer)(LongLong, int, unsigned char*, int, int, std::vector< std::pair<int, int> > vAudioBlocks))
{
    
    SendFunctionPointer = callBackFunctionPointer;
    
}

