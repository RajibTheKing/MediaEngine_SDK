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


struct AudioResources
{
	SmartPointer<AudioPacketHeader> m_pAudioHeader;

	SmartPointer<AudioEncoderInterface> m_pAudioEncoder;
	SmartPointer<AudioDecoderInterface> m_pAudioDecoder;

	SmartPointer<EchoCancellerInterface> m_pEchoCanceler;
	SmartPointer<NoiseReducerInterface> m_pNoiseReducer;

	SmartPointer<AudioGainInterface> m_pRecorderGain;
	SmartPointer<AudioGainInterface> m_pPlayerGain;

	AudioResources(AudioSessionOptions audioSessionOptions);
};


#endif  // !AUDIO_RESOURCES_H
