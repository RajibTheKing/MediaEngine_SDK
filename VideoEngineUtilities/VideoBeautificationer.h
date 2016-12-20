
#ifndef VIDEO_BEAUTIFICATIONER_H
#define VIDEO_BEAUTIFICATIONER_H

#include <string>

#include "../VideoEngineController/Size.h"

class CVideoBeautificationer
{

public:

	CVideoBeautificationer();
	~CVideoBeautificationer();

	void MakeFrameBeautiful(unsigned char *convertingData, int iVideoHeight, int iVideoWidth, unsigned char *convertedData);
	void MakeFrameBlur(unsigned char *convertingData, int iVideoHeight, int iVideoWidth);
	void MakeFrameBlurAndStore(unsigned char *convertingData, int iVideoHeight, int iVideoWidth);
	bool IsSkinPixel(unsigned char pixel);
	void StartBrightening(int iVideoHeight, int iVideoWidth, int nPrecision);
	void MakePixelBright(unsigned char *pixel);
	void SetTemperatureThreshold(int nThreshold);
	void IncreaseTemperatureOfPixel(unsigned char *pixel);
	void StopBrightening();
	void SetBrightnessPrecision(int nPrecision);
	void SetBlurScale(int nScale);
	void MakeFrameBright(unsigned char *convertingData, int iVideoHeight, int iVideoWidth, int nPrecision);
	void IncreaseTemperatureOfFrame(unsigned char *convertingData, int iVideoHeight, int iVideoWidth, int nThreshold);

private:

	int m_nPreviousAddValueForBrightening;
	int m_nAverageLuminanceValue;
	int m_nTotalLuminanceValue;
	int m_ThresholdValueForBrightness;
	int m_nBrightnessPrecision;
	int m_nBlurScale;

	int m_nVideoHeight;
	int m_nVideoWidth;

	unsigned char m_pBluredImage[MAX_FRAME_HEIGHT * MAX_FRAME_WIDTH << 2];
};

#endif 
