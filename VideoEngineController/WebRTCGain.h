#ifndef WEBRTC_GAIN_H
#define WEBRTC_GAIN_H

#include "AudioGainInterface.h"
#include "gain_control.h"
#include "signal_processing_library.h"
#include "Tools.h"


#define AGC_SAMPLES_IN_FRAME 80
#define AGC_ANALYSIS_SAMPLES_IN_FRAME 80
#define AGCMODE_UNCHANGED 0
#define AGCMODE_ADAPTIVE_ANALOG 1
#define AGNMODE_ADAPTIVE_DIGITAL 2
#define AGCMODE_FIXED_DIGITAL 3
#define MINLEVEL 1
#define MAXLEVEL 255


class WebRTCGain : public AudioGainInterface
{
	bool m_bGainEnabled;
	short *m_sTempBuf;
	int m_iVolume;

	void* AGC_instance;
	Tools m_Tools;

public:

	WebRTCGain();

	~WebRTCGain();

	int SetGain(int iGain);

	int AddFarEnd(short *sInBuf, int nBufferSize);

	int AddGain(short *sInBuf, int nBufferSize, bool isLiveStreamRunning);

};


#endif  // !WEBRTC_GAIN_H
