#include "VideoDecoder.h"
#include "CommonElementsBucket.h"
#include "DefinedDataTypes.h"
#include "LogPrinter.h"
#include "Tools.h"
#include "codec_api.h"

CVideoDecoder::CVideoDecoder(CCommonElementsBucket* sharedObject, CDecodingBuffer *decodingBuffer) :

m_pCommonElementsBucket(sharedObject),
m_pDecodingBuffer(decodingBuffer),
m_pSVCVideoDecoder(NULL)

{
	CLogPrinter_Write(CLogPrinter::DEBUGS, "CVideoDecoder::CVideoDecoder video decoder created");
}

CVideoDecoder::~CVideoDecoder()
{
    if(NULL != m_pSVCVideoDecoder)
    {
        m_pSVCVideoDecoder->Uninitialize();
        
        m_pSVCVideoDecoder = NULL;
    }
}

int CVideoDecoder::CreateVideoDecoder()
{
	CLogPrinter_Write(CLogPrinter::INFO, "CVideoDecoder::CreateVideoDecoder");

	long iRet = WelsCreateDecoder(&m_pSVCVideoDecoder);

	if (iRet != 0 || NULL == m_pSVCVideoDecoder)
	{
		CLogPrinter_Write(CLogPrinter::INFO, "Unable to create OpenH264 decoder");

		return -1;
	}

	SDecodingParam decParam = { 0 };
	decParam.sVideoProperty.size = sizeof(decParam.sVideoProperty);
	decParam.eOutputColorFormat = videoFormatI420;
	decParam.uiTargetDqLayer = (uint8_t)-1;
	decParam.eEcActiveIdc = ERROR_CON_FRAME_COPY;
	decParam.sVideoProperty.eVideoBsType = VIDEO_BITSTREAM_DEFAULT;

	iRet = m_pSVCVideoDecoder->Initialize(&decParam);

	if (iRet != 0)
	{
		CLogPrinter_Write(CLogPrinter::INFO, "Unable to initialize OpenH264 decoder");
		return -1;
	}

	int flag = 0;
#define SET_DECODER_OPTION(key, value) \
    flag = value; \
    if(m_pSVCVideoDecoder->SetOption(key, &flag) != 0){ \
        cout << "Error in setting option " << #key << " to OpenH264 decoder\n"; \
			    }

	SET_DECODER_OPTION(DECODER_OPTION_DATAFORMAT, videoFormatI420);
	SET_DECODER_OPTION(DECODER_OPTION_END_OF_STREAM, 0);

	CLogPrinter_Write(CLogPrinter::DEBUGS, "CVideoDecoder::CreateVideoDecoder open h264 video decoder initialized");

	return 1;
}

int CVideoDecoder::Decode(unsigned char *in_data, unsigned int in_size, unsigned char *out_data, int &iVideoHeight, int &iVideoWidth)
{
	if (!m_pSVCVideoDecoder)
	{
		CLogPrinter_Write(CLogPrinter::DEBUGS, "CVideoDecoder::Decode pSVCVideoDecoder == NULL");
		return 0;
	}

	int strides[2] = { 0 };
	unsigned char *outputPlanes[3] = { NULL };
	DECODING_STATE decodingState = m_pSVCVideoDecoder->DecodeFrame(in_data, in_size, outputPlanes, strides, iVideoWidth, iVideoHeight);

	if (decodingState != 0)
	{
        //printf("CVideoDecoder::Decode OpenH264 Decoding FAILEDDDDDDDDDD, %d\n", decodingState);
		return 0;
	}

	int retsize = 0;

	{
		int plane = 0;
		unsigned char *from = outputPlanes[plane];
		int stride = strides[0];

		for (int row = 0; row < iVideoHeight; row++)
		{
			size_t w_count = iVideoWidth;
			memcpy(out_data + retsize, from, w_count);
			retsize += w_count;
			from += stride;
		}
	}

	for (int plane = 1; plane < 3; plane++)
	{
		unsigned char *from = outputPlanes[plane];
		int stride = strides[1];
		int halfiVideoHeight = iVideoHeight >> 1;
		int halfiVideoWidth = iVideoWidth >> 1;

		for (int row = 0; row < halfiVideoHeight; row++)
		{
			size_t w_count = halfiVideoWidth;
			memcpy(out_data + retsize, from, w_count);
			retsize += w_count;
			from += stride;
		}
	}

	return retsize;
}




