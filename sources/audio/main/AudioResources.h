#ifndef AUDIO_RESOURCES_H
#define AUDIO_RESOURCES_H


#include "SmartPointer.h"


namespace MediaSDK
{
	class AudioPacketHeader;
	class AudioEncoderInterface;
	class AudioDecoderInterface;
	class EchoCancellerInterface;
	class NoiseReducerInterface;
	class AudioGainInterface;
	class AudioSessionOptions;


	class AudioResources
	{
	private:

		SharedPointer<AudioPacketHeader> m_pAudioNearEndHeader;
		SharedPointer<AudioPacketHeader> m_pAudioFarEndHeader;

		SharedPointer<AudioEncoderInterface> m_pAudioEncoder;
		SharedPointer<AudioDecoderInterface> m_pAudioDecoder;

		SharedPointer<EchoCancellerInterface> m_pEchoCanceler;
		SharedPointer<NoiseReducerInterface> m_pNoiseReducer;

		SharedPointer<AudioGainInterface> m_pPlayerGain;
		SharedPointer<AudioGainInterface> m_pRecorderGain;


	public:

		AudioResources(AudioSessionOptions audioSessionOptions);
		~AudioResources() { }

		SharedPointer<AudioPacketHeader> GetNearEndPacketHeader() { return m_pAudioNearEndHeader; }
		SharedPointer<AudioPacketHeader> GetFarEndPacketHeader() { return m_pAudioFarEndHeader; }

		SharedPointer<AudioEncoderInterface> GetEncoder()  { return m_pAudioEncoder; }
		SharedPointer<AudioDecoderInterface> GetDecoder()  { return m_pAudioDecoder; }

		SharedPointer<EchoCancellerInterface> GetEchoCanceler() { return m_pEchoCanceler; }
		SharedPointer<NoiseReducerInterface> GetNoiseReducer()  { return m_pNoiseReducer; }

		SharedPointer<AudioGainInterface> GetPlayerGain()   { return m_pPlayerGain; }
		SharedPointer<AudioGainInterface> GetRecorderGain() { return m_pRecorderGain; }
	};

} //namespace MediaSDK

#endif  // !AUDIO_RESOURCES_H
