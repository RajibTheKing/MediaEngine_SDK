#ifndef ECHO_CANCELLER_PROVIDER_H
#define ECHO_CANCELLER_PROVIDER_H

#include "SmartPointer.h"

class EchoCancellerInterface;

enum EchoCancellerType
{
	WebRTC_ECM,
	Speex_ECM
};


class EchoCancellerProvider
{
	
public:

	static SmartPointer<EchoCancellerInterface> GetEchoCanceller(EchoCancellerType echoCancellerType);

};


#endif // !ECHO_CANCELLER_PROVIDER_H
