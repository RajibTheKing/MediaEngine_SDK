#ifndef AUDIO_AAC_DECODER_H
#define AUDIO_AAC_DECODER_H


#include "AudioDecoderInterface.h"
#include "aacdecoder_lib.h"



namespace MediaSDK
{
	class DecoderAAC : public AudioDecoderInterface
	{

	private:

		int m_nRC;
		HANDLE_AACDECODER m_hDecoder;


	protected:

		short ByteArrayToShortBE(unsigned char *byteArray);

		int ThreeBytesIntoIntBE(unsigned char *byteArray);

		void CreateConfBuf(int sampleRate, int numOfChannels, unsigned char *conf);


	public:

		DecoderAAC();

		~DecoderAAC();

		bool SetParameters(int sampleRate, int numberOfChannels);

		int DecodeAudio(unsigned char *inputDataBuffer, unsigned int inputDataSize, short *outputDataBuffer);

	};

} //namespace MediaSDK

#endif  // !AUDIO_AAC_DECODER_H


