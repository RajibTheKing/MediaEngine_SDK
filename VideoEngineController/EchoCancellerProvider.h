#ifndef ECHO_CANCELLER_PROVIDER_H
#define ECHO_CANCELLER_PROVIDER_H


#include "AudioTypes.h"
#include "SmartPointer.h"



class EchoCancellerInterface;

class EchoCancellerProvider
{
	
public:

	static SmartPointer<EchoCancellerInterface> GetEchoCanceller(EchoCancelerType echoCancellerType);

};


#endif // !ECHO_CANCELLER_PROVIDER_H
