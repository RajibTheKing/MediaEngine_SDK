
#include "IPVThread.h"
#include "CommonElementsBucket.h"
#include "VideoCallSession.h"

#if defined(TARGET_OS_IPHONE) || defined(TARGET_IPHONE_SIMULATOR)
#include <dispatch/dispatch.h>
#endif


CIPVThread::CIPVThread(long long llFriendID, CRenderingBuffer *pcRenderingBuffer, CCommonElementsBucket *pcCommonElementsBucket, CVideoCallSession *pcVideoCallSession, bool bIsCheckCall) :

m_pcRenderingBuffer(pcRenderingBuffer),
m_pcCommonElementsBucket(pcCommonElementsBucket),
m_llFriendID(llFriendID),
m_lRenderCallTime(0),
m_bIsCheckCall(bIsCheckCall),
m_nInsetHeight(-1),
m_nInsetWidth(-1)

{
    m_llRenderFrameCounter = 0;
	m_pcVideoCallSession = pcVideoCallSession;
}

CIPVThread::~CIPVThread()
{

}

void CIPVThread::StopThread()
{
	CLogPrinter_WriteLog(CLogPrinter::INFO, THREAD_LOG, "CIPVThread::StopThread() called");

	//if (pInternalThread.get())
	{

		m_bThreadRunning = false;

		while (!m_bThreadClosed)
		{
			m_Tools.SOSleep(5);
		}
	}

	//pInternalThread.reset();

	CLogPrinter_WriteLog(CLogPrinter::INFO, THREAD_LOG, "CIPVThread::StopThread() Rendering Thread STOPPPP");
}

void CIPVThread::StartThread()
{
	CLogPrinter_WriteLog(CLogPrinter::INFO, THREAD_LOG ,"CIPVThread::StartThread() called");

	if (pThreadPointer.get())
	{
		pThreadPointer.reset();
		return;
	}

	m_bThreadRunning = true;
	m_bThreadClosed = false;

#if defined(TARGET_OS_IPHONE) || defined(TARGET_IPHONE_SIMULATOR)

	dispatch_queue_t IPVThreadQ = dispatch_queue_create("IPVThreadQ", DISPATCH_QUEUE_CONCURRENT);
	dispatch_async(IPVThreadQ, ^{
		this->ThreadRunProcedure();
	});

#else

	std::thread myThread(CreateThread, this);
	myThread.detach();

#endif

	CLogPrinter_WriteLog(CLogPrinter::INFO, THREAD_LOG ,"CIPVThread::StartThread() Rendering Thread started");

	return;
}

void *CIPVThread::CreateThread(void* pParam)
{
	CIPVThread *pThis = (CIPVThread*)pParam;
	pThis->ThreadRunProcedure();

	return NULL;
}


void CIPVThread::ThreadRunProcedure()
{
	
}




