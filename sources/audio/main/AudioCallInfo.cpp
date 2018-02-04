
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

	void CAudioCallInfo::SetCameraEnablingMode(bool CameraEnable)
	{
		m_bCameraEnable = CameraEnable;
	}

	void CAudioCallInfo::SetMicrophoneEnablingMode(bool MicrophoneEnable)
	{
		m_bMicrophoneEnable = MicrophoneEnable;
	}
}
