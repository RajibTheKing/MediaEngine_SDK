#ifndef _AUDIO_CALL_INFO_H_
#define _AUDIO_CALL_INFO_H_

namespace MediaSDK
{
	class CAudioCallInfo
	{
	public:
		bool GetCamera();
		bool GetMicrophone();
		void SetCamera(bool CameraEnable);
		void SetMicrophone(bool MicrophoneEnable);
	private:
		bool m_bCameraEnable, m_bMicrophoneEnable;
	};
}

#endif
