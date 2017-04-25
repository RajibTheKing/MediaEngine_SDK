#ifndef AUDIO_SESSION_OPTIONS_H 
#define AUDIO_SESSION_OPTIONS_H


#include "AudioResourceTypes.h"


struct AudioSessionOptions
{
private:
	AudioEncoderType encoderType;
	AudioDecoderType decoderType;

	NoiseReducerType noiseReducerType;
	EchoCancellerType echoCancelerType;
	AudioGainType gainType;

	bool bufferData;
	bool enableMuxing;

	bool adaptEncoderBitrate;
	bool adaptEncoderComplexity;
	bool adaptDecoderBitrate;
	bool adaptDecoderComplexity;

	int headerType;
	int packetType;

public:

	AudioSessionOptions();
	void ResetOptions();

	void SetOptions(AudioEntityActionType entityActionType);
};



#endif  // AUDIO_SESSION_OPTIONS_H
