#include "NaiveGain.h"

#include "AudioMacros.h"
#include "MediaLogger.h"


namespace MediaSDK
{

	NaiveGain::NaiveGain() : m_bGainEnabled(false)
	{
		m_iVolume = DEFAULT_GAIN;
	}


	NaiveGain::~NaiveGain()
	{

	}

	bool NaiveGain::SetGain(int iGain)
	{
#ifdef USE_AGC
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
			//MediaLog(LOG_DEBUG, "SetVolume called with: %d",iVolume);
		}
		else
		{
			m_iVolume = DEFAULT_GAIN;
		}
#endif

		return true;
	}

	bool NaiveGain::AddGain(short *sInBuf, int nBufferSize, bool bPlayerSide, int nEchoStateFlags)
	{
#ifdef USE_AGC
		if (!m_bGainEnabled)
		{
			return false;
		}

		for (int i = 0; i < nBufferSize; i++)
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
#endif

		return true;
	}

} //namespace MediaSDK
