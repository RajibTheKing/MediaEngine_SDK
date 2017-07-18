#ifndef NOISE_REDUCER_PROVIDER_H
#define NOISE_REDUCER_PROVIDER_H


#include "AudioTypes.h"
#include "SmartPointer.h"


namespace MediaSDK
{

	class NoiseReducerInterface;

	class NoiseReducerProvider
	{

	public:

		static SmartPointer<NoiseReducerInterface> GetNoiseReducer(NoiseReducerType noiseReducerType);

	};

} //namespace MediaSDK

#endif  // !NOISE_REDUCER_PROVIDER_H
