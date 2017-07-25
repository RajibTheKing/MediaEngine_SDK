#ifndef ECHO_CANCELLER_PROVIDER_H
#define ECHO_CANCELLER_PROVIDER_H


#include "AudioTypes.h"
#include "SmartPointer.h"


namespace MediaSDK
{

	class EchoCancellerInterface;

	class EchoCancellerProvider
	{

	public:

		static SmartPointer<EchoCancellerInterface> GetEchoCanceller(EchoCancelerType echoCancellerType, bool isLiveRunning);

	};

} //namespace MediaSDK

#endif // !ECHO_CANCELLER_PROVIDER_H
