
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

	void CAudioCallInfo::SetCamera(bool CameraEnable)
	{
		m_bCameraEnable = CameraEnable;
	}

	void CAudioCallInfo::SetMicrophone(bool MicrophoneEnable)
	{
		m_bMicrophoneEnable = MicrophoneEnable;
	}
}
