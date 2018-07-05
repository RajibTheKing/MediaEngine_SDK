#include "AudioSessionOptions.h"
#include "LogPrinter.h"
#include "InterfaceOfAudioVideoEngine.h"

namespace MediaSDK
{


	AudioSessionOptions::AudioSessionOptions()
	{
		ResetOptions();
	}


	void AudioSessionOptions::ResetOptions()
	{
		headerType = HEADER_COMMON;

		encoderType = No_Encoder;
		decoderType = No_Decoder;

		noiseReducerType = No_NoiseReducer;
		echoCancelerType = No_ECM;
		gainType = No_Gain;

		adaptEncoderBitrate = false;
		adaptEncoderComplexity = false;
		adaptDecoderBitrate = false;
		adaptDecoderComplexity = false;

		enableBufferData = false;
		enableMuxing = false;
		enablePacketization = false;
		isLiveStreamingRunning = false;
	}


	AudioEntityRoleType AudioSessionOptions::GetEntityRoleType(int serviceType, int entityType)
	{
		if (serviceType == SERVICE_TYPE_CALL || serviceType == SERVICE_TYPE_SELF_CALL)
		{
			return EntityInCall;
		}
		else if (AUDIO_FLOW_AAC_LIVE_CHANNEL == serviceType)
		{
			return EntityChannel;
		}
		else if (AUDIO_FLOW_OPUS_LIVE_CHANNEL == serviceType || SERVICE_TYPE_SELF_STREAM == serviceType)
		{
			if (ENTITY_TYPE_PUBLISHER == entityType)
			{
				return EntityPublisher;
			}
			else if (ENTITY_TYPE_PUBLISHER_CALLER == entityType)
			{
				return EntityPublisherInCall;
			}
			else if (ENTITY_TYPE_VIEWER == entityType)
			{
				return EntityViewer;
			}
			else if (ENTITY_TYPE_VIEWER_CALLEE == entityType)
			{
				return EntityViewerInCall;
			}
			else
			{
				return EntityNone;
			}
		}
		else
		{
			return EntityNone;
		}

	}


	void AudioSessionOptions::SetOptions(int serviceType, int entityType)
	{
		AudioEntityRoleType entityRoleType = GetEntityRoleType(serviceType, entityType);

		switch (entityRoleType)
		{
		case EntityInCall:
			SetOptionsForCall();
			break;

		case EntityChannel:
			SetOptionsForChannel();
			break;

		case EntityPublisher:
			SetOptionsForPublisher();
			break;

		case EntityPublisherInCall:
			SetOptionsForPublisherInCall();
			break;

		case EntityViewer:
			SetOptionsForViewer();
			break;

		case EntityViewerInCall:
			SetOptionsForViewerInCall();
			break;

		default:
			ResetOptions();
			break;
		}
	}


	void AudioSessionOptions::SetOptionsForCall()
	{
		MR_DEBUG("#aso# AudioSessionOptions::SetOptionsForCall()");

		headerType = HEADER_COMMON;

		encoderType = Opus_Encoder;
		decoderType = Opus_Decoder;

		noiseReducerType = WebRTC_NoiseReducer;
		echoCancelerType = WebRTC_ECM;
		gainType = WebRTC_Gain;

		adaptEncoderBitrate = false;
		adaptEncoderComplexity = false;
		adaptDecoderBitrate = false;
		adaptDecoderComplexity = false;

		enableBufferData = false;
		enableMuxing = false;
		enablePacketization = false;
		isLiveStreamingRunning = false;
	}


	void AudioSessionOptions::SetOptionsForChannel()
	{
		MR_DEBUG("#aso# AudioSessionOptions::SetOptionsForChannel()");

		headerType = HEADER_COMMON;

		encoderType = No_Encoder;
		decoderType = AAC_Decoder;

		noiseReducerType = No_NoiseReducer;
		echoCancelerType = No_ECM;
		gainType = WebRTC_Gain;

		adaptEncoderBitrate = false;
		adaptEncoderComplexity = false;
		adaptDecoderBitrate = false;
		adaptDecoderComplexity = false;

		enableBufferData = false;
		enableMuxing = false;
		enablePacketization = false;
		isLiveStreamingRunning = false;
	}


	void AudioSessionOptions::SetOptionsForPublisher()
	{
		MR_DEBUG("#aso# AudioSessionOptions::SetOptionsForPublisher()");

		headerType = HEADER_LIVE;

		encoderType = Opus_Encoder;
		decoderType = Opus_Decoder;

		noiseReducerType = WebRTC_NoiseReducer;
		echoCancelerType = WebRTC_ECM;
		gainType = WebRTC_Gain;

		adaptEncoderBitrate = false;
		adaptEncoderComplexity = false;
		adaptDecoderBitrate = false;
		adaptDecoderComplexity = false;

		enableBufferData = false;
		enableMuxing = false;
		enablePacketization = false;
		isLiveStreamingRunning = true;
	}


	void AudioSessionOptions::SetOptionsForPublisherInCall()
	{
		MR_DEBUG("#aso# AudioSessionOptions::SetOptionsForPublisherInCall()");

		headerType = HEADER_LIVE;

		encoderType = PCM_Encoder;
		decoderType = PCM_Decoder;

		noiseReducerType = No_NoiseReducer;
		echoCancelerType = WebRTC_ECM;
		gainType = WebRTC_Gain;

		adaptEncoderBitrate = false;
		adaptEncoderComplexity = false;
		adaptDecoderBitrate = false;
		adaptDecoderComplexity = false;

		enableBufferData = false;
		enableMuxing = true;
		enablePacketization = false;
		isLiveStreamingRunning = true;
	}


	void AudioSessionOptions::SetOptionsForViewer()
	{
		MR_DEBUG("#aso# AudioSessionOptions::SetOptionsForViewer()");

		headerType = HEADER_LIVE;

		encoderType = Opus_Encoder;
		decoderType = Opus_Decoder;

		noiseReducerType = No_NoiseReducer;
		echoCancelerType = WebRTC_ECM;
		gainType = WebRTC_Gain;

		adaptEncoderBitrate = false;
		adaptEncoderComplexity = false;
		adaptDecoderBitrate = false;
		adaptDecoderComplexity = false;

		enableBufferData = false;
		enableMuxing = false;
		enablePacketization = false;
		isLiveStreamingRunning = true;
	}


	void AudioSessionOptions::SetOptionsForViewerInCall()
	{
		MR_DEBUG("#aso# AudioSessionOptions::SetOptionsForViewerInCall()");

		headerType = HEADER_LIVE;

		encoderType = PCM_Encoder;
		decoderType = PCM_Decoder;

		noiseReducerType = No_NoiseReducer;
		echoCancelerType = WebRTC_ECM;
		gainType = WebRTC_Gain;

		adaptEncoderBitrate = false;
		adaptEncoderComplexity = false;
		adaptDecoderBitrate = false;
		adaptDecoderComplexity = false;

		enableBufferData = false;
		enableMuxing = false;
		enablePacketization = false;
		isLiveStreamingRunning = true;
	}

} //namespace MediaSDK


