
#ifndef MUXING_VIDEO_DATA_H
#define MUXING_VIDEO_DATA_H

#include "AudioVideoEngineDefinitions.h"
#include "ColorConverter.h"
#include "SmartPointer.h"
#include "LockHandler.h"

#define MAX_FRAME_HEIGHT_MUXING_SESSION 640
#define MAX_FRAME_WIDTH_MUXING_SESSION 480

#include <string>

#define RGB 1
#define YUV_NV21 2
#define YUV_NV12 3
#define YUV_I420 4

#define BMP_HEADER_SIZE 54

class CMuxingVideoData
{

public:

	CMuxingVideoData();
    ~CMuxingVideoData();
    void SetBMP32Frame(unsigned char *pBMP32Data, int iLen, int iHeight, int iWidth);
    
    void GenerateUVIndexMatrix(int iHeight, int iWidth);
    void GenerateCheckMatrix(unsigned char *pBMP32Data, int iHeight, int iWidth);
    
    int MergeFrameYUV_With_VideoYUV(unsigned char* pFrameYuv, unsigned char *pVideoYuv, int iHeight, int iWidth, unsigned char *pMergedData);

private:

    int m_IndexFor_U[MAX_FRAME_HEIGHT_MUXING_SESSION * MAX_FRAME_WIDTH_MUXING_SESSION];
    int m_IndexFor_V[MAX_FRAME_HEIGHT_MUXING_SESSION * MAX_FRAME_WIDTH_MUXING_SESSION];
    
    bool m_bCheckMatrix[MAX_FRAME_HEIGHT_MUXING_SESSION * MAX_FRAME_WIDTH_MUXING_SESSION];
    bool m_bBMP32FrameIsSet;
    
	SmartPointer<CLockHandler> m_pMuxingVideoMutex;
};

#endif  // end of MUXING_VIDEO_DATA_H
