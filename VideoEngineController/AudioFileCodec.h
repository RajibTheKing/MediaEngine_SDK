//
// Created by Fahad-PC on 8/29/2016.
//

#ifndef FAHADOPUS_AUDIOFILECODEC_H
#define FAHADOPUS_AUDIOFILECODEC_H

#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <string>
#include <stdlib.h>

#include "opus.h"
#include "Tools.h"
#include "size.h"
#include "LogPrinter.h"

//#define AUDIO_FIXED_BITRATE

#define OPUS_MIN_COMPLEXITY 1
#define OPUS_MAX_COMPLEXITY 10

class CAudioFileCodec
{

public:

    CAudioFileCodec();
    ~CAudioFileCodec();

    int CreateAudioEncoder();

    int decodeAudio(unsigned char *in_data, unsigned int in_size, short *out_buffer);
    int encodeAudio(short *in_data, unsigned int in_size, unsigned char *out_buffer);

    bool SetBitrateOpus(int nBitrate);
    bool SetComplexityOpus(int nComplexity);

private:

    OpusEncoder	*encoder;
    OpusDecoder	*decoder;
    opus_int32	length;
    int 		err;

    opus_int16 in[AUDIO_FRAME_SIZE * AUDIO_CHANNELS];
    opus_int16 out[AUDIO_MAX_FRAME_SIZE * AUDIO_CHANNELS];

    int nbBytes;

    Tools m_Tools;

};



#endif //FAHADOPUS_AUDIOFILECODEC_H
