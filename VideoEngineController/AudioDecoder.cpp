#include "AudioDecoder.h"
#include "CommonElementsBucket.h"
#include "DefinedDataTypes.h"
#include "LogPrinter.h"
#include "Tools.h"

CAudioDecoder::CAudioDecoder(CCommonElementsBucket* sharedObject) :

m_pCommonElementsBucket(sharedObject)

{
	CLogPrinter_Write(CLogPrinter::INFO, "CAudioDecoder::CAudioDecoder");
}

CAudioDecoder::~CAudioDecoder()
{

}

int CAudioDecoder::CreateAudioDecoder()
{
	CLogPrinter_Write(CLogPrinter::INFO, "CAudioDecoder::CreateAudioDecoder");
	
	return 1;
}

int CAudioDecoder::Decode(unsigned char *in_data, unsigned int in_size, unsigned char *out_data)
{
	CLogPrinter_Write(CLogPrinter::INFO, "CAudioDecoder::Decode called");

	return 1;
}




