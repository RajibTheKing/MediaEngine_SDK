#ifndef NOISE_REDUCER_PROVIDER_H
#define NOISE_REDUCER_PROVIDER_H


#include "NoiseReducerInterface.h"


enum NoiseReducerType
{
	WebRTC_NR
};



class NoiseReducerProvider
{

public:

	static NoiseReducerInterface* GetNoiseReducer(NoiseReducerType noiseReducerType);

};


#endif  // !NOISE_REDUCER_PROVIDER_H
