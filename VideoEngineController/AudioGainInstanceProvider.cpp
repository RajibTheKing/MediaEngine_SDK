#include "AudioGainInstanceProvider.h"
#include "WebRTCGain.h"
#include "GomGomGain.h"
#include "NaiveGain.h"
#include "LogPrinter.h"

AudioGainInterface* AudioGainInstanceProvider::GetAudioGainInstance(AudioGainType audioGainType)
{
	switch (audioGainType)
	{
	case WebRTC:
		LOG_AAC("#gain# new instance webRTC");
		return new WebRTCGain();

	case GomGomGain:
		return new CGomGomGain();

	case Naive:
		return new NaiveGain();

	default:
		return nullptr;
	}
}