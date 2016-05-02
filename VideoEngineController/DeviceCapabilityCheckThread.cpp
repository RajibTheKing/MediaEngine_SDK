
#include "DeviceCapabilityCheckThread.h"
#include "Controller.h"

#if defined(TARGET_OS_IPHONE) || defined(TARGET_IPHONE_SIMULATOR)
#include <dispatch/dispatch.h>
#endif

CDeviceCapabilityCheckThread::CDeviceCapabilityCheckThread(CController *pCController, CDeviceCapabilityCheckBuffer *pDeviceCapabilityCheckBuffer) :

m_pCController(pCController),
m_pDeviceCapabilityCheckBuffer(pDeviceCapabilityCheckBuffer)

{

}

CDeviceCapabilityCheckThread::~CDeviceCapabilityCheckThread()
{

}

void CDeviceCapabilityCheckThread::StopDeviceCapabilityCheckThread()
{
	//if (pInternalThread.get())
	{
		bDeviceCapabilityCheckThreadRunning = false;

		while (!bDeviceCapabilityCheckThreadClosed)
			m_Tools.SOSleep(5);
	}

	//pInternalThread.reset();
}

void CDeviceCapabilityCheckThread::StartDeviceCapabilityCheckThread()
{
	CLogPrinter_WriteLog(CLogPrinter::INFO, THREAD_LOG, "CDeviceCapabilityCheckThread::StartDeviceCapabilityCheckThread() called");

	m_nIdolCounter = 0;

	if (pDeviceCapabilityCheckThread.get())
	{
		pDeviceCapabilityCheckThread.reset();

		return;
	}

	bDeviceCapabilityCheckThreadRunning = true;
	bDeviceCapabilityCheckThreadClosed = false;

#if defined(TARGET_OS_IPHONE) || defined(TARGET_IPHONE_SIMULATOR)

	dispatch_queue_t DeviceCapabilityCheckThreadQ = dispatch_queue_create("DeviceCapabilityCheckThreadQ", DISPATCH_QUEUE_CONCURRENT);
	dispatch_async(DeviceCapabilityCheckThreadQ, ^{
		this->DeviceCapabilityCheckThreadProcedure();
	});

#else

	std::thread myThread(CreateVideoDeviceCapabilityCheckThread, this);
	myThread.detach();

#endif

	CLogPrinter_WriteLog(CLogPrinter::INFO, THREAD_LOG, "CDeviceCapabilityCheckThread::StartDeviceCapabilityCheckThread() DeviceCapabilityCheck Thread started");

	return;
}

void *CDeviceCapabilityCheckThread::CreateVideoDeviceCapabilityCheckThread(void* param)
{
	CDeviceCapabilityCheckThread *pThis = (CDeviceCapabilityCheckThread*)param;

	pThis->DeviceCapabilityCheckThreadProcedure();

	return NULL;
}

void CDeviceCapabilityCheckThread::DeviceCapabilityCheckThreadProcedure()
{
	CLogPrinter_WriteLog(CLogPrinter::INFO, THREAD_LOG, "CDeviceCapabilityCheckThread::DeviceCapabilityCheckThreadProcedure() started DeviceCapabilityCheck method");

	int nOperation, nVideoHeigth, nVideoWidth;
	long long llFriendID;

	while (bDeviceCapabilityCheckThreadRunning)
	{
		CLogPrinter_WriteLog(CLogPrinter::INFO, THREAD_LOG, "CDeviceCapabilityCheckThread::DeviceCapabilityCheckThreadProcedure() RUNNING DeviceCapabilityCheck method");

		if (m_pDeviceCapabilityCheckBuffer->GetQueueSize() == 0)
		{
			CLogPrinter_WriteLog(CLogPrinter::INFO, THREAD_LOG, "CDeviceCapabilityCheckThread::DeviceCapabilityCheckThreadProcedure() NOTHING for Sending method");

			m_nIdolCounter++;

			if (m_nIdolCounter == 5)
				bDeviceCapabilityCheckThreadRunning = false;

			m_Tools.SOSleep(10);
		}
		else
		{
			nOperation = m_pDeviceCapabilityCheckBuffer->DeQueue(llFriendID, nVideoHeigth, nVideoWidth);

			if (nOperation == START_DEVICE_CHECK)
			{

#if defined(TARGET_OS_WINDOWS_PHONE)

				m_pCController->m_nDeviceStrongness = STATUS_UNABLE;
				m_pCController->m_nMemoryEnoughness = STATUS_UNABLE;
				m_pCController->m_nEDVideoSupportablity = STATUS_UNABLE;
				m_pCController->m_nHighFPSVideoSupportablity = STATUS_UNABLE;

#endif

				m_pCController->m_ullTotalDeviceMemory = Tools::GetTotalSystemMemory();

				if (m_pCController->m_ullTotalDeviceMemory >= LEAST_MEMORY_OF_STRONG_DEVICE)
				{
					m_pCController->m_nDeviceStrongness = STATUS_ABLE;
					m_pCController->m_nMemoryEnoughness = STATUS_ABLE;
				}
				else
				{
					m_pCController->m_nDeviceStrongness = STATUS_UNABLE;
					m_pCController->m_nMemoryEnoughness = STATUS_UNABLE;
					m_pCController->m_nEDVideoSupportablity = STATUS_UNABLE;
					m_pCController->m_nHighFPSVideoSupportablity = STATUS_UNABLE;
				}

				m_pCController->StartTestAudioCall(llFriendID);
				m_pCController->StartTestVideoCall(llFriendID, nVideoHeigth, nVideoWidth, 0);
			}
			else if (nOperation == STOP_DEVICE_CHECK)
			{
				m_pCController->StopTestAudioCall(llFriendID);
				m_pCController->StopTestVideoCall(llFriendID);
			}

			m_Tools.SOSleep(1);
		}
	}

	bDeviceCapabilityCheckThreadClosed = true;

	CLogPrinter_WriteLog(CLogPrinter::INFO, THREAD_LOG, "CDeviceCapabilityCheckThread::DeviceCapabilityCheckThreadProcedure() stopped DeviceCapabilityCheckThreadProcedure method.");
}