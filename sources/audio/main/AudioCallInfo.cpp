
#include "AudioCallInfo.h"

namespace MediaSDK
{
	CAudioCallInfo::CAudioCallInfo(bool bCameraEnable, bool bMicrophoneEnable) :m_bCameraEnable(bCameraEnable), m_bMicrophoneEnable(bMicrophoneEnable)
	{

	}
	bool CAudioCallInfo::IsCameraEnable()
	{
		return m_bCameraEnable;
	}

	bool CAudioCallInfo::IsMicrophoneEnable()
	{
		return m_bMicrophoneEnable;
	}

	void CAudioCallInfo::SetCameraMode(bool bCameraEnable)
	{
		m_bCameraEnable = bCameraEnable;
	}

	void CAudioCallInfo::SetMicrophoneMode(bool bMicrophoneEnable)
	{
		m_bMicrophoneEnable = bMicrophoneEnable;
	}
}
