#include "AudioPacketHeader.h"
#include "AudioHeaderCommon.h"
#include "AudioSessionOptions.h"



namespace MediaSDK
{

	SharedPointer<AudioPacketHeader> AudioPacketHeader::GetInstance(AudioHeaderTypes type)
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

		SharedPointer<AudioPacketHeader> pHeader(pPacketHeader);

		return pHeader;
	}

} //namespace MediaSDK
