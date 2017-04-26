#include "AudioSessionOptions.h"
#include "InterfaceOfAudioVideoEngine.h"



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


AudioEntityActionType AudioSessionOptions::GetActionType(int serviceType, int entityType)
{
	if (serviceType == SERVICE_TYPE_CALL || serviceType == SERVICE_TYPE_SELF_CALL)
	{
		return EntityInCall;
	}
	else if (SERVICE_TYPE_CHANNEL == serviceType)
	{
		return EntityChannel;
	}
	else if (SERVICE_TYPE_LIVE_STREAM == serviceType || SERVICE_TYPE_SELF_STREAM == serviceType)
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
	AudioEntityActionType actionType = GetActionType(serviceType, entityType);

	switch (actionType)
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

}


void AudioSessionOptions::SetOptionsForChannel()
{

}

void AudioSessionOptions::SetOptionsForPublisher()
{

}


void AudioSessionOptions::SetOptionsForPublisherInCall()
{

}


void AudioSessionOptions::SetOptionsForViewer()
{

}


void AudioSessionOptions::SetOptionsForViewerInCall()
{

}



