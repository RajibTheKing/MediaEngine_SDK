#ifndef WEBRTC_NOISE_REDUCER_H
#define WEBRTC_NOISE_REDUCER_H

#include "NoiseReducerInterface.h"

#ifdef USE_ANS
#include "noise_suppression.h"
#endif

namespace MediaSDK
{

	class WebRTCNoiseReducer : public NoiseReducerInterface
	{
#ifdef USE_ANS
		NsHandle* NS_instance;
#endif

	public:

		WebRTCNoiseReducer();
		~WebRTCNoiseReducer();

		int Denoise(short *sInBuf, int sBufferSize, short * sOutBuf, bool isLiveStreamRunning);
	};

} //namespace MediaSDK

#endif // !WEBRTC_NOISE_REDUCER_H

