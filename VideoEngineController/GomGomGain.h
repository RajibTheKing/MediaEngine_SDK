#pragma once


#include "Tools.h"
#include "AudioMacros.h"
#include "filt.h"
#include "AudioGainInterface.h"


class CGomGomGain : public AudioGainInterface
{
private:
	Filter *mFilter;
	double m_daMovingAvg[MAX_AUDIO_FRAME_SAMPLE_SIZE];
	unsigned int m_naMultFactor[MAX_AUDIO_FRAME_SAMPLE_SIZE];
	short m_sFilteredFrame[MAX_AUDIO_FRAME_SAMPLE_SIZE];
	short m_sLastFilteredFrame[MAX_AUDIO_FRAME_SAMPLE_SIZE];
	bool b1stFrame;
	int m_nMovingSum;

public:
	CGomGomGain();

	~CGomGomGain();
	
	int SetGain(int iGain);

	int AddFarEnd(short *sInBuf, int nBufferSize);

	int AddGain(short *sInBuf, int sBufferSize, bool isLiveStreamRunning);
};