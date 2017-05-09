
#include "VideoDecoder.h"
#include "CommonElementsBucket.h"
#include "LogPrinter.h"
#include "ThreadTools.h"

#include <string>

CVideoDecoder::CVideoDecoder(CCommonElementsBucket* pSharedObject) :

m_pcCommonElementsBucket(pSharedObject),
m_pcSVCVideoDecoder(NULL)

{
	CLogPrinter_Write(CLogPrinter::DEBUGS, "CVideoDecoder::CVideoDecoder video decoder created");
}

CVideoDecoder::~CVideoDecoder()
{
	if (NULL != m_pcSVCVideoDecoder)
    {
		m_pcSVCVideoDecoder->Uninitialize();
        
		m_pcSVCVideoDecoder = NULL;
    }
}

int CVideoDecoder::CreateVideoDecoder()
{
	CLogPrinter_Write(CLogPrinter::INFO, "CVideoDecoder::CreateVideoDecoder");

	long nReturnedValueFromDecoder = WelsCreateDecoder(&m_pcSVCVideoDecoder);

	if (nReturnedValueFromDecoder != 0 || NULL == m_pcSVCVideoDecoder)
	{
		CLogPrinter_Write(CLogPrinter::INFO, "Unable to create OpenH264 decoder");

		return -1;
	}

	SDecodingParam decoderParemeters = { 0 };
	SVideoProperty sVideoProparty;

	sVideoProparty.eVideoBsType = VIDEO_BITSTREAM_AVC;

	decoderParemeters.sVideoProperty.size = sizeof(decoderParemeters.sVideoProperty);
	decoderParemeters.sVideoProperty  = sVideoProparty;
	decoderParemeters.uiTargetDqLayer = (uint8_t)-1;
	decoderParemeters.eEcActiveIdc = ERROR_CON_FRAME_COPY;
	decoderParemeters.sVideoProperty.eVideoBsType = VIDEO_BITSTREAM_AVC;

	nReturnedValueFromDecoder = m_pcSVCVideoDecoder->Initialize(&decoderParemeters);

	if (nReturnedValueFromDecoder != 0)
	{
		CLogPrinter_Write(CLogPrinter::INFO, "Unable to initialize OpenH264 decoder");
		return -1;
	}

	/*if (SetDecoderOption(DECODER_OPTION_DATAFORMAT, videoFormatI420) != 0)
	{
		cout << "Error in setting option " << DECODER_OPTION_DATAFORMAT << " to OpenH264 decoder\n";
	}*/

	if (SetDecoderOption(DECODER_OPTION_END_OF_STREAM, 0) != 0)
	{
		//cout << "Error in setting option " << DECODER_OPTION_END_OF_STREAM << " to OpenH264 decoder\n";
	}

	CLogPrinter_Write(CLogPrinter::DEBUGS, "CVideoDecoder::CreateVideoDecoder open h264 video decoder initialized");

	return 1;
}

int CVideoDecoder::SetDecoderOption(int nKey, int nValue)
{
	return m_pcSVCVideoDecoder->SetOption(DECODER_OPTION_END_OF_STREAM, &nValue);
}

int CVideoDecoder::DecodeVideoFrame(unsigned char *ucaDecodingVideoFrameData, unsigned int unLength, unsigned char *ucaDecodedVideoFrameData, int &rnVideoHeight, int &rnVideoWidth)
{
	if (!m_pcSVCVideoDecoder)
	{
		CLogPrinter_Write(CLogPrinter::DEBUGS, "CVideoDecoder::Decode pSVCVideoDecoder == NULL");

		return 0;
	}

	int strides[2] = { 0 };
	unsigned char *outputPlanes[3] = { NULL };
	DECODING_STATE decodingState = m_pcSVCVideoDecoder->DecodeFrame(ucaDecodingVideoFrameData, unLength, outputPlanes, strides, rnVideoWidth, rnVideoHeight);

	if (decodingState != 0)
	{
        //printf("CVideoDecoder::Decode OpenH264 Decoding FAILEDDDDDDDDDD, %d\n", decodingState);
		return 0;
	}

	int decodedVideoFrameSize = 0;

	{
		int plane = 0;
		unsigned char *outputPlane = outputPlanes[plane];
		int stride = strides[0];

		for (int row = 0; row < rnVideoHeight; row++)
		{
			memcpy(ucaDecodedVideoFrameData + decodedVideoFrameSize, outputPlane, rnVideoWidth);

			decodedVideoFrameSize += rnVideoWidth;
			outputPlane += stride;
		}
	}

	for (int plane = 1; plane < 3; plane++)
	{
		unsigned char *outputPlane = outputPlanes[plane];
		int stride = strides[1];
		int halfiVideoHeight = rnVideoHeight >> 1;
		int halfiVideoWidth = rnVideoWidth >> 1;

		for (int row = 0; row < halfiVideoHeight; row++)
		{
			size_t stLength = halfiVideoWidth;

			memcpy(ucaDecodedVideoFrameData + decodedVideoFrameSize, outputPlane, stLength);

			decodedVideoFrameSize += stLength;
			outputPlane += stride;
		}
	}

	return decodedVideoFrameSize;
}




