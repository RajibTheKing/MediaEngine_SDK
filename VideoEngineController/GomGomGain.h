#ifndef GOMGOM_GAIN_H
#define GOMGOM_GAIN_H

#include "AudioGainInterface.h"
#include "AudioMacros.h"

class Filter;

class GomGomGain : public AudioGainInterface
{

public:
	GomGomGain();

	virtual ~GomGomGain();
	
	int SetGain(int iGain) { return true; };

	int AddFarEnd(short *sInBuf, int nBufferSize) { return true; };

	int AddGain(short *sInBuf, int sBufferSize, bool isLiveStreamRunning);

private:

	Filter *mFilter;

	double m_daMovingAvg[MAX_AUDIO_FRAME_SAMPLE_SIZE];
	unsigned int m_naMultFactor[MAX_AUDIO_FRAME_SAMPLE_SIZE];
	short m_sFilteredFrame[MAX_AUDIO_FRAME_SAMPLE_SIZE];
	short m_sLastFilteredFrame[MAX_AUDIO_FRAME_SAMPLE_SIZE];

	bool b1stFrame;
	int m_nMovingSum;
};

#endif