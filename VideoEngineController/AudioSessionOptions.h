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
	EchoCancelerType echoCancelerType;
	AudioGainType gainType;

	bool adaptEncoderBitrate;
	bool adaptEncoderComplexity;
	bool adaptDecoderBitrate;
	bool adaptDecoderComplexity;

	bool enableBufferData;
	bool enableMuxing;
	bool enablePacketization;


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


public:

	AudioHeaderTypes GetHeaderType()       { return headerType; }

	AudioEncoderType GetEncoderType()      { return encoderType; }
	AudioDecoderType GetDecoderType()      { return decoderType; }

	NoiseReducerType GetNoiseReducerType() { return noiseReducerType; }
	EchoCancelerType GetEchoCancelerType() { return echoCancelerType; }
	AudioGainType GetGainType()            { return gainType; }

	bool IsAdaptEncoderBitrate()           { return adaptEncoderBitrate; }
	bool IsAdaptEncoderComplexity()        { return adaptEncoderComplexity; }
	bool IsAdaptDecoderBitrate()           { return adaptDecoderBitrate; }
	bool IsAdaptDecoderComplexity()        { return adaptDecoderComplexity; }

	bool IsEnabledBufferData()             { return enableBufferData; }
	bool IsEnabledMuxing()                 { return enableMuxing; }
	bool IsEnabledPacketization()          { return enablePacketization; }
};



#endif  // AUDIO_SESSION_OPTIONS_H
