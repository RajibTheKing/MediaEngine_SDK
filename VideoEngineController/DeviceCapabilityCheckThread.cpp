
#include "DeviceCapabilityCheckThread.h"
#include "Controller.h"
#include "CommonElementsBucket.h"


#if defined(TARGET_OS_IPHONE) || defined(TARGET_IPHONE_SIMULATOR)
#include <dispatch/dispatch.h>
#endif

CDeviceCapabilityCheckThread::CDeviceCapabilityCheckThread(CController *pCController, CDeviceCapabilityCheckBuffer *pDeviceCapabilityCheckBuffer, CCommonElementsBucket *pCommonElementBucket) :

m_pCController(pCController),
m_pDeviceCapabilityCheckBuffer(pDeviceCapabilityCheckBuffer)

{
    m_pCommonElementBucket = pCommonElementBucket;
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

	int nOperation, nVideoHeigth, nVideoWidth, nNotification;
	long long llFriendID;

	while (bDeviceCapabilityCheckThreadRunning)
	{
		CLogPrinter_WriteLog(CLogPrinter::INFO, THREAD_LOG, "CDeviceCapabilityCheckThread::DeviceCapabilityCheckThreadProcedure() RUNNING DeviceCapabilityCheck method");

		if (m_pDeviceCapabilityCheckBuffer->GetQueueSize() == 0)
		{
			CLogPrinter_WriteLog(CLogPrinter::INFO, THREAD_LOG, "CDeviceCapabilityCheckThread::DeviceCapabilityCheckThreadProcedure() NOTHING for Sending method");

			m_Tools.SOSleep(10);
		}
		else
		{
			nOperation = m_pDeviceCapabilityCheckBuffer->DeQueue(llFriendID, nNotification, nVideoHeigth, nVideoWidth);

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
                printf("Samaun--> STOP_DEVICE_CHECK, iVideoWidth,iVideoHeight = %d,%d ....... Notification = %d\n", nVideoWidth, nVideoHeigth, nNotification);
				m_pCController->StopTestAudioCall(llFriendID);
				m_pCController->StopTestVideoCall(llFriendID);

#ifdef __ANDROID__
				if(nNotification == DEVICE_CHECK_SUCCESS && nVideoWidth == 480)
				{
					m_pCController->m_nSupportedResolutionFPSLevel = SUPPORTED_RESOLUTION_FPS_640_25;

					m_pCommonElementBucket->m_pEventNotifier->fireVideoNotificationEvent(llFriendID, m_pCommonElementBucket->m_pEventNotifier->SET_CAMERA_RESOLUTION_640x480_25FPS);
				}
				else if(nNotification == DEVICE_CHECK_FAILED && nVideoWidth == 480)
				{
					m_pCommonElementBucket->m_pEventNotifier->fireVideoNotificationEvent(llFriendID, m_pCommonElementBucket->m_pEventNotifier->SET_CAMERA_RESOLUTION_640x480_25FPS_NOT_SUPPORTED);
				}
				else if(nNotification == DEVICE_CHECK_SUCCESS && nVideoWidth<480)
				{
					m_pCController->m_nSupportedResolutionFPSLevel = SUPPORTED_RESOLUTION_FPS_352_25;

					m_pCommonElementBucket->m_pEventNotifier->fireVideoNotificationEvent(llFriendID, m_pCommonElementBucket->m_pEventNotifier->SET_CAMERA_RESOLUTION_352x288_25FPS);
				}
				else if(nNotification == DEVICE_CHECK_FAILED && nVideoWidth<480)
				{
					m_pCController->m_nSupportedResolutionFPSLevel = SUPPORTED_RESOLUTION_FPS_352_15;

					m_pCommonElementBucket->m_pEventNotifier->fireVideoNotificationEvent(llFriendID, m_pCommonElementBucket->m_pEventNotifier->SET_CAMERA_RESOLUTION_352x288_25FPS_NOT_SUPPORTED);
				}



				if((nVideoWidth < 480) || (nVideoWidth == 480 && nNotification == DEVICE_CHECK_SUCCESS))
				{
					bDeviceCapabilityCheckThreadRunning = false;

				}

#else
                
                if(nNotification == DEVICE_CHECK_SUCCESS && nVideoWidth == 640)
                {
					m_pCController->m_nSupportedResolutionFPSLevel = SUPPORTED_RESOLUTION_FPS_640_25;

                    m_pCommonElementBucket->m_pEventNotifier->fireVideoNotificationEvent(llFriendID, m_pCommonElementBucket->m_pEventNotifier->SET_CAMERA_RESOLUTION_640x480_25FPS);
                }
                else if(nNotification == DEVICE_CHECK_FAILED && nVideoWidth == 640)
                {
                    m_pCommonElementBucket->m_pEventNotifier->fireVideoNotificationEvent(llFriendID, m_pCommonElementBucket->m_pEventNotifier->SET_CAMERA_RESOLUTION_640x480_25FPS_NOT_SUPPORTED);
                }
                else if(nNotification == DEVICE_CHECK_SUCCESS && nVideoWidth<640)
                {
					m_pCController->m_nSupportedResolutionFPSLevel = SUPPORTED_RESOLUTION_FPS_352_25;

                    m_pCommonElementBucket->m_pEventNotifier->fireVideoNotificationEvent(llFriendID, m_pCommonElementBucket->m_pEventNotifier->SET_CAMERA_RESOLUTION_352x288_25FPS);
                }
                else if(nNotification == DEVICE_CHECK_FAILED && nVideoWidth<640)
                {
					m_pCController->m_nSupportedResolutionFPSLevel = SUPPORTED_RESOLUTION_FPS_352_15;

                    m_pCommonElementBucket->m_pEventNotifier->fireVideoNotificationEvent(llFriendID, m_pCommonElementBucket->m_pEventNotifier->SET_CAMERA_RESOLUTION_352x288_25FPS_NOT_SUPPORTED);
                }
                
                
                
                if((nVideoWidth < 640) || (nVideoWidth == 640 && nNotification == DEVICE_CHECK_SUCCESS))
                {
                    bDeviceCapabilityCheckThreadRunning = false;
                    
                }
                
#endif
                
                
			}

			m_Tools.SOSleep(1);
		}
	}

	bDeviceCapabilityCheckThreadClosed = true;

	CLogPrinter_WriteLog(CLogPrinter::INFO, THREAD_LOG, "CDeviceCapabilityCheckThread::DeviceCapabilityCheckThreadProcedure() stopped DeviceCapabilityCheckThreadProcedure method.");
}