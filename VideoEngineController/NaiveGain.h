#ifndef NAIVE_GAIN_H
#define NAIVE_GAIN_H

#include "AudioGainInterface.h"

class NaiveGain : public AudioGainInterface
{

public:

	NaiveGain();

	virtual ~NaiveGain();

	int SetGain(int iGain);

	int AddFarEnd(short *sInBuf, int nBufferSize) { return true; };

	int AddGain(short *sInBuf, int nBufferSize, bool isLiveStreamRunning);

private:
	bool m_bGainEnabled;
	int m_iVolume;

};


#endif  // !NAIVE_GAIN_H
