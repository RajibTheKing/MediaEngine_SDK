#include "NoiseReducerProvider.h"

#include "WebRTCNoiseReducer.h"

SmartPointer<NoiseReducerInterface> NoiseReducerProvider::GetNoiseReducer(NoiseReducerType noiseReducerType)
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

	return SmartPointer<NoiseReducerInterface>(pInstance);
}


