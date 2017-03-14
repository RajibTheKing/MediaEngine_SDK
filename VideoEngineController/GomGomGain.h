#pragma once


#include "Tools.h"
#include "AudioMacros.h"
#include "filt.h"


class CGomGomGain
{
private:
	int m_ID;
	Filter *mFilter;
	double m_daMovingAvg[MAX_AUDIO_FRAME_SAMPLE_SIZE];
	unsigned int m_naMultFactor[MAX_AUDIO_FRAME_SAMPLE_SIZE];
	short m_sFilteredFrame[MAX_AUDIO_FRAME_SAMPLE_SIZE];
	short m_sLastFilteredFrame[MAX_AUDIO_FRAME_SAMPLE_SIZE];
	bool b1stFrame;
	int m_nMovingSum;
public:
	CGomGomGain(int id);
	~CGomGomGain();
	int AddGain(short *sInBuf, int sBufferSize);
};