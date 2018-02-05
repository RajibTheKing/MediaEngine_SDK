#ifndef _AUDIO_CALL_INFO_H_
#define _AUDIO_CALL_INFO_H_

namespace MediaSDK
{
	class CAudioCallInfo
	{
	public:
		CAudioCallInfo(bool bCameraEnable, bool bMicrophoneEnable);
		~CAudioCallInfo();
		//Getter
		bool IsCameraEnable();
		bool IsMicrophoneEnable();
		//Setter
		void SetCameraMode(bool bCameraEnable);
		void SetMicrophoneMode(bool bMicrophoneEnable);
	private:
		bool m_bCameraEnable, m_bMicrophoneEnable;
	};
}

#endif
