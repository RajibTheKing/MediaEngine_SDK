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


class AudioResources
{
private:

	SmartPointer<AudioPacketHeader> m_pAudioHeader;

	SmartPointer<AudioEncoderInterface> m_pAudioEncoder;
	SmartPointer<AudioDecoderInterface> m_pAudioDecoder;

	SmartPointer<EchoCancellerInterface> m_pEchoCanceler;
	SmartPointer<NoiseReducerInterface> m_pNoiseReducer;

	SmartPointer<AudioGainInterface> m_pRecorderGain;
	SmartPointer<AudioGainInterface> m_pPlayerGain;


public:

	AudioResources(AudioSessionOptions audioSessionOptions);
	~AudioResources() { }

	SmartPointer<AudioPacketHeader> GetPacketHeader() { return m_pAudioHeader; }

	SmartPointer<AudioEncoderInterface> GetEncoder()  { return m_pAudioEncoder; }
	SmartPointer<AudioDecoderInterface> GetDecoder()  { return m_pAudioDecoder; }

	SmartPointer<EchoCancellerInterface> GetEchoCanceler() { return m_pEchoCanceler; }
	SmartPointer<NoiseReducerInterface> GetNoiseReducer()  { return m_pNoiseReducer; }

	SmartPointer<AudioGainInterface> GetRecorderGain() { return m_pRecorderGain; }
	SmartPointer<AudioGainInterface> GetPlayerGain()   { return m_pPlayerGain; }
};


#endif  // !AUDIO_RESOURCES_H
