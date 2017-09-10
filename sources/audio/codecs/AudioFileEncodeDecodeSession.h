#ifndef AUDIO_FILE_ENCODE_DECODE_SESSION_H
#define AUDIO_FILE_ENCODE_DECODE_SESSION_H


#define OPUS_ENABLED


namespace MediaSDK
{
	class CAudioFileCodec;
	class G729CodecNative;

	class CAudioFileEncodeDecodeSession
	{

	public:

		CAudioFileEncodeDecodeSession();
		~CAudioFileEncodeDecodeSession();

		int StartAudioEncodeDecodeSession();
		int EncodeAudioFile(short *psaEncodingDataBuffer, int nAudioFrameSize, unsigned char *ucaEncodedDataBuffer);
		int DecodeAudioFile(unsigned char *ucaDecodedDataBuffer, int nAudioFrameSize, short *psaDecodingDataBuffer);
		int StopAudioEncodeDecodeSession();

	private:
#ifdef OPUS_ENABLED
		CAudioFileCodec *m_pAudioCodec;
#else
		G729CodecNative *m_pG729CodecNative;
#endif
	};

} //namespace MediaSDK

#endif
