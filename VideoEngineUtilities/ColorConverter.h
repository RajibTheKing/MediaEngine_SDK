
#ifndef _COLOR_CONVERTER_H_
#define _COLOR_CONVERTER_H_

#include "AudioVideoEngineDefinitions.h"
#include "SmartPointer.h"
#include "LockHandler.h"

#if _MSC_VER > 1000
#pragma once
#endif

#define MAX_FRAME_HEIGHT 640
#define MAX_FRAME_WIDTH 480

#include <string>

class CColorConverter
{

public:

	CColorConverter(int iVideoHeight, int iVideoWidth);
	~CColorConverter();

	int ConvertI420ToNV21(unsigned char *convertingData, int iVideoHeight, int iVideoWidth);
	int ConvertI420ToNV12(unsigned char *convertingData, int iVideoHeight, int iVideoWidth);
	int ConvertI420ToYV12(unsigned char *convertingData, int iVideoHeight, int iVideoWidth);
	int ConvertNV12ToI420(unsigned char *convertingData);
	int ConvertNV21ToI420(unsigned char *convertingData);
	int ConvertYUY2ToI420(unsigned char * input, unsigned char * output);
	int ConvertRGB24ToI420(unsigned char *input, unsigned char *output);

	void mirrorRotateAndConvertNV21ToI420(unsigned char *m_pFrame, unsigned char *pData);
	void mirrorRotateAndConvertNV12ToI420(unsigned char *m_pFrame, unsigned char *pData);
	void mirrorAndConvertNV12ToI420(unsigned char *m_pFrame, unsigned char *pData);
	void mirrorRotateAndConvertNV21ToI420ForBackCam(unsigned char *m_pFrame, unsigned char *pData);
	void mirrorRotateAndConvertNV12ToI420ForBackCam(unsigned char *m_pFrame, unsigned char *pData);

	int ConverterYUV420ToRGB24(unsigned char * pYUVs, unsigned char * pRGBs, int height, int width);

	int GetWidth();
	int GetHeight();

	void SetHeightWidth(int iVideoHeight, int iVideoWidth);

private:

	int m_iVideoHeight;
	int m_iVideoWidth;
	int m_YPlaneLength;
	int m_VPlaneLength;
	int m_UVPlaneMidPoint;
	int m_UVPlaneEnd;

	unsigned char m_pVPlane[(MAX_FRAME_HEIGHT * MAX_FRAME_WIDTH) >> 2];
	unsigned char m_pUPlane[(MAX_FRAME_HEIGHT * MAX_FRAME_WIDTH) >> 2];
	unsigned char m_pTempPlane[(MAX_FRAME_HEIGHT * MAX_FRAME_WIDTH) >> 2];
	
	unsigned char m_pClip[900];
	bool m_bClipInitialization;
	int cyb, cyg, cyr;
	
	int m_Multiplication[481][641];

	SmartPointer<CLockHandler> m_pColorConverterMutex;
};

#endif 
