#ifndef NOISE_REDUCER_INTERFACE_H
#define NOISE_REDUCER_INTERFACE_H


class NoiseReducerInterface
{
public:

	virtual int Denoise(short *sInBuf, int sBufferSize, short * sOutBuf, bool isLiveStreamRunning) = 0;

	virtual	~NoiseReducerInterface(){ }
};

#endif // !NOISE_REDUCER_INTERFACE_H


// NoiseReducerInterface --> 

