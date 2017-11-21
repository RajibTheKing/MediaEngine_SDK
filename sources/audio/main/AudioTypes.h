#ifndef AUDIO_TYPES_H
#define AUDIO_TYPES_H

#include <vector>
#include <cstddef>
#include <climits>


namespace MediaSDK
{
	//External Callbacks
	typedef void(*SendFunctionPointerType)(long long, int, unsigned char*, int, int, std::vector< std::pair<int, int> >);

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
		HEADER_CALL,
		HEADER_LIVE
	};

	struct DeviceInformation
	{
		// For Publisher Device Information = 0 index
		// For Callee Device Information = 0 index
		long long llDelay[2], llDelayFraction[2], llStartUpFarEndBufferSize[2], llCurrentFarEndBufferSizeMax[2], llCurrentFarEndBufferSizeMin[2], llAverageTimeDiff[2];
		long long llCallCount, llIsCallInLive;

		long long llLastTime;
		void Reset(int end=2)
		{
			for (int i = 0; i < end; ++i)
			{
				llDelay[i] = SHRT_MAX;
				llDelayFraction[i] = 255;
				llStartUpFarEndBufferSize[i] = SHRT_MIN;
				llCurrentFarEndBufferSizeMax[i] = SHRT_MIN;
				llCurrentFarEndBufferSizeMin[i] = SHRT_MAX;
				llAverageTimeDiff[i] = 0;
			}
			llLastTime = -1;
		}
	};

} //namespace MediaSDK

#endif  // !AUDIO_TYPES_H
