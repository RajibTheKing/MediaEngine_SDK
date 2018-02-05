
#include "AudioCallInfo.h"

namespace MediaSDK
{
	bool CAudioCallInfo::IsCameraEnable()
	{
		return m_bCameraEnable;
	}

	bool CAudioCallInfo::IsMicrophoneEnable()
	{
		return m_bMicrophoneEnable;
	}

	void CAudioCallInfo::SetCameraEnablingMode(bool bCameraEnable)
	{
		m_bCameraEnable = bCameraEnable;
	}

	void CAudioCallInfo::SetMicrophoneEnablingMode(bool bMicrophoneEnable)
	{
		m_bMicrophoneEnable = bMicrophoneEnable;
	}
}
