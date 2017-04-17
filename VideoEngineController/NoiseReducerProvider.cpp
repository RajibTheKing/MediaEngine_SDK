#include "NoiseReducerProvider.h"
#include "WebRTCNoiseReducer.h"


NoiseReducerInterface* NoiseReducerProvider::GetNoiseReducer(NoiseReducerType noiseReducerType)
{
	switch (noiseReducerType)
	{
	case WebRTC_NR:
		return new WebRTCNoiseReducer();

	default:
		return nullptr;
	}
}


