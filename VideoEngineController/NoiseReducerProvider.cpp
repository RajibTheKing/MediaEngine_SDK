#include "NoiseReducerProvider.h"
#include "WebRTCNoiseReducer.h"

#include "SmartPointer.h"

SmartPointer<NoiseReducerInterface> NoiseReducerProvider::GetNoiseReducer(NoiseReducerType noiseReducerType)
{
	NoiseReducerInterface* pInstance;
	switch (noiseReducerType)
	{
	case WebRTC_ANR:
		pInstance = new WebRTCNoiseReducer();
		break;

	default:
		pInstance = nullptr;
	}

	return SmartPointer<NoiseReducerInterface>(pInstance);
}


