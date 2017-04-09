#ifndef SPEEX_ECHO_CANCELLER_H
#define SPEEX_ECHO_CANCELLER_H


#include "EchoCancellerInterface.h"


class SpeexEchoCanceller : public EchoCancellerInterface
{

public:

	int AddFarEndData(short *farEndData, int dataLen);
	
	int CancelEchoFromNearEndData(short *nearEndData, int dataLen);

};


#endif  // !SPEEX_ECHO_CANCELLER_H