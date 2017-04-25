#include "AudioSessionOptions.h"



AudioSessionOptions::AudioSessionOptions()
{
	ResetOptions();
}


void AudioSessionOptions::ResetOptions()
{
	encoderType = Disable_Encoder;
	decoderType = Disable_Decoder;

	noiseReducerType = Disable_ANR;
	echoCancelerType = Disable_ECM;
	gainType = Disable_Gain;

	bufferData = false;
	enableMuxing = false;

	adaptEncoderBitrate = false;
	adaptEncoderComplexity = false;
	adaptDecoderBitrate = false;
	adaptDecoderComplexity = false;

	headerType = -1;
	packetType = -1;
}
