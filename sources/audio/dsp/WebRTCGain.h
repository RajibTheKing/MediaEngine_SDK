#ifndef WEBRTC_GAIN_H
#define WEBRTC_GAIN_H

#include "AudioGainInterface.h"

#define WEBRTC_AGC_MIN_LEVEL 1
#define WEBRTC_AGC_MAX_LEVEL 255


namespace MediaSDK
{
	enum AGCMode
	{
		MODE_UNCHANGED = 0,
		MODE_ADAPTIVE_ANALOG = 1,
		MODE_ADAPTIVE_DIGITAL = 2,
		MODE_FIXED_DIGITAL = 3
	};

	class WebRTCGain : public AudioGainInterface
	{
	private:
		bool m_bGainEnabled;
		//short *m_sTempBuf = nullptr;
		int m_iVolume;
		int m_iSkipFrames;
		int m_iSampleSize = -1;
		int m_iServiceType = -1;

		void* AGC_instance;


	public:
		WebRTCGain();

		void Init(int serviceType);
		virtual ~WebRTCGain();

		bool SetGain(int iGain);

		bool AddFarEnd(short *sInBuf, int nBufferSize);

		bool AddGain(short *sInBuf, int nBufferSize, bool bPlayerSide, int nEchoStateFlags);

	};

} //namespace MediaSDK

#endif  // !WEBRTC_GAIN_H
