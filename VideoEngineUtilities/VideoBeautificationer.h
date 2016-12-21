
#ifndef VIDEO_BEAUTIFICATIONER_H
#define VIDEO_BEAUTIFICATIONER_H

#include <string>

#include "../VideoEngineController/Size.h"

class CVideoBeautificationer
{

public:

	CVideoBeautificationer(int iVideoHeight, int iVideoWidth);
	~CVideoBeautificationer();

	void SetHeightWidth(int iVideoHeight, int iVideoWidth);
	void GenerateUVIndex( int iVideoHeight, int iVideoWidth, int iDataFormat );
	void MakeFrameBeautiful(unsigned char *convertingData, int iVideoHeight, int iVideoWidth, unsigned char *convertedData);
	void MakeFrameBlur(unsigned char *convertingData, int iVideoHeight, int iVideoWidth);
	void MakeFrameBlurAndStore(unsigned char *convertingData, int iVideoHeight, int iVideoWidth);
	bool IsSkinPixel(unsigned char *pixel);
	void StartBrightening(int iVideoHeight, int iVideoWidth, int nPrecision);

	void SetBrighteningValue(int m_AverageValue, int brightnessPrecision);

	void MakePixelBright(unsigned char *pixel);
	void SetTemperatureThreshold(int nThreshold);
	void IncreaseTemperatureOfPixel(unsigned char *pixel);
	void StopBrightening();
	void SetBrightnessPrecision(int nPrecision);
	void SetBlurScale(int nScale);
	void MakeFrameBright(unsigned char *convertingData, int iVideoHeight, int iVideoWidth, int nPrecision);
	void IncreaseTemperatureOfFrame(unsigned char *convertingData, int iVideoHeight, int iVideoWidth, int nThreshold);

	void boxesForGauss(float sigma, int n);
	void GaussianBlur_4thApproach(unsigned char *scl , unsigned char *tcl, int h, int w, float r);
	void boxBlur_4 (unsigned char *scl, unsigned char *tcl , int h, int w, int r);
	void boxBlurH_4 (unsigned char *scl, unsigned char *tcl, int h, int w, int r);



	unsigned char m_pBluredImage[MAX_FRAME_HEIGHT * MAX_FRAME_WIDTH << 2];

private:

	int m_nPreviousAddValueForBrightening;
	int m_nAverageLuminanceValue;
	int m_nTotalLuminanceValue;
	int m_ThresholdValueForBrightness;
	int m_nBrightnessPrecision;
	int m_nBlurScale;
	unsigned char m_nThresholdValue;

	int m_nVideoHeight;
	int m_nVideoWidth;
	int m_Sizes[3];

	int m_pUIndex[MAX_FRAME_HEIGHT * MAX_FRAME_WIDTH + 1];
	int m_pVIndex[MAX_FRAME_HEIGHT * MAX_FRAME_WIDTH  + 1];
};

#endif 
