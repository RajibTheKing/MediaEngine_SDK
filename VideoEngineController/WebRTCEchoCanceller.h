#ifndef WEBRTC_ECHO_CANCELLER_H
#define WEBRTC_ECHO_CANCELLER_H


#include "EchoCancellerInterface.h"


class WebRTCEchoCanceller : public EchoCancellerInterface
{

public:

	int AddFarEndData(short *farEndData, int dataLen);

	int CancelEchoFromNearEndData(short *nearEndData, int dataLen);

};


#endif  // !WEBRTC_ECHO_CANCELLER_H
