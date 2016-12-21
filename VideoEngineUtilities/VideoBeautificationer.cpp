
#include "VideoBeautificationer.h"
#include <cmath>
#include "../VideoEngineController/LogPrinter.h"


#define NV21 21
#define NV12 12

#define getMin(a,b) a<b?a:b
#define getMax(a,b) a>b?a:b

CVideoBeautificationer::CVideoBeautificationer(int iVideoHeight, int iVideoWidth) :

m_nPreviousAddValueForBrightening(0),
m_nBrightnessPrecision(0)

{
	m_nVideoHeight = iVideoHeight;
	m_nVideoWidth = iVideoWidth;

	GenerateUVIndex(m_nVideoHeight,m_nVideoWidth, NV12 );


	boxesForGauss(1, 3);
}

CVideoBeautificationer::~CVideoBeautificationer()
{

}


void CVideoBeautificationer::SetHeightWidth(int iVideoHeight, int iVideoWidth)
{
	m_nVideoHeight = iVideoHeight;
	m_nVideoWidth = iVideoWidth;

	GenerateUVIndex(m_nVideoHeight,m_nVideoWidth, NV12 );

}

void CVideoBeautificationer::GenerateUVIndex( int iVideoHeight, int iVideoWidth, int dataFormat )
{
	//LOGE("fahad -->> ----------------- iHeight = %d, iWdith = %d", iVideoHeight, iVideoWidth);
	int iHeight = iVideoHeight;
	int iWidth = iVideoWidth;
	int yLength = iHeight * iWidth;
	int uLength = yLength/2;

	int yConIndex = 0;
	int vIndex ;
	int uIndex ;
	vIndex = yLength+(yLength/4);
	uIndex = yLength;


	//LOGE("fahad -->> ----------------- iHeight = %d, iWdith = %d , vIndex = %d, uIndex = %d, yLength = %d", iVideoHeight, iVideoWidth, vIndex, uIndex, yLength);
	int heightIndex = 1;
	for(int i = 0;  ;)
	{
		if(i == iWidth*heightIndex) {
			i+=iWidth;
			heightIndex+=2;
		}
		if(i>=yLength) break;
		yConIndex = i;



		m_pUIndex[yConIndex] = uIndex;
		m_pUIndex[yConIndex + 1] = uIndex;
		m_pUIndex[yConIndex + iWidth] = uIndex;
		m_pUIndex[yConIndex + iWidth + 1] = uIndex;

		m_pVIndex[yConIndex] = vIndex;
		m_pVIndex[yConIndex + 1] = vIndex;
		m_pVIndex[yConIndex + iWidth] = vIndex;
		m_pVIndex[yConIndex + iWidth + 1] = vIndex;

		//LOGE("fahad -->> ----------------- iHeight = %d, iWdith = %d , vIndex = %d, uIndex = %d, yLength = %d", iVideoHeight, iVideoWidth, vIndex, uIndex, yLength);

		uIndex += 1;
		vIndex += 1;
		i+=2;
	}
}

void CVideoBeautificationer::MakeFrameBeautiful(unsigned char *convertingData, int iVideoHeight, int iVideoWidth, unsigned char *convertedData)
{

}

void CVideoBeautificationer::MakeFrameBlur(unsigned char *convertingData, int iVideoHeight, int iVideoWidth)
{

}

void CVideoBeautificationer::MakeFrameBlurAndStore(unsigned char *convertingData, int iVideoHeight, int iVideoWidth)
{
	GaussianBlur_4thApproach(convertingData, m_pBluredImage, iVideoHeight, iVideoWidth, 2 );
	//memcpy(convertingData,  m_pBluredImage, iVideoHeight*iVideoWidth );
}

bool CVideoBeautificationer::IsSkinPixel(unsigned char *pixel)
{
	int iTotLen = m_nVideoWidth * m_nVideoHeight;
	int iLen =  m_nVideoWidth * m_nVideoHeight;//(int)(modifData.length / 1.5);
	//int totalYValue = 0;
	for(int i=0; i<iLen; i++)
	{
		int yVal = pixel[i]  & 0xFF;
		int uVal = pixel[m_pUIndex[i]] & 0xFF;
		int vVal = pixel[m_pVIndex[i]] & 0xFF;

		if( (uVal > 94 && uVal < 126) && (vVal > 134 && vVal < 176)  )
		{
			pixel[i] = m_pBluredImage[i];
		}

		//totalYValue += yVal;
		//MakePixelBright(&pixel[i]);
	}



	//int m_AverageValue = totalYValue / iLen;

	//SetBrighteningValue(m_AverageValue , 10/*int brightnessPrecision*/);

	IncreaseTemperatureOfFrame(pixel, m_nVideoHeight, m_nVideoWidth, 4);

	return true;
}

void CVideoBeautificationer::StartBrightening(int iVideoHeight, int iVideoWidth, int nPrecision)
{
	m_nVideoHeight = iVideoHeight;
	m_nVideoWidth = iVideoWidth;
	m_nPreviousAddValueForBrightening = 0;
}


void CVideoBeautificationer::SetBrighteningValue(int m_AverageValue, int brightnessPrecision)
{
	if(m_AverageValue < 10)
	{
		m_nThresholdValue = 44;
	}else if(m_AverageValue < 20)
	{
		m_nThresholdValue = 50;
	}else if(m_AverageValue < 30)
	{
		m_nThresholdValue = 60;
	}else if(m_AverageValue < 40)
	{
		m_nThresholdValue = 70;
	}else if(m_AverageValue < 50)
	{
		m_nThresholdValue = 75;
	}else if(m_AverageValue < 60)
	{
		m_nThresholdValue = 80;
	}else if(m_AverageValue < 70)
	{
		m_nThresholdValue = 90;
	}else if(m_AverageValue < 80)
	{
		m_nThresholdValue = 95;
	}else if(m_AverageValue < 90)
	{
		m_nThresholdValue = 100;
	}else if(m_AverageValue < 100)
	{
		m_nThresholdValue = 110;
	}else if(m_AverageValue < 110){
		m_nThresholdValue = 115;
	}else if(m_AverageValue < 120){
		m_nThresholdValue = 125;
	}else{
		m_nThresholdValue = 100;
	}

	m_nPreviousAddValueForBrightening = (m_nThresholdValue - m_AverageValue);
	m_nPreviousAddValueForBrightening = m_nPreviousAddValueForBrightening >> 1;
	if (m_nPreviousAddValueForBrightening < 0)
		m_nPreviousAddValueForBrightening = 0;

	m_nBrightnessPrecision = brightnessPrecision;
}

void CVideoBeautificationer::MakePixelBright(unsigned char *pixel)
{
	int iPixelValue = *pixel + m_nPreviousAddValueForBrightening + m_nBrightnessPrecision;
	*pixel = getMin(iPixelValue, 255);

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

void CVideoBeautificationer::IncreaseTemperatureOfFrame(unsigned char *convertingData, int iVideoHeight, int iVideoWidth,
														unsigned char nThreshold)
{
	int startUIndex = iVideoHeight * iVideoWidth;
	int uLen = startUIndex + (startUIndex / 4);
	int vLen = startUIndex + (startUIndex / 2);
	for(int i=startUIndex; i <uLen; i++)
	{
		convertingData[i] = convertingData[i] - nThreshold;
	}

	for(int i=uLen; i <vLen; i++)
	{
		convertingData[i] = convertingData[i] + nThreshold;
	}
}

void CVideoBeautificationer::boxesForGauss(float sigma, int n)  // standard deviation, number of boxes
{
	float wIdeal = (float)sqrt((12*sigma*sigma/n)+1);  // Ideal averaging filter width
	int wl = (int)floor(wIdeal);
	if(wl%2==0) wl--;
	int wu = wl+2;

	float mIdeal = (12*sigma*sigma - n*wl*wl - 4*n*wl - 3*n)/(-4*wl - 4);
	int m = (int) floor(mIdeal + 0.5);

	// var sigmaActual = sqrt( (m*wl*wl + (n-m)*wu*wu - n)/12 );

	for(int i=0; i<n; i++)
	{
		m_Sizes[i]=i<m?wl:wu;
	}

	return;
}

void CVideoBeautificationer::GaussianBlur_4thApproach(unsigned char *scl , unsigned char *tcl, int h, int w, float r)
{
	boxBlur_4 (scl,tcl, h, w, (m_Sizes[2]-1)/2);

}


void CVideoBeautificationer::boxBlur_4 (unsigned char *scl, unsigned char *tcl , int h, int w, int r)
{
	boxBlurH_4( scl, tcl, h, w, r);
}

/*void CVideoBeautificationer::boxBlurH_4 (unsigned char *scl, unsigned char *tcl, int h, int w, int r)
{
	int iarr = (r+r+1);
	for(int i=0; i<h; i++)
	{
		int ti = i*w, li = ti, ri = ti+r;
		int fv = scl[ti], lv = scl[ti+w-1], val = (r+1)*fv;

		for(int j=0; j<r; j++) val += scl[ti+j] ;
		for(int j=0  ; j<=r ; j++) {
			val += scl[ri++]  - fv;
			tcl[ti++] = ( unsigned char)(val/iarr) & 0xFF;
		}
		for(int j=r+1; j<w-r; j++) {
			val += scl[ri++]  - scl[li++] ;
			tcl[ti++] = (unsigned char)(val/iarr) & 0xFF;;
		}
	//for(int j=w-r; j<w  ; j++) { val += lv        - scl[li++];   tcl[ti++] = (unsigned char)floor(val*iarr + 0.5); }
	}
}*/

void CVideoBeautificationer::boxBlurH_4(unsigned char *scl, unsigned char *tcl, int h, int w, int r)
{
	int iarr = (r+r+1);
	for(int i=0; i<h; i++)
	{
		int ti = i*w, li = ti, ri = ti+r;
		int fv = (scl[ti] & 0xFF), lv = (scl[ti+w-1] & 0xFF), val = (r+1)*fv;

		for(int j=0; j<r; j++) val += (scl[ti+j] & 0xFF);
		for(int j=0  ; j<=r ; j++) { val += (scl[ri++] & 0xFF) - fv       ;   tcl[ti++] = (unsigned char)((unsigned char)(val/iarr) & 0xFF); }
		for(int j=r+1; j<w-r; j++) {
			val += (scl[ri++] & 0xFF) - (scl[li++] & 0xFF);   tcl[ti++] = (unsigned char)( (unsigned char)(val/iarr)  & 0xFF);
		}
		for(int j=w-r; j<w  ; j++) {
			val += (scl[ri++] & 0xFF) - (scl[li++] & 0xFF);   tcl[ti++] = (unsigned char)( (unsigned char)(val/iarr)  & 0xFF);
		}//(unsigned char)floor(val*iarr + 0.5); }
	}
	//return tcl;
}