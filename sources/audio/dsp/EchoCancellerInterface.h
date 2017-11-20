#ifndef ECHO_CANCELLER_INTERFACE_H
#define ECHO_CANCELLER_INTERFACE_H

#include "AudioMacros.h"

#define AECM_SAMPLES_IN_FRAME 80


namespace MediaSDK
{
	class EchoCancellerInterface
	{

	public:

		virtual int AddFarEndData(short *farEndData, int dataLen) = 0;

		virtual int CancelEcho(short *nearEndData, int dataLen, long long llDelay) = 0;

		virtual ~EchoCancellerInterface() {	}
	};

} //namespace MediaSDK

#endif // !ECHO_CANCELLER_INTERFACE_H
