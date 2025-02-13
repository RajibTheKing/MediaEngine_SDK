#include "AudioSessionOptions.h"
#include "LogPrinter.h"
#include "InterfaceOfAudioVideoEngine.h"
#include "AudioMacros.h"

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


	AudioEntityRoleType AudioSessionOptions::GetEntityRoleType(int audioFlowType, int entityType)
	{
		if (audioFlowType == AUDIO_FLOW_OPUS_CALL || audioFlowType == AUDIO_FLOW_USELESS_CALL)
		{
			return EntityInCall;
		}
		else if (AUDIO_FLOW_AAC_LIVE_CHANNEL == audioFlowType)
		{
			return EntityChannel;
		}
		else if (AUDIO_FLOW_OPUS_LIVE_CHANNEL == audioFlowType || AUDIO_FLOW_USELESS_STREAM == audioFlowType)
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


	void AudioSessionOptions::SetOptions(int audioFlowType, int entityType)
	{
		AudioEntityRoleType entityRoleType = GetEntityRoleType(audioFlowType, entityType);

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


