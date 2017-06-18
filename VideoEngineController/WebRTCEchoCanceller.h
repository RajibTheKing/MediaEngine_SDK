#ifndef WEBRTC_ECHO_CANCELLER_H
#define WEBRTC_ECHO_CANCELLER_H


#include "EchoCancellerInterface.h"
#include "AudioMacros.h"
#include "AudioCallSession.h"

#ifdef USE_AECM
#include "echo_control_mobile.h"
#endif

const int ECHO_LEVEL_CALL = 4;
const int ECHO_LEVEL_LIVE = 2;


namespace MediaSDK
{
	class WebRTCEchoCanceller : public EchoCancellerInterface
	{
	
	public:
		WebRTCEchoCanceller(bool isLiveRunning);
	
		virtual ~WebRTCEchoCanceller();
	
		int AddFarEndData(short *farEndData, int dataLen, bool isLiveStreamRunning);
	
		int CancelEcho(short *nearEndData, int dataLen, bool isLiveStreamRunning, long long llDelay);
	
	private:
		void* AECM_instance;
	
		bool m_bAecmCreated;
		bool m_bAecmInited;
		bool m_bWritingDump;
	
		long long m_llLastFarendTime;
		int iCounter, iCounter2;
	
		short m_sZeroBuf[AECM_SAMPLES_IN_FRAME];
		bool m_bNearEndingOrFarEnding;
	
	};
} //namespace MediaSDK


#endif  // !WEBRTC_ECHO_CANCELLER_H
