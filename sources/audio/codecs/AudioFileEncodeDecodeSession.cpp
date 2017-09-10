#include "AudioFileEncodeDecodeSession.h"

#ifdef OPUS_ENABLED
#include "AudioFileCodec.h"
#else
#include "G729CodecNative.h"
#endif


namespace MediaSDK
{
	//#define __CAudioFileEncodeDecodeSession_DUMP_FILE__

#ifdef __CAudioFileEncodeDecodeSession_DUMP_FILE__
	FILE *FileOutput;
	FILE *Fileinput;
#endif

	CAudioFileEncodeDecodeSession::CAudioFileEncodeDecodeSession()
	{
#ifdef __CAudioFileEncodeDecodeSession_DUMP_FILE__
		//FileOutput = fopen("/sdcard/OutputPCMN.pcm", "a+");
		//Fileinput = fopen("/sdcard/FahadInputPCMN.pcm", "a+");
		FileOutput = fopen("/storage/emulated/0/FahadO.pcm", "w");;
		//	Fileinput = fopen("/storage/emulated/0/FahadInputPCMN.pcm", "w");;
#endif

#ifdef OPUS_ENABLED
		m_pAudioCodec = nullptr;
#else
		m_pG729CodecNative = nullptr;
#endif
	}

	CAudioFileEncodeDecodeSession::~CAudioFileEncodeDecodeSession()
	{

#ifdef __CAudioFileEncodeDecodeSession_DUMP_FILE__
		fclose(FileOutput);
		//fclose(Fileinput);
#endif

#ifdef OPUS_ENABLED
		if (nullptr != this->m_pAudioCodec)
		{
			delete this->m_pAudioCodec;

			this->m_pAudioCodec = nullptr;
		}
#else
		if (nullptr != m_pG729CodecNative)
		{
			delete m_pG729CodecNative;

			m_pG729CodecNative = nullptr;
		}
#endif
	}

	int CAudioFileEncodeDecodeSession::StartAudioEncodeDecodeSession()
	{


#ifdef OPUS_ENABLED
		if (nullptr == this->m_pAudioCodec)
		{
			this->m_pAudioCodec = new CAudioFileCodec();

		}
		return m_pAudioCodec->CreateAudioEncoder();
#else
		if (nullptr == m_pG729CodecNative)
		{
			m_pG729CodecNative = new G729CodecNative();
		}

		return m_pG729CodecNative->Open();
#endif

	}

	int CAudioFileEncodeDecodeSession::EncodeAudioFile(short *psaEncodingDataBuffer, int nAudioFrameSize, unsigned char *ucaEncodedDataBuffer)
	{
#ifdef OPUS_ENABLED
#ifdef __CAudioFileEncodeDecodeSession_DUMP_FILE__
		//fwrite(psaEncodingDataBuffer, 2, nAudioFrameSize, Fileinput);
#endif

		if (nullptr == this->m_pAudioCodec)
		{
			this->m_pAudioCodec = new CAudioFileCodec();
			m_pAudioCodec->CreateAudioEncoder();
		}

		int size = m_pAudioCodec->encodeAudio(psaEncodingDataBuffer, nAudioFrameSize, ucaEncodedDataBuffer);

		return size;
#else
		if (nullptr == m_pG729CodecNative)
		{
			m_pG729CodecNative = new G729CodecNative();
			m_pG729CodecNative->Open();
		}

		int size = m_pG729CodecNative->Encode(psaEncodingDataBuffer, nAudioFrameSize, ucaEncodedDataBuffer);

		return size;

#endif
	}

	int CAudioFileEncodeDecodeSession::DecodeAudioFile(unsigned char *ucaDecodedDataBuffer, int nAudioFrameSize, short *psaDecodingDataBuffer)
	{
#ifdef OPUS_ENABLED

		if (nullptr == this->m_pAudioCodec)
		{
			this->m_pAudioCodec = new CAudioFileCodec();
			m_pAudioCodec->CreateAudioEncoder();
		}
		int size = m_pAudioCodec->decodeAudio(ucaDecodedDataBuffer, nAudioFrameSize, psaDecodingDataBuffer);

#ifdef __CAudioFileEncodeDecodeSession_DUMP_FILE__
		fwrite(psaDecodingDataBuffer, 2, size, FileOutput);
#endif

		return size;
#else
		if (nullptr == m_pG729CodecNative)
		{
			m_pG729CodecNative = new G729CodecNative();
			m_pG729CodecNative->Open();
		}

		int size = m_pG729CodecNative->Decode(ucaDecodedDataBuffer, nAudioFrameSize, psaDecodingDataBuffer);

		return size;

#endif
	}

	int CAudioFileEncodeDecodeSession::StopAudioEncodeDecodeSession()
	{

#ifdef OPUS_ENABLED
		if (nullptr != this->m_pAudioCodec)
		{
			delete this->m_pAudioCodec;

			this->m_pAudioCodec = nullptr;
		}
#else
		if (nullptr != m_pG729CodecNative)
		{
			delete m_pG729CodecNative;

			m_pG729CodecNative = nullptr;
		}
#endif

		return 1;
	}

} // namespace MediaSDK



