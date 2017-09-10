
#ifndef IPV_DEVICE_CAPABILITY_CHECK_THREAD_H
#define IPV_DEVICE_CAPABILITY_CHECK_THREAD_H

#include "Tools.h"
#include "SmartPointer.h"
#include "LogPrinter.h"
#include "DeviceCapabilityCheckBuffer.h"
#include <thread>

namespace MediaSDK
{

	class CController;
	class CCommonElementsBucket;

	class CDeviceCapabilityCheckThread
	{
	public:

		CDeviceCapabilityCheckThread(CController *pCController, CDeviceCapabilityCheckBuffer *pDeviceCapabilityCheckBuffer, CCommonElementsBucket *pCommonElementBucket);
		~CDeviceCapabilityCheckThread();

		int StartDeviceCapabilityCheckThread(int iHeight, int iWidth);
		void StopDeviceCapabilityCheckThread();
		void DeviceCapabilityCheckThreadProcedure();
		static void *CreateVideoDeviceCapabilityCheckThread(void* param);

	private:

		bool bDeviceCapabilityCheckThreadRunning;
		bool bDeviceCapabilityCheckThreadClosed;

		unsigned char m_ucaDummmyFrame[3][MAX_VIDEO_ENCODER_FRAME_SIZE];

		int m_nIdolCounter;

		CDeviceCapabilityCheckBuffer *m_pDeviceCapabilityCheckBuffer;

		CController *m_pCController;

		unsigned char m_EncodedFrame[MAX_VIDEO_ENCODER_FRAME_SIZE];

		Tools m_Tools;
		CCommonElementsBucket *m_pCommonElementBucket;
		bool m_bThreadAllreadyStarted;

		SharedPointer<std::thread> pDeviceCapabilityCheckThread;
	};

} //namespace MediaSDK

#endif
