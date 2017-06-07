#ifndef AUDIO_TYPES_H
#define AUDIO_TYPES_H

#include "AudioVideoEngineDefinitions.h"
#include <vector>
#include <cstddef>

namespace MediaSDK
{
	//External Callbacks
	typedef void(*SendFunctionPointerType)(LongLong, int, unsigned char*, int, int, std::vector< std::pair<int, int> >);

	//Internal Callback Interfaces.
	//TODO: Should go on different header
	class DataReadyListenerInterface
	{
	public:
		virtual void OnDataReadyToSend(int mediaType, unsigned char* data, size_t dataLength) = 0;
	};

	class PacketEventListener
	{
	public:
		virtual void FirePacketEvent(int eventType, size_t dataLenth, unsigned char data[]) = 0;
	};
	
	class DataEventListener
	{
	public:
		virtual void FireDataEvent(int eventType, size_t dataLenth, short data[]) = 0;
	};
	
	class NetworkChangeListener
	{
	public:
		virtual void FireNetworkChange(int eventType) = 0;
	};
	
	class AudioAlarmListener
	{
	public: 
		virtual void FireAudioAlarm(int eventType) = 0;
	};

	class PackatizedDataListener
	{
	public:
		virtual void SendPackatizedData(unsigned char*, int) = 0;
	};

	class DepackatizedDataListener
	{
	public:
		virtual void SendDepackatizedData(unsigned char*, int) = 0;
	};

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

} //namespace MediaSDK

#endif  // !AUDIO_TYPES_H
