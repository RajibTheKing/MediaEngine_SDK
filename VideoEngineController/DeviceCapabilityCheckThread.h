
#ifndef _DEVICE_CAPABILITY_CHECK_THREAD_H_
#define _DEVICE_CAPABILITY_CHECK_THREAD_H_

#include "Tools.h"
#include "SmartPointer.h"
#include "LogPrinter.h"
#include "DeviceCapabilityCheckBuffer.h"
#include <thread>

class CController;

class CDeviceCapabilityCheckThread
{
public:

	CDeviceCapabilityCheckThread(CController *pCController, CDeviceCapabilityCheckBuffer *pDeviceCapabilityCheckBuffer);
	~CDeviceCapabilityCheckThread();

	void StartDeviceCapabilityCheckThread();
	void StopDeviceCapabilityCheckThread();
	void DeviceCapabilityCheckThreadProcedure();
	static void *CreateVideoDeviceCapabilityCheckThread(void* param);

private:

	bool bDeviceCapabilityCheckThreadRunning;
	bool bDeviceCapabilityCheckThreadClosed;

	int m_nIdolCounter;

	CDeviceCapabilityCheckBuffer *m_pDeviceCapabilityCheckBuffer;

	CController *m_pCController;

	unsigned char m_EncodedFrame[MAX_VIDEO_ENCODER_FRAME_SIZE];

	Tools m_Tools;

	SmartPointer<std::thread> pDeviceCapabilityCheckThread;
};

#endif 