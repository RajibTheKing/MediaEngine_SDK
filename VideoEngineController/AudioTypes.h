#ifndef AUDIO_TYPES_H
#define AUDIO_TYPES_H

#include "AudioVideoEngineDefinitions.h"
#include <vector>

//External Callbacks
typedef void(*SendFunctionPointerType)(LongLong, int, unsigned char*, int, int, std::vector< std::pair<int, int> >);

//Internal Callbacks
typedef void(*OnDataReadyToSendCB)(int mediaType, unsigned char* data, size_t dataLength);
typedef void(*OnFirePacketEventCB)(int eventType, size_t dataLenth, unsigned char data[]);
typedef void(*OnFireDataEventCB)(int eventType, size_t dataLenth, short data[]);
typedef void(*OnFireNetworkChangeCB)(int eventType);
typedef void(*OnFireAudioAlarmCB)(int eventType);

typedef void(*OnPackatizedDataReadyCallback)(unsigned char*, int);
typedef void(*OnDepackatizedDataReadyCallback)(unsigned char*, int);

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
