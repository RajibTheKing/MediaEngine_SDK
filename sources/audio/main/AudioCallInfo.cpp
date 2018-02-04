
#include "AudioCallInfo.h"

namespace MediaSDK
{
	bool CAudioCallInfo::GetCamera()
	{
		return m_bCameraEnable;
	}

	bool CAudioCallInfo::GetMicrophone()
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
