#define OPUS_MIN_COMPLEXITY 1
#define OPUS_MAX_COMPLEXITY 10
#define OpusEncoder int
#define OpusDecoder int
#define opus_int32 int
#define opus_int16 short
#define opus_encoder_create(...) 0
#define opus_decoder_create(...) 0
#define opus_encoder_destroy(...) 0
#define opus_decoder_destroy(...) 0
//#define opus_encode(...) 0
#define opus_decode(...) 0
#define opus_encoder_ctl(...) 0
#define OPUS_EXPORT
# define OPUS_ARG_NONNULL(_x)
# define OPUS_WARN_UNUSED_RESULT

OPUS_EXPORT OPUS_WARN_UNUSED_RESULT opus_int32 opus_encode(
        OpusEncoder *st,
        const opus_int16 *pcm,
        int frame_size,
        unsigned char *data,
        opus_int32 max_data_bytes
) OPUS_ARG_NONNULL(1) OPUS_ARG_NONNULL(2) OPUS_ARG_NONNULL(4);