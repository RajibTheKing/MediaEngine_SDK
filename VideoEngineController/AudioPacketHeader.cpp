#include "AudioPacketHeader.h"
#include "AudioHeaderCommon.h"

static SmartPointer<AudioPacketHeader> GetInstance(AudioHeaderTypes type)
{
	AudioPacketHeader* pPacketHeader = nullptr;
	switch (type)
	{
	case HeaderCommon:
		pPacketHeader = new AudioHeaderCommon();
		break;
	case HeaderChannel:
	case HeaderVoiceCall:
	default:
		pPacketHeader = nullptr;
		break;
	}

	SmartPointer<AudioPacketHeader> pHeader(pPacketHeader);
	
	return pHeader;
}