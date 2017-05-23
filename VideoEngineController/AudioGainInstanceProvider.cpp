#include "AudioGainInstanceProvider.h"

#include "WebRTCGain.h"
#include "GomGomGain.h"
#include "NaiveGain.h"

namespace MediaSDK
{

	SmartPointer<AudioGainInterface> AudioGainInstanceProvider::GetAudioGainInstance(AudioGainType audioGainType)
	{
		AudioGainInterface* pInstance = nullptr;
		switch (audioGainType)
		{
		case WebRTC_Gain:
			pInstance = new WebRTCGain();
			break;

		case GomGom_Gain:
			pInstance = new GomGomGain();
			break;

		case Naive_Gain:
			pInstance = new NaiveGain();
			break;

		default:
			//Do nothing;
			break;
		}

		return SmartPointer<AudioGainInterface>(pInstance);
	}

} // namespace MediaSDK
