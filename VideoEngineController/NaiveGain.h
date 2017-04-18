#ifndef NAIVE_GAIN_H
#define NAIVE_GAIN_H

#include "AudioGainInterface.h"


class NaiveGain : public AudioGainInterface
{
	bool m_bGainEnabled;
	int m_iVolume;


public:

	NaiveGain();

	~NaiveGain();

	int SetGain(int iGain);

	int AddFarEnd(short *sInBuf, int nBufferSize);

	int AddGain(short *sInBuf, int nBufferSize, bool isLiveStreamRunning);

};


#endif  // !NAIVE_GAIN_H
