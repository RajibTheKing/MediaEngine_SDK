#ifndef AUDIO_HEADER_COMMON_H
#define AUDIO_HEADER_COMMON_H


#include "AudioPacketHeader.h"


namespace MediaSDK
{

	class AudioHeaderCommon : public AudioPacketHeader
	{
	public:

		AudioHeaderCommon();
		AudioHeaderCommon(unsigned int * Information);
		AudioHeaderCommon(unsigned char *Header);

		~AudioHeaderCommon();


	private:
		void InitHeaderBitMap();

	};

} //namespace MediaSDK

#endif

