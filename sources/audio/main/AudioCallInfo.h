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
		void SetCameraEnablingMode(bool bCameraEnable);
		void SetMicrophoneEnablingMode(bool bMicrophoneEnable);
	private:
		bool m_bCameraEnable, m_bMicrophoneEnable;
	};
}

#endif
