#include "EncoderPCM.h"
#include <cstring>

int EncoderPCM::EncodeAudio(short *in_data, unsigned int in_size, unsigned char *out_buffer)
{
	memcpy(out_buffer, in_data, in_size * sizeof(short));

	return (in_size * sizeof(short));
}
