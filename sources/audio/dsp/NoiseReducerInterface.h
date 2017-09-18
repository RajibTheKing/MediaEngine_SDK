#ifndef NOISE_REDUCER_INTERFACE_H
#define NOISE_REDUCER_INTERFACE_H


namespace MediaSDK
{
	class NoiseReducerInterface
	{
	public:

		virtual int Denoise(short *sInBuf, int sBufferSize, short * sOutBuf, bool isLiveStreamRunning) = 0;

		virtual	~NoiseReducerInterface() { }
	};


} //namespace MediaSDK

#endif // !NOISE_REDUCER_INTERFACE_H


// NoiseReducerInterface --> WebRTCNoiseReducer --> NoiseReducerProvider

