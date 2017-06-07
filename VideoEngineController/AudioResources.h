#ifndef AUDIO_RESOURCES_H
#define AUDIO_RESOURCES_H

#include "SmartPointer.h"
#include "AudioPacketHeader.h"
#include "AudioEncoderInterface.h"
#include "AudioDecoderInterface.h"
#include "EchoCancellerInterface.h"
#include "AudioGainInterface.h"
#include "NoiseReducerInterface.h"
#include "AudioSessionOptions.h"

namespace MediaSDK
{

	class AudioResources
	{
	private:

		SmartPointer<AudioPacketHeader> m_pAudioNearEndHeader;
		SmartPointer<AudioPacketHeader> m_pAudioFarEndHeader;

		SmartPointer<AudioEncoderInterface> m_pAudioEncoder;
		SmartPointer<AudioDecoderInterface> m_pAudioDecoder;

		SmartPointer<EchoCancellerInterface> m_pEchoCanceler;
		SmartPointer<NoiseReducerInterface> m_pNoiseReducer;

		SmartPointer<AudioGainInterface> m_pPlayerGain;


	public:

		AudioResources(AudioSessionOptions audioSessionOptions);
		~AudioResources() { }

		SmartPointer<AudioPacketHeader> GetNearEndPacketHeader() { return m_pAudioNearEndHeader; }
		SmartPointer<AudioPacketHeader> GetFarEndPacketHeader() { return m_pAudioFarEndHeader; }

		SmartPointer<AudioEncoderInterface> GetEncoder()  { return m_pAudioEncoder; }
		SmartPointer<AudioDecoderInterface> GetDecoder()  { return m_pAudioDecoder; }

		SmartPointer<EchoCancellerInterface> GetEchoCanceler() { return m_pEchoCanceler; }
		SmartPointer<NoiseReducerInterface> GetNoiseReducer()  { return m_pNoiseReducer; }

		SmartPointer<AudioGainInterface> GetPlayerGain()   { return m_pPlayerGain; }
	};

} //namespace MediaSDK

#endif  // !AUDIO_RESOURCES_H
