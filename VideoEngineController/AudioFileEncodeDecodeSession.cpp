#include "AudioFileEncodeDecodeSession.h"


CAudioFileEncodeDecodeSession::CAudioFileEncodeDecodeSession():

m_pG729CodecNative(NULL)

{
	
}

CAudioFileEncodeDecodeSession::~CAudioFileEncodeDecodeSession()
{
	if (NULL != m_pG729CodecNative)
	{
		delete m_pG729CodecNative;

		m_pG729CodecNative = NULL;
	}
}

int CAudioFileEncodeDecodeSession::StartAudioEncodeDecodeSession()
{
	if (NULL == m_pG729CodecNative)
	{
		m_pG729CodecNative = new G729CodecNative();
	}

	return m_pG729CodecNative->Open();
}

int CAudioFileEncodeDecodeSession::EncodeAudioFile(short *psaEncodingDataBuffer, int nAudioFrameSize, unsigned char *ucaEncodedDataBuffer)
{
	if (NULL == m_pG729CodecNative)
	{
		m_pG729CodecNative = new G729CodecNative();
		m_pG729CodecNative->Open();
	}

	int size = m_pG729CodecNative->Encode(psaEncodingDataBuffer, nAudioFrameSize, ucaEncodedDataBuffer);

	return size;
}

int CAudioFileEncodeDecodeSession::DecodeAudioFile(unsigned char *ucaDecodedDataBuffer, int nAudioFrameSize, short *psaDecodingDataBuffer)
{
	if (NULL == m_pG729CodecNative)
	{
		m_pG729CodecNative = new G729CodecNative();
		m_pG729CodecNative->Open();
	}

	int size = m_pG729CodecNative->Decode(ucaDecodedDataBuffer, nAudioFrameSize, psaDecodingDataBuffer);

	return size;
}

int CAudioFileEncodeDecodeSession::StopAudioEncodeDecodeSession()
{
	if (NULL != m_pG729CodecNative)
	{
		delete m_pG729CodecNative;

		m_pG729CodecNative = NULL;
	}

	return 1;
}




