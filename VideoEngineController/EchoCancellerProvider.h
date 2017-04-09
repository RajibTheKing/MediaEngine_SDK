#ifndef ECHO_CANCELLER_PROVIDER_H
#define ECHO_CANCELLER_PROVIDER_H


#include "EchoCancellerInterface.h"


enum EchoCancellerType
{
	WebRTC,
	Speex
};


class EchoCancellerProvider
{
	
public:

	static EchoCancellerInterface* GetEchoCanceller(EchoCancellerType echoCancellerType);

};


#endif // !ECHO_CANCELLER_PROVIDER_H
