#include "EchoCancellerProvider.h"
#include "WebRTCEchoCanceller.h"
#include "SpeexEchoCanceller.h"


EchoCancellerInterface* EchoCancellerProvider::GetEchoCanceller(EchoCancellerType echoCancellerType)
{
	switch (echoCancellerType)
	{
	case WebRTC:
		return new WebRTCEchoCanceller();

	case Speex:
		return new SpeexEchoCanceller();
	
	default:
		return nullptr;
	}
}