#ifndef AUDIO_SESSION_OPTIONS_H 
#define AUDIO_SESSION_OPTIONS_H


#include "AudioResourceTypes.h"


class AudioSessionOptions
{
private:

	AudioHeaderTypes headerType;

	AudioEncoderType encoderType;
	AudioDecoderType decoderType;

	NoiseReducerType noiseReducerType;
	EchoCancellerType echoCancelerType;
	AudioGainType gainType;

	bool adaptEncoderBitrate;
	bool adaptEncoderComplexity;
	bool adaptDecoderBitrate;
	bool adaptDecoderComplexity;

	bool bufferData;
	bool enableMuxing;
	bool packetizeEnable;


protected:

	AudioEntityRoleType GetActionType(int serviceType, int entityType);

	void SetOptionsForCall();
	void SetOptionsForChannel();

	void SetOptionsForPublisher();
	void SetOptionsForPublisherInCall();

	void SetOptionsForViewer();
	void SetOptionsForViewerInCall();


public:

	AudioSessionOptions();
	~AudioSessionOptions() { }

	void ResetOptions();
	void SetOptions(int serviceType, int entityType);
};



#endif  // AUDIO_SESSION_OPTIONS_H
