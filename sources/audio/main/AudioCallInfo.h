#ifndef _AUDIO_CALL_INFO_H_
#define _AUDIO_CALL_INFO_H_

namespace MediaSDK
{
	class CAudioCallInfo
	{
	public:
		//Getter
		bool IsCameraEnable();
		bool IsMicrophoneEnable();
		//Setter
		void SetCameraEnablingMode(bool CameraEnable);
		void SetMicrophoneEnablingMode(bool MicrophoneEnable);
	private:
		bool m_bCameraEnable, m_bMicrophoneEnable;
	};
}

#endif
