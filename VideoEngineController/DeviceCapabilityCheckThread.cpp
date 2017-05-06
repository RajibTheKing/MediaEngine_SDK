
#include "DeviceCapabilityCheckThread.h"
#include "Controller.h"
#include "CommonElementsBucket.h"


#if defined(TARGET_OS_IPHONE) || defined(TARGET_IPHONE_SIMULATOR)
#include <dispatch/dispatch.h>
#endif

CDeviceCapabilityCheckThread::CDeviceCapabilityCheckThread(CController *pCController, CDeviceCapabilityCheckBuffer *pDeviceCapabilityCheckBuffer, CCommonElementsBucket *pCommonElementBucket) :

m_pCController(pCController),
m_pDeviceCapabilityCheckBuffer(pDeviceCapabilityCheckBuffer),
bDeviceCapabilityCheckThreadClosed(true)

{
    m_pCommonElementBucket = pCommonElementBucket;
	m_bThreadAllreadyStarted = false;
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

int CDeviceCapabilityCheckThread::StartDeviceCapabilityCheckThread(int iHeight, int iWidth)
{
	if(m_bThreadAllreadyStarted == true) return -1;
	m_bThreadAllreadyStarted = true;

	CLogPrinter_WriteLog(CLogPrinter::INFO, THREAD_LOG || CHECK_CAPABILITY_LOG, "CDeviceCapabilityCheckThread::StartDeviceCapabilityCheckThread() called");

	m_nIdolCounter = 0;

	for (int k = 0; k<3; k++)
	{
		memset(m_ucaDummmyFrame[k], 0, sizeof(m_ucaDummmyFrame[k]));
		
		for (int i = 0; i< iHeight; i++)
		{
			int color = rand() % 255;
			
			for (int j = 0; j < iWidth; j++)
			{
				m_ucaDummmyFrame[k][i * iHeight + j] = color;
			}
			
		}
	}

	if (pDeviceCapabilityCheckThread.get())
	{
		pDeviceCapabilityCheckThread.reset();

		return -1;
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

	CLogPrinter_WriteLog(CLogPrinter::INFO, THREAD_LOG || CHECK_CAPABILITY_LOG, "CDeviceCapabilityCheckThread::StartDeviceCapabilityCheckThread() DeviceCapabilityCheck Thread started");


	return 1;
}

void *CDeviceCapabilityCheckThread::CreateVideoDeviceCapabilityCheckThread(void* param)
{
	CDeviceCapabilityCheckThread *pThis = (CDeviceCapabilityCheckThread*)param;

	pThis->DeviceCapabilityCheckThreadProcedure();

	return NULL;
}

void CDeviceCapabilityCheckThread::DeviceCapabilityCheckThreadProcedure()
{
	CLogPrinter_WriteLog(CLogPrinter::INFO, THREAD_LOG || CHECK_CAPABILITY_LOG, "CDeviceCapabilityCheckThread::DeviceCapabilityCheckThreadProcedure() started DeviceCapabilityCheck method");

	int nOperation, nVideoHeigth, nVideoWidth, nNotification;
	long long llFriendID;

	while (bDeviceCapabilityCheckThreadRunning)
	{
		//CLogPrinter_WriteLog(CLogPrinter::INFO, THREAD_LOG, "CDeviceCapabilityCheckThread::DeviceCapabilityCheckThreadProcedure() RUNNING DeviceCapabilityCheck method");

		if (m_pDeviceCapabilityCheckBuffer->GetQueueSize() == 0)
		{
			//CLogPrinter_WriteLog(CLogPrinter::INFO, THREAD_LOG || CHECK_CAPABILITY_LOG, "CDeviceCapabilityCheckThread::DeviceCapabilityCheckThreadProcedure() NOTHING for Sending method");

			m_Tools.SOSleep(10);
		}
		else
		{
			nOperation = m_pDeviceCapabilityCheckBuffer->DeQueue(llFriendID, nNotification, nVideoHeigth, nVideoWidth);

			if (nOperation == START_DEVICE_CHECK)
			{
				CLogPrinter_WriteLog(CLogPrinter::INFO, THREAD_LOG || CHECK_CAPABILITY_LOG, "CDeviceCapabilityCheckThread::DeviceCapabilityCheckThreadProcedure() got START_DEVICE_CHECK instruction");

/*
#if defined(TARGET_OS_WINDOWS_PHONE)

				m_pCController->m_nDeviceStrongness = STATUS_UNABLE;
				m_pCController->m_nMemoryEnoughness = STATUS_UNABLE;
				m_pCController->m_nEDVideoSupportablity = STATUS_UNABLE;
				m_pCController->m_nHighFPSVideoSupportablity = STATUS_UNABLE;

#endif
*/
				m_pCController->m_ullTotalDeviceMemory = Tools::GetTotalSystemMemory();

				if (m_pCController->m_ullTotalDeviceMemory >= LEAST_MEMORY_OF_STRONG_DEVICE)
				{
					m_pCController->m_nDeviceStrongness = STATUS_ABLE;
					m_pCController->m_nMemoryEnoughness = STATUS_ABLE;

					CLogPrinter_WriteLog(CLogPrinter::INFO, CHECK_CAPABILITY_LOG, "CDeviceCapabilityCheckThread::DeviceCapabilityCheckThreadProcedure() memory enough");

				}
				else
				{
					m_pCController->m_nDeviceStrongness = STATUS_UNABLE;
					m_pCController->m_nMemoryEnoughness = STATUS_UNABLE;
					m_pCController->m_nEDVideoSupportablity = STATUS_UNABLE;
					m_pCController->m_nHighFPSVideoSupportablity = STATUS_UNABLE;

					CLogPrinter_WriteLog(CLogPrinter::INFO, CHECK_CAPABILITY_LOG, "CDeviceCapabilityCheckThread::DeviceCapabilityCheckThreadProcedure() memory insufficient");
				}

				m_pCController->StartTestAudioCall(llFriendID);

				CVideoCallSession* pVideoSession = m_pCController->StartTestVideoCall(llFriendID, nVideoHeigth, nVideoWidth, 0);
                
                m_Tools.SOSleep(1000);

#ifdef OLD_ENCODING_THREAD

				while (pVideoSession->m_pVideoEncodingThread->IsThreadStarted() == false)
				{
					m_Tools.SOSleep(10);
				}
#else
				while (pVideoSession->m_pVideoEncodingThreadOfCall->IsThreadStarted() == false)
				{
					m_Tools.SOSleep(10);
				}
#endif
                
#if defined(SOUL_SELF_DEVICE_CHECK)

				int numberOfFrames = HIGH_FRAME_RATE * 5;
                long long llCurrentTimestamp = m_Tools.CurrentTimestamp();
                int factor = 1;
				
				long long lastFramePushTime = m_Tools.CurrentTimestamp();
				/*
				CAverageCalculator pdAvg;
				pdAvg.Reset();
				*/

				CLogPrinter_WriteLog(CLogPrinter::INFO, CHECK_CAPABILITY_LOG, "CDeviceCapabilityCheckThread::DeviceCapabilityCheckThreadProcedure() pushing sample data");

				for (int i = 0; i < numberOfFrames; i++)
				{
					CLogPrinter_WriteLog(CLogPrinter::INFO, CHECK_CAPABILITY_LOG, "CDeviceCapabilityCheckThread::DeviceCapabilityCheckThreadProcedure() pushed sample data " + m_Tools.getText(i));

					long long now = m_Tools.CurrentTimestamp();

#ifdef OLD_ENCODING_THREAD

                    int nowQueueSize = pVideoSession->m_pVideoEncodingThread->m_pEncodingBuffer->GetQueueSize();
#else
					int nowQueueSize = pVideoSession->m_pVideoEncodingThreadOfCall->m_pEncodingBuffer->GetQueueSize();
#endif
                    if(nowQueueSize == MAX_VIDEO_ENCODER_BUFFER_SIZE)
                    {
                        m_Tools.SOSleep(1);
                        i--;
                        continue;
                    }

#ifdef OLD_ENCODING_THREAD

					pVideoSession->m_pVideoEncodingThread->m_pEncodingBuffer->Queue(m_ucaDummmyFrame[i % 3], nVideoWidth * nVideoHeigth * 3 / 2, nVideoHeigth, nVideoWidth, now, 0);
#else
					pVideoSession->m_pVideoEncodingThreadOfCall->m_pEncodingBuffer->Queue(m_ucaDummmyFrame[i % 3], nVideoWidth * nVideoHeigth * 3 / 2, nVideoHeigth, nVideoWidth, now, 0);
#endif
			
					/*
					printFile("Push Difference = %lld\n", now - lastFramePushTime);
					if (i)
					{
						pdAvg.UpdateData(now - lastFramePushTime);
						printFile("pdAvg = %lf\n", pdAvg.GetAverage());
					}
					lastFramePushTime = now;
					*/
                    m_Tools.SOSleep(factor);
				}
			
				CLogPrinter_WriteLog(CLogPrinter::INFO, CHECK_CAPABILITY_LOG, "CDeviceCapabilityCheckThread::DeviceCapabilityCheckThreadProcedure() pushed sample data");
#endif

			}
			else if (nOperation == STOP_DEVICE_CHECK)
			{
                //printf("Samaun--> STOP_DEVICE_CHECK, iVideoWidth,iVideoHeight = %d,%d ....... Notification = %d\n", nVideoWidth, nVideoHeigth, nNotification);

				CLogPrinter_WriteLog(CLogPrinter::INFO, CHECK_CAPABILITY_LOG, "CDeviceCapabilityCheckThread::DeviceCapabilityCheckThreadProcedure() got STOP_DEVICE_CHECK instruction");

				m_pCController->StopTestAudioCall(llFriendID);
				m_pCController->StopTestVideoCall(llFriendID);

				CLogPrinter_WriteLog(CLogPrinter::INFO, CHECK_CAPABILITY_LOG, "CDeviceCapabilityCheckThread::DeviceCapabilityCheckThreadProcedure() audio video processing stopped");

                if(nNotification == DEVICE_CHECK_SUCCESS && (nVideoHeigth * nVideoWidth == 640 * 480))
                {
					CLogPrinter_WriteLog(CLogPrinter::INFO, CHECK_CAPABILITY_LOG, "CDeviceCapabilityCheckThread::DeviceCapabilityCheckThreadProcedure() DEVICE_CHECK_SUCCESS && 640 * 480");

                    m_pCController->m_nSupportedResolutionFPSLevel = SUPPORTED_RESOLUTION_FPS_640_25;
                    
                    m_pCommonElementBucket->m_pEventNotifier->fireVideoNotificationEvent(llFriendID, m_pCommonElementBucket->m_pEventNotifier->SET_CAMERA_RESOLUTION_640x480_25FPS);
                }
				else if (nNotification == DEVICE_CHECK_FAILED && (nVideoHeigth * nVideoWidth == 640 * 480))
                {
					CLogPrinter_WriteLog(CLogPrinter::INFO, CHECK_CAPABILITY_LOG, "CDeviceCapabilityCheckThread::DeviceCapabilityCheckThreadProcedure() DEVICE_CHECK_FAILED && 640 * 480");

#if defined(SOUL_SELF_DEVICE_CHECK)
						
					m_pDeviceCapabilityCheckBuffer->Queue(llFriendID, START_DEVICE_CHECK, DEVICE_CHECK_STARTING, 352, 288);

#else

                    m_pCommonElementBucket->m_pEventNotifier->fireVideoNotificationEvent(llFriendID, m_pCommonElementBucket->m_pEventNotifier->SET_CAMERA_RESOLUTION_640x480_25FPS_NOT_SUPPORTED);

#endif

                }
                else if(nNotification == DEVICE_CHECK_SUCCESS && (nVideoHeigth * nVideoWidth < 640 * 480))
                {
					CLogPrinter_WriteLog(CLogPrinter::INFO, CHECK_CAPABILITY_LOG, "CDeviceCapabilityCheckThread::DeviceCapabilityCheckThreadProcedure() DEVICE_CHECK_SUCCESS && < 640 * 480");

                    m_pCController->m_nSupportedResolutionFPSLevel = SUPPORTED_RESOLUTION_FPS_352_25;
                    
                    m_pCommonElementBucket->m_pEventNotifier->fireVideoNotificationEvent(llFriendID, m_pCommonElementBucket->m_pEventNotifier->SET_CAMERA_RESOLUTION_352x288_25FPS);
                }
                else if(nNotification == DEVICE_CHECK_FAILED && (nVideoHeigth * nVideoWidth < 640 * 480))
                {
					CLogPrinter_WriteLog(CLogPrinter::INFO, CHECK_CAPABILITY_LOG, "CDeviceCapabilityCheckThread::DeviceCapabilityCheckThreadProcedure() DEVICE_CHECK_FAILED && < 640 * 480");

                    m_pCController->m_nSupportedResolutionFPSLevel = SUPPORTED_RESOLUTION_FPS_352_15;
                    
                    m_pCommonElementBucket->m_pEventNotifier->fireVideoNotificationEvent(llFriendID, m_pCommonElementBucket->m_pEventNotifier->SET_CAMERA_RESOLUTION_352x288_25FPS_NOT_SUPPORTED);
                }
                       
                if((nVideoHeigth * nVideoWidth < 640 * 480) || (nVideoHeigth * nVideoWidth == 640 * 480 && nNotification == DEVICE_CHECK_SUCCESS))
                {
					if (nVideoHeigth * nVideoWidth < 640 * 480)
						CLogPrinter_WriteLog(CLogPrinter::INFO, CHECK_CAPABILITY_LOG, "CDeviceCapabilityCheckThread::DeviceCapabilityCheckThreadProcedure() stopping device check < 640 * 480");
					else
						CLogPrinter_WriteLog(CLogPrinter::INFO, CHECK_CAPABILITY_LOG, "CDeviceCapabilityCheckThread::DeviceCapabilityCheckThreadProcedure() stopping device check DEVICE_CHECK_SUCCESS && 640 * 480");

                    bDeviceCapabilityCheckThreadRunning = false;    
                }
                
			}

			m_Tools.SOSleep(1);
		}
	}

	bDeviceCapabilityCheckThreadClosed = true;
	m_bThreadAllreadyStarted = false;

	CLogPrinter_WriteLog(CLogPrinter::INFO, THREAD_LOG || CHECK_CAPABILITY_LOG, "CDeviceCapabilityCheckThread::DeviceCapabilityCheckThreadProcedure() stopped DeviceCapabilityCheckThreadProcedure method.");
}