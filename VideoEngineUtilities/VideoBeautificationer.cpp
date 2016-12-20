
#include "VideoBeautificationer.h"


CVideoBeautificationer::CVideoBeautificationer() :

m_nPreviousAddValueForBrightening(0),
m_nBrightnessPrecision(0)

{

}

CVideoBeautificationer::~CVideoBeautificationer()
{

}

void CVideoBeautificationer::MakeFrameBeautiful(unsigned char *convertingData, int iVideoHeight, int iVideoWidth, unsigned char *convertedData)
{

}

void CVideoBeautificationer::MakeFrameBlur(unsigned char *convertingData, int iVideoHeight, int iVideoWidth)
{

}

void CVideoBeautificationer::MakeFrameBlurAndStore(unsigned char *convertingData, int iVideoHeight, int iVideoWidth)
{

}

bool CVideoBeautificationer::IsSkinPixel(unsigned char pixel)
{

}

void CVideoBeautificationer::StartBrightening(int iVideoHeight, int iVideoWidth, int nPrecision)
{
	m_nVideoHeight = iVideoHeight;
	m_nVideoWidth = iVideoWidth;
	m_nPreviousAddValueForBrightening = 0;
}

void CVideoBeautificationer::MakePixelBright(unsigned char *pixel)
{
	int brightenPixelValue = *pixel + m_nPreviousAddValueForBrightening + m_nBrightnessPrecision;

	if (brightenPixelValue > MAXIMUM_LUMINANCE_VALUE)
		brightenPixelValue = MAXIMUM_LUMINANCE_VALUE;

	*pixel = brightenPixelValue;

	m_nAverageLuminanceValue = m_nTotalLuminanceValue / (m_nVideoHeight * m_nVideoWidth);

	if (m_nAverageLuminanceValue < 10)
	{
		m_ThresholdValueForBrightness = 44;
	}
	else if (m_nAverageLuminanceValue < 20)
	{
		m_ThresholdValueForBrightness = 50;
	}
	else if (m_nAverageLuminanceValue < 30)
	{
		m_ThresholdValueForBrightness = 60;
	}
	else if (m_nAverageLuminanceValue < 40)
	{
		m_ThresholdValueForBrightness = 70;
	}
	else if (m_nAverageLuminanceValue < 50)
	{
		m_ThresholdValueForBrightness = 75;
	}
	else if (m_nAverageLuminanceValue < 60)
	{
		m_ThresholdValueForBrightness = 80;
	}
	else if (m_nAverageLuminanceValue < 70)
	{
		m_ThresholdValueForBrightness = 90;
	}
	else if (m_nAverageLuminanceValue < 80)
	{
		m_ThresholdValueForBrightness = 95;
	}
	else if (m_nAverageLuminanceValue < 90)
	{
		m_ThresholdValueForBrightness = 100;
	}
	else if (m_nAverageLuminanceValue < 100)
	{
		m_ThresholdValueForBrightness = 110;
	}
	else if (m_nAverageLuminanceValue < 110)
	{
		m_ThresholdValueForBrightness = 115;
	}
	else if (m_nAverageLuminanceValue < 120)
	{
		m_ThresholdValueForBrightness = 125;
	}
	else
	{
		m_ThresholdValueForBrightness = 100;
	}

	m_nPreviousAddValueForBrightening = (m_ThresholdValueForBrightness - m_nAverageLuminanceValue);

	if (m_nPreviousAddValueForBrightening < 0)
		m_nPreviousAddValueForBrightening = 0;

	m_nPreviousAddValueForBrightening = m_nPreviousAddValueForBrightening >> 1;
}

void CVideoBeautificationer::SetTemperatureThreshold(int nThreshold)
{

}

void CVideoBeautificationer::IncreaseTemperatureOfPixel(unsigned char *pixel)
{

}

void CVideoBeautificationer::StopBrightening()
{

}

void CVideoBeautificationer::SetBrightnessPrecision(int nPrecision)
{
	m_nBrightnessPrecision = nPrecision;
}

void CVideoBeautificationer::SetBlurScale(int nScale)
{
	m_nBlurScale = nScale;
}

void CVideoBeautificationer::MakeFrameBright(unsigned char *convertingData, int iVideoHeight, int iVideoWidth, int nPrecision)
{

}

void CVideoBeautificationer::IncreaseTemperatureOfFrame(unsigned char *convertingData, int iVideoHeight, int iVideoWidth, int nThreshold)
{

}