#ifndef AUDIO_TYPES_H
#define AUDIO_TYPES_H


enum AudioEntityRoleType
{
	EntityInCall,
	EntityChannel,

	EntityPublisher,
	EntityPublisherInCall,

	EntityViewer,
	EntityViewerInCall, 

	EntityNone
};


enum AudioEncoderType
{
	Opus_Encoder,
	PCM_Encoder,
	No_Encoder
};


enum AudioDecoderType
{
	AAC_Decoder,
	Opus_Decoder,
	PCM_Decoder,
	No_Decoder
};


enum AudioGainType
{
	WebRTC_Gain,
	GomGom_Gain,
	Naive_Gain,
	No_Gain
};


enum EchoCancelerType
{
	WebRTC_ECM,
	Speex_ECM,
	No_ECM
};


enum NoiseReducerType
{
	WebRTC_NoiseReducer,
	No_NoiseReducer
};


enum AudioHeaderTypes
{
	HEADER_COMMON,
	HEADER_CHANNEL,
	HEADER_CALL
};


#endif  // !AUDIO_TYPES_H
