
#ifndef IPV_COLOR_CONVERTER_H
#define IPV_COLOR_CONVERTER_H

#include "SmartPointer.h"
#include "CommonTypes.h"
#include "Size.h"
#include "VideoBeautificationer.h"

#if _MSC_VER > 1000
#pragma once
#endif

#include <string>

namespace MediaSDK
{

	class CCommonElementsBucket;

	#define RGB24   1001
	#define RGB32   1002
	#define YUV420  1003
	#define YUVNV12 1004
	#define YUVNV21 1005
	#define YUVYV12 1006

	class CColorConverter
	{

	public:

		CColorConverter(int iVideoHeight, int iVideoWidth, CCommonElementsBucket* commonElementsBucket, long long lfriendID);
		~CColorConverter();

		int ConvertI420ToNV21(unsigned char *convertingData, int iVideoHeight, int iVideoWidth);
		int ConvertI420ToNV12(unsigned char *convertingData, int iVideoHeight, int iVideoWidth);
		int ConvertI420ToYV12(unsigned char *convertingData, int iVideoHeight, int iVideoWidth);
		int ConvertNV12ToI420(unsigned char *convertingData, int iVideoHeight, int iVideoWidth);
		int ConvertNV21ToI420(unsigned char *convertingData, int iVideoHeight, int iVideoWidth);
		int ConvertYV12ToI420(unsigned char *convertingData, int iVideoHeight, int iVideoWidth);
		int ConvertYUY2ToI420(unsigned char * input, unsigned char * output, int iVideoHeight, int iVideoWidth);
		int ConvertRGB24ToI420(unsigned char *input, unsigned char *output, int iVideoHeight, int iVideoWidth);

		int ConvertRGB32ToRGB24(unsigned char *input, int iHeight, int iWidth, unsigned char *output);


		void mirrorRotateAndConvertNV21ToI420(unsigned char *m_pFrame, unsigned char *pData, int iVideoHeight, int iVideoWidth);
		void NegativeRotateAndConvertNV12ToI420(unsigned char *m_pFrame, unsigned char *pData, int iVideoHeight, int iVideoWidth);
		void mirrorRotateAndConvertNV12ToI420(unsigned char *m_pFrame, unsigned char *pData, int iVideoHeight, int iVideoWidth);
		void mirrorAndConvertNV12ToI420(unsigned char *m_pFrame, unsigned char *pData, int iVideoHeight, int iVideoWidth);
		void mirrorRotateAndConvertNV21ToI420ForBackCam90(unsigned char *m_pFrame, unsigned char *pData, int iVideoHeight, int iVideoWidth);
		void mirrorRotateAndConvertNV21ToI420ForBackCam270(unsigned char *m_pFrame, unsigned char *pData, int iVideoHeight, int iVideoWidth);
		void mirrorRotateAndConvertNV12ToI420ForBackCam(unsigned char *m_pFrame, unsigned char *pData, int iVideoHeight, int iVideoWidth);
		int mirrorI420_XDirection(unsigned char *inData, unsigned char *outData, int iHeight, int iWidth);

		int ConverterYUV420ToRGB24(unsigned char * pYUVs, unsigned char * pRGBs, int height, int width);

		int DownScaleYUVNV12_YUVNV21_AverageNotApplied(unsigned char* pData, int &iHeight, int &iWidth, unsigned char* outputData);
		int DownScaleYUVNV12_YUVNV21_AverageVersion1(unsigned char* pData, int &iHeight, int &iWidth, unsigned char* outputData);
		int DownScaleYUVNV12_YUVNV21_AverageVersion2(unsigned char* pData, int &iHeight, int &iWidth, unsigned char* outputData);

		int DownScaleYUV420_EvenVersion(unsigned char* pData, int &iHeight, int &iWidth, unsigned char* outputData);
		int DownScaleYUV420_Dynamic(unsigned char* pData, int &iHeight, int &iWidth, unsigned char* outputData, int diff);
		int DownScaleYUV420_Dynamic_Version2(unsigned char* pData, int inHeight, int inWidth, unsigned char* outputData, int outHeight, int outWidth);

		void mirrorYUVI420(unsigned char *pFrame, unsigned char *pData, int iHeight, int iWidth);

		bool GetSmallFrameStatus();
		void GetSmallFrame(unsigned char *pSmallFrame);


		int GetSmallFrameWidth();
		int GetSmallFrameHeight();

		int GetScreenHeight();
		int GetScreenWidth();

		void ClearSmallScreen();

		void SetHeightWidth(int iVideoHeight, int iVideoWidth);
		void SetDeviceHeightWidth(int iVideoHeight, int iVideoWidth);

		int CreateFrameBorder(unsigned char* pData, int iHeight, int iWidth, int Y, int U, int V);
		void SetSmallFrame(unsigned char * smallFrame, int iHeight, int iWidth, int nLength, int iTargetHeight, int iTargetWidth, bool bShouldBeCropped);
		int getUIndex(int h, int w, int yVertical, int xHorizontal, int& total);
		int getVIndex(int h, int w, int yVertical, int xHorizontal, int& total);
		int getUIndexforNV12(int h, int w, int yVertical, int xHorizontal, int& total);
		int getVIndexforNV12(int h, int w, int yVertical, int xHorizontal, int& total);
		int getUIndexforNV21(int h, int w, int yVertical, int xHorizontal, int& total);
		int getVIndexforNV21(int h, int w, int yVertical, int xHorizontal, int& total);
		int Merge_Two_Video(unsigned char *pInData1, int iPosX, int iPosY, int iVideoHeight, int iVideoWidth);
		int Merge_Two_Video_With_Round_Corner(unsigned char *pInData1, int iPosX, int iPosY, int iVideoHeight, int iVideoWidth);
		int Merge_Two_VideoNV12(unsigned char *pInData1, int iPosX, int iPosY, int iVideoHeight, int iVideoWidth);
		int Merge_Two_VideoNV21(unsigned char *pInData1, int iPosX, int iPosY, int iVideoHeight, int iVideoWidth);
		int CropWithAspectRatio_YUVNV12_YUVNV21_RGB24(unsigned char* pData, int inHeight, int inWidth, int screenHeight, int screenWidth, unsigned char* outputData, int &outHeight, int &outWidth, int pColorFormat);
		int Crop_RGB24(unsigned char* pData, int inHeight, int inWidth, int startXDiff, int endXDiff, int startYDiff, int endYDiff, unsigned char* outputData, int &outHeight, int &outWidth);
		int Crop_YUV420(unsigned char* pData, int inHeight, int inWidth, int startXDiff, int endXDiff, int startYDiff, int endYDiff, unsigned char* outputData, int &outHeight, int &outWidth);
		int Crop_YUVNV12_YUVNV21(unsigned char* pData, int inHeight, int inWidth, int startXDiff, int endXDiff, int startYDiff, int endYDiff, unsigned char* outputData, int &outHeight, int &outWidth);
		int TestVideoEffect(int *param, int size);

		void CalculateAspectRatioWithScreenAndModifyHeightWidth(int inHeight, int inWidth, int iScreenHeight, int iScreenWidth, int &newHeight, int &newWidth);
		int GetInsetLocation(int inHeight, int inWidth, int &iPosX, int &iPosY);
        int RotateI420(unsigned char *pInput, int inHeight, int inWidth, unsigned char *pOutput, int &outHeight, int &outWidth, int rotationParameter);
	private:

		int m_iDeviceHeight;
		int m_iDeviceWidth;

		/*int m_iVideoHeight;
		int m_iVideoWidth;

		int m_YPlaneLength;
		int m_VPlaneLength;
		int m_UVPlaneMidPoint;
		int m_UVPlaneEnd;*/

		int m_iSmallFrameHeight;
		int m_iSmallFrameWidth;

		int m_iSmallFrameSize;

		bool m_bMergingSmallFrameEnabled;

		int m_PrevAddValue;
		int m_AverageValue;
		int m_ThresholdValue;

		CCommonElementsBucket* m_pCommonElementsBucket;
		long long m_lfriendID;

		unsigned char m_pVPlane[(MAX_FRAME_HEIGHT * MAX_FRAME_WIDTH) >> 2];
		unsigned char m_pUPlane[(MAX_FRAME_HEIGHT * MAX_FRAME_WIDTH) >> 2];
		unsigned char m_pTempPlane[(MAX_FRAME_HEIGHT * MAX_FRAME_WIDTH) >> 2];
		unsigned char m_pSmallFrame[(MAX_FRAME_HEIGHT * MAX_FRAME_WIDTH) >> 1];

		int CumulativeSum[MAX_FRAME_HEIGHT][MAX_FRAME_HEIGHT];
		int CumulativeSum_U[MAX_FRAME_HEIGHT][MAX_FRAME_HEIGHT];
		int CumulativeSum_V[MAX_FRAME_HEIGHT][MAX_FRAME_HEIGHT];

		unsigned char m_pClip[900];
		bool m_bClipInitialization;
		int cyb, cyg, cyr;

		int m_Multiplication[641][641];

		//CVideoBeautificationer *m_VideoBeautificationer;

		SharedPointer<CLockHandler> m_pColorConverterMutex;
	};

} //namespace MediaSDK

#endif 
