#include "VideoMuxingAndEncodeSession.h"


#define __CVideoFileEncodeDecodeSession_DUMP_FILE__

#ifdef __CVideoFileEncodeDecodeSession_DUMP_FILE__
FILE *FileOutput;
FILE *Fileinput;
#endif

#define FRAME_RATE_MUXING_ENCODE_SESSION 30
#define I_FRAME_INTERVAL_MUXING_ENCODE_SESSION 60

CVideoMuxingAndEncodeSession::CVideoMuxingAndEncodeSession(CCommonElementsBucket *pCommonElementsBucket)
{
#ifdef __CVideoFileEncodeDecodeSession_DUMP_FILE__
	//FileOutput = fopen("/sdcard/OutputPCMN.pcm", "a+");
	//Fileinput = fopen("/sdcard/FahadInputPCMN.pcm", "a+");
	FileOutput = fopen("/storage/emulated/0/FahadO.h264", "w");;
//	Fileinput = fopen("/storage/emulated/0/FahadInputPCMN.pcm", "w");;
#endif

	LOGE("fahad -->> Inside CVideoMuxingAndEncodeSession");
	m_pCommonElementsBucket = pCommonElementsBucket;
	m_CMuxingVideoData = NULL;
	m_VideoEncoder = NULL;
	m_ColorConverter = NULL;
	m_YUV420ConvertedLen = 0;
}

CVideoMuxingAndEncodeSession::~CVideoMuxingAndEncodeSession()
{

#ifdef __CVideoFileEncodeDecodeSession_DUMP_FILE__
	fclose(FileOutput);
	//fclose(Fileinput);
#endif

	if (NULL != this->m_CMuxingVideoData)
	{
		delete this->m_CMuxingVideoData;

		this->m_CMuxingVideoData = NULL;
	}

	if (NULL != this->m_VideoEncoder)
	{
		delete this->m_VideoEncoder;

		this->m_VideoEncoder = NULL;
	}

	if (NULL != this->m_ColorConverter)
	{
		delete this->m_ColorConverter;

		this->m_ColorConverter = NULL;
	}
}

int CVideoMuxingAndEncodeSession::StartVideoMuxingAndEncodeSession(unsigned char *pBMP32Data,int iLen, int nVideoHeight, int nVideoWidth)
{
	LOGE("fahad -->> CVideoMuxingAndEncodeSession::StartVideoMuxingAndEncodeSession  nVideoHeight = %d, int nVideoWidth = %d, bmpLen = %d", nVideoHeight, nVideoWidth, iLen);
	if(NULL == this->m_CMuxingVideoData)
	{
		this->m_CMuxingVideoData = new CMuxingVideoData();
		
	}

	if(NULL == this->m_VideoEncoder)
	{
		this->m_VideoEncoder = new CVideoEncoder(m_pCommonElementsBucket, 205);

	}


	if(NULL == this->m_ColorConverter)
	{
		LOGE("fahad -->> CColorConverter Is NOw NULLLLLLL");
		this->m_ColorConverter = new CColorConverter( nVideoHeight, nVideoWidth);
		LOGE("fahad -->> CColorConverter constructor returned");

	}


	LOGE("fahad -->> CColorConverter before ConvertRGB32ToRGB24 call");

	int iBMP24ConvertedLen =this->m_ColorConverter->ConvertRGB32ToRGB24(pBMP32Data+BMP_HEADER_SIZE,nVideoHeight, nVideoWidth, m_ucaBMP24Frame);

	LOGE("fahad -->> CColorConverter after ConvertRGB32ToRGB24 call");

	m_YUV420ConvertedLen = m_ColorConverter->ConvertRGB24ToI420(m_ucaBMP24Frame, m_ucaYUVMuxFrame);

	m_CMuxingVideoData->SetBMP32Frame(pBMP32Data+BMP_HEADER_SIZE, iLen - BMP_HEADER_SIZE,nVideoHeight, nVideoWidth);

    return m_VideoEncoder->CreateVideoEncoder(nVideoHeight, nVideoWidth, FRAME_RATE_MUXING_ENCODE_SESSION, I_FRAME_INTERVAL_MUXING_ENCODE_SESSION,false);


}

int CVideoMuxingAndEncodeSession::FrameMuxAndEncode( unsigned char *pVideoYuv, int iHeight, int iWidth, unsigned char *pMergedData)
{

	if( NULL == this->m_VideoEncoder || NULL == m_ColorConverter || NULL == m_CMuxingVideoData)
	{
		LOGE("fahad -->> CVideoMuxingAndEncodeSession::FrameMuxAndEncode  return 0");
		return 0;
	}

	LOGE("fahad -->> CVideoMuxingAndEncodeSession::FrameMuxAndEncode  processing");
	//memcpy(pMergedData, pVideoYuv, (iHeight * iWidth * 3) / 2);
	m_ColorConverter->mirrorRotateAndConvertNV21ToI420( pVideoYuv, m_ucaRotateYUVFrame );
	int iMergedYUVLen = m_CMuxingVideoData->MergeFrameYUV_With_VideoYUV(m_ucaYUVMuxFrame, m_ucaRotateYUVFrame, iHeight, iWidth, m_ucaMergedYUVFrame);

	//m_ColorConverter->ConvertI420ToNV21(m_ucaMergedYUVFrame, iHeight, iWidth);
	int size = m_VideoEncoder->EncodeVideoFrame(m_ucaMergedYUVFrame, iMergedYUVLen, pMergedData);

	LOGE("fahad -->> CVideoMuxingAndEncodeSession::FrameMuxAndEncode  size = %d", size);

#ifdef __CVideoFileEncodeDecodeSession_DUMP_FILE__
	fwrite(pMergedData, 1, size, FileOutput);
	fflush(FileOutput);
#endif

	return size;

}


int CVideoMuxingAndEncodeSession::StopVideoMuxingAndEncodeSession()
{

	if (NULL != this->m_CMuxingVideoData)
	{
		delete this->m_CMuxingVideoData;

		this->m_CMuxingVideoData = NULL;
	}

	if (NULL != this->m_VideoEncoder)
	{
		delete this->m_VideoEncoder;

		this->m_VideoEncoder = NULL;
	}

	if (NULL != this->m_ColorConverter)
	{
		delete this->m_ColorConverter;

		this->m_ColorConverter = NULL;
	}

	return 1;
}




