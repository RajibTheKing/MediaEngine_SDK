#include "AudioGainInstanceProvider.h"
#include "WebRTCGain.h"
#include "GomGomGain.h"
#include "NaiveGain.h"

SmartPointer<AudioGainInterface> AudioGainInstanceProvider::GetAudioGainInstance(AudioGainType audioGainType)
{
	AudioGainInterface* pInstance = nullptr;
	switch (audioGainType)
	{
	case WebRTC_AGC:

		pInstance = new WebRTCGain();
		break;

	case GomGomGain_AGC:
		pInstance = new GomGomGain();
		break;

	case Naive_AGC:
		pInstance = new NaiveGain();
		break;

	default:
		//Do nothing;
		break;
	}

	return SmartPointer<AudioGainInterface>(pInstance);
}