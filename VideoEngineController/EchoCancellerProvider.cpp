#include "EchoCancellerProvider.h"

#include "WebRTCEchoCanceller.h"
#include "SpeexEchoCanceller.h"


SmartPointer<EchoCancellerInterface> EchoCancellerProvider::GetEchoCanceller(EchoCancelerType echoCancellerType)
{
	EchoCancellerInterface* pInstance = nullptr;

	switch (echoCancellerType)
	{
	case WebRTC_ECM:
		pInstance = new WebRTCEchoCanceller();
		break;

	case Speex_ECM:
		pInstance =  new SpeexEchoCanceller();
		break;
	
	default:
		pInstance = nullptr;
	}

	return SmartPointer<EchoCancellerInterface>(pInstance);
}