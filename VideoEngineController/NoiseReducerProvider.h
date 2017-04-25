#ifndef NOISE_REDUCER_PROVIDER_H
#define NOISE_REDUCER_PROVIDER_H


#include "AudioResourceTypes.h"
#include "SmartPointer.h"



class NoiseReducerInterface;


class NoiseReducerProvider
{

public:

	static SmartPointer<NoiseReducerInterface> GetNoiseReducer(NoiseReducerType noiseReducerType);

};


#endif  // !NOISE_REDUCER_PROVIDER_H
