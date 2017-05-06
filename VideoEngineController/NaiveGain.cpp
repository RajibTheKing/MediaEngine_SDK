#include "NaiveGain.h"

#include "AudioMacros.h"
#include "LogPrinter.h"

#ifndef ALOG
#define ALOG(a) CLogPrinter_WriteSpecific6(CLogPrinter::INFO, "ALOG:" + a);
#endif

NaiveGain::NaiveGain() : m_bGainEnabled(false)
{
	m_iVolume = DEFAULT_GAIN;
}


NaiveGain::~NaiveGain()
{

}

int NaiveGain::SetGain(int iGain)
{
	if (iGain < 0)
	{
		m_bGainEnabled = false;
		return false;
	}
	else if (iGain >= 0)
	{
		m_bGainEnabled = true;
	}

	if (iGain >= 0 && iGain <= MAX_GAIN)
	{
		m_iVolume = iGain;
		ALOG("SetVolume called with: " + Tools::IntegertoStringConvert(iVolume));
	}
	else
	{
		m_iVolume = DEFAULT_GAIN;
	}

	return true;
}

int NaiveGain::AddGain(short *sInBuf, int nBufferSize, bool isLiveStreamRunning)
{
	if (!m_bGainEnabled)
	{
		return false;
	}

	for (int i = 0; i < CURRENT_AUDIO_FRAME_SAMPLE_SIZE(isLiveStreamRunning); i++)
	{
		int temp = (int)sInBuf[i] * m_iVolume;
		if (temp > SHRT_MAX)
		{
			temp = SHRT_MAX;
		}
		if (temp < SHRT_MIN)
		{
			temp = SHRT_MIN;
		}
		sInBuf[i] = temp;
	}

	return true;
}