
#ifndef VIDEO_MUXING_ENCODE_SESSION
#define VIDEO_MUXING_ENCODE_SESSION

#include "../VideoEngineUtilities/ColorConverter.h"
#include "../VideoEngineUtilities/MuxingVideoData.h"
#include "VideoEncoder.h"
#include "CommonElementsBucket.h"
#include "SmartPointer.h"
#include "LockHandler.h"


#define BMP_HEADER_SIZE 54

#define FINAL_ENCODED_FRAME_BUFFER_LEN 1024*1024*10


class CVideoMuxingAndEncodeSession
{

public:

	CVideoMuxingAndEncodeSession(CCommonElementsBucket *pCommonElementsBucket);
	~CVideoMuxingAndEncodeSession();

	int StartVideoMuxingAndEncodeSession(unsigned char *pBMP32Data,int iLen, int nVideoHeight, int nVideoWidth);
	int FrameMuxAndEncode( unsigned char *pVideoYuv, int iHeight, int iWidth);
	int StopVideoMuxingAndEncodeSession(unsigned char *finalData);

private:

	CCommonElementsBucket *m_pCommonElementsBucket;
	CMuxingVideoData *m_CMuxingVideoData;
	CVideoEncoder *m_VideoEncoder;
	CColorConverter *m_ColorConverter;

	unsigned char m_ucaBMP32Frame[MAX_FRAME_HEIGHT * MAX_FRAME_WIDTH * 4];
	unsigned char m_ucaBMP24Frame[MAX_FRAME_HEIGHT * MAX_FRAME_WIDTH * 3];
	unsigned char m_ucaYUVMuxFrame[MAX_FRAME_HEIGHT * MAX_FRAME_WIDTH * 2];
	unsigned char m_ucaMergedYUVFrame[MAX_FRAME_HEIGHT * MAX_FRAME_WIDTH * 2];
	unsigned char m_ucaRotateYUVFrame[MAX_FRAME_HEIGHT * MAX_FRAME_WIDTH * 2];
	unsigned char m_ucaEncodedFrame[MAX_FRAME_HEIGHT * MAX_FRAME_WIDTH * 2];
	unsigned char m_ucaMergedData[MAX_FRAME_HEIGHT * MAX_FRAME_WIDTH * 2];
	unsigned char m_ucaFinalEncodedFrameBuffer[FINAL_ENCODED_FRAME_BUFFER_LEN];



	int m_YUV420ConvertedLen;
	int m_iFinalEncodedFrameBufferIndx;

	SmartPointer<CLockHandler> m_pVideoMuxingEncodeSessionMutex;
};


#endif
