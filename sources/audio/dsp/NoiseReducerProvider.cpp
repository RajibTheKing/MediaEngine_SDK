#include "NoiseReducerProvider.h"

#include "WebRTCNoiseReducer.h"

namespace MediaSDK
{

	SharedPointer<NoiseReducerInterface> NoiseReducerProvider::GetNoiseReducer(NoiseReducerType noiseReducerType)
	{
		NoiseReducerInterface* pInstance;
		switch (noiseReducerType)
		{
		case WebRTC_NoiseReducer:
			pInstance = new WebRTCNoiseReducer();
			break;

		default:
			pInstance = nullptr;
		}

		return SharedPointer<NoiseReducerInterface>(pInstance);
	}

} //namespace MediaSDK
