#ifndef NAIVE_GAIN_H
#define NAIVE_GAIN_H

#include "AudioGainInterface.h"


namespace MediaSDK
{

	class NaiveGain : public AudioGainInterface
	{

	public:

		NaiveGain();

		virtual ~NaiveGain();

		void Init(int serviceType){};

		bool SetGain(int iGain);

		bool AddFarEnd(short *sInBuf, int nBufferSize) { return true; };

		bool AddGain(short *sInBuf, int nBufferSize, bool isLiveStreamRunning);

	private:
		bool m_bGainEnabled;
		int m_iVolume;

	};

} //namespace MediaSDK

#endif  // !NAIVE_GAIN_H
