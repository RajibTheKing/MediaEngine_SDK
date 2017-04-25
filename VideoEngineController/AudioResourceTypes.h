#ifndef AUDIO_RESOURCE_TYPES
#define AUDIO_RESOURCE_TYPES


enum AudioEncoderType
{
	Opus_Encoder,
	PCM_Encoder,
	Disable_Encoder
};


enum AudioDecoderType
{
	AAC_Decoder,
	Opus_Decoder,
	PCM_Decoder,
	Disable_Decoder
};


enum AudioGainType
{
	WebRTC_AGC,
	GomGomGain_AGC,
	Naive_AGC,
	Disable_Gain
};


enum EchoCancellerType
{
	WebRTC_ECM,
	Speex_ECM,
	Disable_ECM
};


enum NoiseReducerType
{
	WebRTC_ANR,
	Disable_ANR
};


#endif
