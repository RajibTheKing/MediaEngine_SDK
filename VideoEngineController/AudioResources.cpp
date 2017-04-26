#include "AudioResources.h"
#include "AudioEncoderProvider.h"
#include "AudioDecoderProvider.h"
#include "EchoCancellerProvider.h"
#include "AudioGainInstanceProvider.h"
#include "NoiseReducerProvider.h"



AudioResources::AudioResources(AudioSessionOptions audioSessionOptions)
{
	m_pAudioHeader = AudioPacketHeader::GetInstance(audioSessionOptions.GetHeaderType());
	
	m_pAudioEncoder = AudioEncoderProvider::GetAudioEncoder(audioSessionOptions.GetEncoderType());
	m_pAudioDecoder = AudioDecoderProvider::GetAudioDecoder(audioSessionOptions.GetDecoderType());

	m_pEchoCanceler = EchoCancellerProvider::GetEchoCanceller(audioSessionOptions.GetEchoCancelerType());
	m_pNoiseReducer = NoiseReducerProvider::GetNoiseReducer(audioSessionOptions.GetNoiseReducerType());

	m_pRecorderGain = AudioGainInstanceProvider::GetAudioGainInstance(audioSessionOptions.GetGainType());
	m_pPlayerGain = AudioGainInstanceProvider::GetAudioGainInstance(audioSessionOptions.GetGainType());

}
