#include "DecoderPCM.h"
#include <cstring>

namespace MediaSDK
{

	int DecoderPCM::DecodeAudio(unsigned char *in_data, unsigned int in_size, short *out_buffer)
	{
		memcpy(out_buffer, in_data, in_size);

		return (in_size / sizeof(short));
	}

} //namespace MediaSDK
