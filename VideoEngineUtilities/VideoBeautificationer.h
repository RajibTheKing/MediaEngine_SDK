
#ifndef VIDEO_BEAUTIFICATIONER_H
#define VIDEO_BEAUTIFICATIONER_H

#include <string>

#include "../VideoEngineController/Size.h"
#include "../videoEngineController/Tools.h"

#define SHARPING_SHIFTING_NUMBER_1 2
#define SHARPING_SHIFTING_NUMBER_2 1
#define U_VALUE_1 95
#define U_VALUE_2 125
#define V_VALUE_1 135
#define V_VALUE_2 175

class CVideoBeautificationer
{

public:

	CVideoBeautificationer(int iVideoHeight, int iVideoWidth);
	~CVideoBeautificationer();

	void SetHeightWidth(int iVideoHeight, int iVideoWidth);
	void SetDeviceHeightWidth(int iVideoHeight, int iVideoWidth);
	void GenerateUVIndex( int iVideoHeight, int iVideoWidth, int iDataFormat );
	void MakeFrameBeautiful(unsigned char *pixel);
	void MakeFrameBlur(unsigned char *convertingData, int iVideoHeight, int iVideoWidth);
	void MakeFrameBlurAndStore(unsigned char *convertingData, int iVideoHeight, int iVideoWidth);
	bool IsSkinPixel(unsigned char YPixel, unsigned char UPixel, unsigned char VPixel);
	void StartBrightening(int iVideoHeight, int iVideoWidth, int nPrecision);
	bool IsNotSkinPixel(unsigned char UPixel, unsigned char VPixel);

	void SetBrighteningValue(int m_AverageValue, int brightnessPrecision);

	void MakePixelBright(unsigned char *pixel);
	void MakePixelBrightNew(unsigned char *pixel);
	void SetTemperatureThreshold(int nThreshold);
	void IncreaseTemperatureOfPixel(unsigned char *pixel);
	void StopBrightening();
	void SetBrightnessPrecision(int nPrecision);
	void SetBlurScale(int nScale);
	void MakeFrameBright(unsigned char *convertingData, int iVideoHeight, int iVideoWidth, int nPrecision);
	void IncreaseTemperatureOfFrame(unsigned char *convertingData, int iVideoHeight, int iVideoWidth, unsigned char  nThreshold);

	void boxesForGauss(float sigma, int n);
	void GaussianBlur_4thApproach(unsigned char *scl , unsigned char *tcl, int h, int w, float r);
	void boxBlur_4 (unsigned char *scl, unsigned char *tcl , int h, int w, int r);
	void boxBlurH_4 (unsigned char *scl, unsigned char *tcl, int h, int w, int r);

	pair<int, int> BeautificationFilter(unsigned char *pBlurConvertingData, int iLen, int iHeight, int iWidth, int iNewHeight, int iNewWidth, int *effectParam);
	pair<int, int> BeautificationFilter(unsigned char *pBlurConvertingData, int iLen, int iHeight, int iWidth, int *effectParam);
	pair<int, int> BeautificationFilter2(unsigned char *pBlurConvertingData, int iLen, int iHeight, int iWidth, int *effectParam);

	unsigned char m_pBluredImage[MAX_FRAME_HEIGHT * MAX_FRAME_WIDTH << 2];

	Tools m_Tools;

private:

	int m_nPreviousAddValueForBrightening;
	int m_nAverageLuminanceValue;
	int m_nTotalLuminanceValue;
	int m_ThresholdValueForBrightness;
	int m_nBrightnessPrecision;
	int m_nBlurScale;
	int m_nThresholdValue;

	int m_iDeviceHeight;
	int m_iDeviceWidth;
	int luminaceHigh;

	int m_nVideoHeight;
	int m_nVideoWidth;
	int m_Sizes[3];

	int m_pUIndex[MAX_FRAME_HEIGHT * MAX_FRAME_WIDTH + 1];
	int m_pVIndex[MAX_FRAME_HEIGHT * MAX_FRAME_WIDTH  + 1];

	unsigned char modifYUV[266];

	int m_mean[MAX_FRAME_HEIGHT+1][MAX_FRAME_HEIGHT+1];
	int m_variance[MAX_FRAME_HEIGHT+1][MAX_FRAME_HEIGHT+1];

	int m_square[256];
	int m_EffectValue;

	int m_precSharpness[256][2300];
	int m_Multiplication[641][641];

};

#endif 
