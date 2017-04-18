#include "AudioNoEncoder.h"
#include <cstring>

int AudioNoEncoder::encodeAudio(short *in_data, unsigned int in_size, unsigned char *out_buffer)
{
	memcpy(out_buffer, in_data, in_size * sizeof(short));
}
