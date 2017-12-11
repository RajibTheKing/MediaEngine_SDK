#ifndef WEBRTC_NOISE_REDUCER_H
#define WEBRTC_NOISE_REDUCER_H

#include "NoiseReducerInterface.h"
#include "AudioMacros.h"

#ifdef USE_ANS
#include "noise_suppression_x.h"
#endif

namespace MediaSDK
{

	class WebRTCNoiseReducer : public NoiseReducerInterface
	{
#ifdef USE_ANS
		NsxHandle* NS_instance = 0;
		short m_tmpbuffer[1600];
#endif

	public:

		WebRTCNoiseReducer();
		~WebRTCNoiseReducer();

		void Init();
		void Release();
		void Reset();

		int Denoise(short *sInBuf, int sBufferSize, short * sOutBuf, bool isLiveStreamRunning);
	};

} //namespace MediaSDK

#endif // !WEBRTC_NOISE_REDUCER_H

