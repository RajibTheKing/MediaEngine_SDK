#include "EchoCancellerProvider.h"

#include "WebRTCEchoCanceller.h"
#include "SpeexEchoCanceller.h"

namespace MediaSDK
{

	SharedPointer<EchoCancellerInterface> EchoCancellerProvider::GetEchoCanceller(EchoCancelerType echoCancellerType, bool isLiveRunning)
	{
		EchoCancellerInterface* pInstance = nullptr;

		switch (echoCancellerType)
		{
		case WebRTC_ECM:
			pInstance = new WebRTCEchoCanceller(isLiveRunning);
			break;

		case Speex_ECM:
			pInstance = new SpeexEchoCanceller();
			break;

		default:
			pInstance = nullptr;
		}

		return SharedPointer<EchoCancellerInterface>(pInstance);
	}

} //namespace MediaSDK
