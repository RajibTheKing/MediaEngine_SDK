#include "AudioEncoder.h"
#include "CommonElementsBucket.h"
#include "DefinedDataTypes.h"
#include "LogPrinter.h"
#include "G729CodecNative.h"



CAudioEncoder::CAudioEncoder(CCommonElementsBucket* sharedObject) :
m_pCommonElementsBucket(sharedObject)
{
	m_pMediaSocketMutex.reset(new CLockHandler);
	m_pAudioEncoderThread = NULL;

	CLogPrinter::Write(CLogPrinter::INFO, "CAudioEncoder::CAudioEncoder");
}

CAudioEncoder::~CAudioEncoder()
{
	/*	if (m_pAudioEncoderThread)
	{
	delete m_pAudioEncoderThread;
	m_pAudioEncoderThread = NULL;
	}*/

	/*if (NULL != m_pCEncodedFramePacketizer)
	{
	delete m_pCEncodedFramePacketizer;
	m_pCEncodedFramePacketizer = NULL;
	}*/

	SHARED_PTR_DELETE(m_pMediaSocketMutex);
}

int CAudioEncoder::CreateAudioEncoder()
{
	CLogPrinter::Write(CLogPrinter::INFO, "CAudioEncoder::CreateAudioEncoder");

	return 1;
}

int CAudioEncoder::Encode(short *in_data, unsigned int in_size, unsigned char *out_buffer)
{
	CLogPrinter::Write(CLogPrinter::INFO, "CAudioEncoder::EncodeAudioData");

	int size = Encode(in_data, in_size, out_buffer);

	return size;
}

int CAudioEncoder::Decode(unsigned char *in_data, unsigned int in_size, short *out_buffer)
{
	CLogPrinter::Write(CLogPrinter::INFO, "CAudioEncoder::EncodeAudioData");

	int size = Decode(in_data, in_size, out_buffer);

	return size;
}

void *CAudioEncoder::CreateAudioEncoderThread(void* param)
{
	CAudioEncoder *pThis = (CAudioEncoder*)param;
	//	pThis->CreateAudioEncoderSDP();

	return NULL;
}

void CAudioEncoder::StartAudioEncoderThread()
{
	//	if (m_pAudioEncoderThread != NULL)
	//	{
	//        CLogPrinter::Write(CLogPrinter::ERRORS, "already StartAudioEncoderThread is running");
	//		return;
	//	}
	std::thread t(CreateAudioEncoderThread, this);
	t.detach();

	return;
}

void CAudioEncoder::StopAudioEncoderThread()
{
	if (m_pAudioEncoderThread != NULL)
	{
		Locker lock(*m_pMediaSocketMutex);

		/*delete m_pAudioEncoderThread;
		m_pAudioEncoderThread = NULL;*/
	}

}





