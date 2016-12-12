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
	FileOutput = fopen("/storage/emulated/0/FahadO.h264", "w");
//	Fileinput = fopen("/storage/emulated/0/FahadInputPCMN.pcm", "w");;
#endif

	LOGE("fahad -->> Inside CVideoMuxingAndEncodeSession");
	m_pCommonElementsBucket = pCommonElementsBucket;
	m_CMuxingVideoData = NULL;
	m_VideoEncoder = NULL;
	m_ColorConverter = NULL;
	m_YUV420ConvertedLen = 0;
	m_iFinalEncodedFrameBufferIndx = 0;

	m_pVideoMuxingEncodeSessionMutex.reset(new CLockHandler);
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

	SHARED_PTR_DELETE(m_pVideoMuxingEncodeSessionMutex);
}

int CVideoMuxingAndEncodeSession::StartVideoMuxingAndEncodeSession(unsigned char *pBMP32Data,int iLen, int nVideoHeight, int nVideoWidth)
{
	Locker lock(*m_pVideoMuxingEncodeSessionMutex);
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

    return m_VideoEncoder->CreateVideoEncoder(nVideoHeight, nVideoWidth, FRAME_RATE_MUXING_ENCODE_SESSION, I_FRAME_INTERVAL_MUXING_ENCODE_SESSION,false, 11);


}

int CVideoMuxingAndEncodeSession::FrameMuxAndEncode( unsigned char *pVideoYuv, int iHeight, int iWidth)
{
	Locker lock(*m_pVideoMuxingEncodeSessionMutex);

	if( NULL == this->m_VideoEncoder || NULL == m_ColorConverter || NULL == m_CMuxingVideoData)
	{
		LOGE("fahad -->> CVideoMuxingAndEncodeSession::FrameMuxAndEncode  return 0");
		return 0;
	}

	LOGE("fahad -->> CVideoMuxingAndEncodeSession::FrameMuxAndEncode  processing");

	m_ColorConverter->mirrorRotateAndConvertNV21ToI420( pVideoYuv, m_ucaRotateYUVFrame );
	int iMergedYUVLen = m_CMuxingVideoData->MergeFrameYUV_With_VideoYUV(m_ucaYUVMuxFrame, m_ucaRotateYUVFrame, iHeight, iWidth, m_ucaMergedYUVFrame);

	//m_ColorConverter->ConvertI420ToNV21(m_ucaMergedYUVFrame, iHeight, iWidth);
	int encodedSize = m_VideoEncoder->EncodeVideoFrame(m_ucaMergedYUVFrame, iMergedYUVLen, m_ucaMergedData);

	if(m_iFinalEncodedFrameBufferIndx + encodedSize < FINAL_ENCODED_FRAME_BUFFER_LEN)
	{
		memcpy(m_ucaFinalEncodedFrameBuffer + m_iFinalEncodedFrameBufferIndx, m_ucaMergedData, encodedSize );
		m_iFinalEncodedFrameBufferIndx += encodedSize;
	}

	LOGE("fahad -->> CVideoMuxingAndEncodeSession::FrameMuxAndEncode  encodedSize = %d", encodedSize);


	return encodedSize;

}


int CVideoMuxingAndEncodeSession::StopVideoMuxingAndEncodeSession(unsigned char *finalData)
{
	Locker lock(*m_pVideoMuxingEncodeSessionMutex);
	
	memcpy(finalData, m_ucaFinalEncodedFrameBuffer, m_iFinalEncodedFrameBufferIndx);

#ifdef __CVideoFileEncodeDecodeSession_DUMP_FILE__
	fwrite(finalData, 1, m_iFinalEncodedFrameBufferIndx, FileOutput);
	fflush(FileOutput);
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

	return m_iFinalEncodedFrameBufferIndx;
}




