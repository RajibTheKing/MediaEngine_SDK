#ifndef WEBRTC_ECHO_CANCELLER_H
#define WEBRTC_ECHO_CANCELLER_H


#include "EchoCancellerInterface.h"
#include "echo_control_mobile.h"

#include "Tools.h"
#include "AudioMacros.h"


//#define ECHO_ANALYSIS

#ifdef ECHO_ANALYSIS
FILE *EchoFile;
#define HEADER_SIZE 1
#define WEBRTC_FAREND 1
#define NEAREND 3
#endif


class WebRTCEchoCanceller : public EchoCancellerInterface
{
private:
	void* AECM_instance;

	bool m_bAecmCreated;
	bool m_bAecmInited;
	bool m_bWritingDump;

	long long m_llLastFarendTime;
	int iCounter, iCounter2;

	short m_sZeroBuf[AECM_SAMPLES_IN_FRAME];
	Tools m_Tools;


public:
	WebRTCEchoCanceller();

	~WebRTCEchoCanceller();

	int AddFarEndData(short *farEndData, int dataLen, bool isLiveStreamRunning);

	int CancelEcho(short *nearEndData, int dataLen, bool isLiveStreamRunning);

};


#endif  // !WEBRTC_ECHO_CANCELLER_H
