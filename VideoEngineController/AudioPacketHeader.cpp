#include "AudioPacketHeader.h"
#include "AudioHeaderCommon.h"

static SmartPointer<AudioPacketHeader> GetInstance(AudioHeaderTypes type)
{
	AudioPacketHeader* pPacketHeader = nullptr;
	switch (type)
	{
	case HEADER_COMMON:
		pPacketHeader = new AudioHeaderCommon();
		break;
	case HEADER_CHANNEL:
	case HEADER_CALL:
	default:
		pPacketHeader = nullptr;
		break;
	}

	SmartPointer<AudioPacketHeader> pHeader(pPacketHeader);
	
	return pHeader;
}