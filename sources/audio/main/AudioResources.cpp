#include "AudioResources.h"
#include "AudioEncoderProvider.h"
#include "AudioDecoderProvider.h"
#include "EchoCancellerProvider.h"
#include "AudioGainInstanceProvider.h"
#include "NoiseReducerProvider.h"
#include "AudioSessionOptions.h"
#include "AudioPacketHeader.h"


namespace MediaSDK
{
	AudioResources::AudioResources(AudioSessionOptions audioSessionOptions)
	{
		m_pAudioNearEndHeader = AudioPacketHeader::GetInstance(audioSessionOptions.GetHeaderType());
		m_pAudioFarEndHeader = AudioPacketHeader::GetInstance(audioSessionOptions.GetHeaderType());

		m_pAudioEncoder = AudioEncoderProvider::GetAudioEncoder(audioSessionOptions.GetEncoderType());
		m_pAudioDecoder = AudioDecoderProvider::GetAudioDecoder(audioSessionOptions.GetDecoderType());


		m_pEchoCanceler = EchoCancellerProvider::GetEchoCanceller(audioSessionOptions.GetEchoCancelerType(), audioSessionOptions.GetIsLiveStreamRunning());
		m_pNoiseReducer = NoiseReducerProvider::GetNoiseReducer(audioSessionOptions.GetNoiseReducerType());

		m_pPlayerGain = AudioGainInstanceProvider::GetAudioGainInstance(audioSessionOptions.GetGainType());
		m_pRecorderGain = AudioGainInstanceProvider::GetAudioGainInstance(audioSessionOptions.GetGainType());
	}

} //namespace MediaSDK
