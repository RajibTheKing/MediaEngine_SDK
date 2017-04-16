#ifndef WEBRTC_NOISE_REDUCER_H
#define WEBRTC_NOISE_REDUCER_H

#include "noise_suppression.h"
#include "Tools.h"


class WebRTCNoiseReducer
{
	NsHandle* NS_instance;
	Tools m_Tools;

public:
	
	WebRTCNoiseReducer();
	~WebRTCNoiseReducer();
	
	int Denoise(short *sInBuf, int sBufferSize, short * sOutBuf, bool isLiveStreamRunning);
};


#endif // !WEBRTC_NOISE_REDUCER_H
